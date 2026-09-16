#include "MultigroupSensitivityAux.h"
#include "EnergyGroupStructures.h"

#ifdef ENABLE_OPENMC_COUPLING
#include "openmc/settings.h"
#include "openmc/material.h"
#endif

registerMooseObject("CardinalApp", MultigroupSensitivityAux);

InputParameters
MultigroupSensitivityAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addRequiredCoupledVar("forward_fluxes", "Forward flux for each energy group");
  params.addRequiredCoupledVar("adjoint_fluxes", "Adjoint flux for each energy group");

  params.addRequiredParam<std::vector<std::string>>(
      "material_names", "Names of the materials");
  params.addRequiredParam<std::string>(
      "from_material", "Name of the material being replaced");
  params.addRequiredParam<std::string>(
      "to_material", "Name of the replacement material");

  params.addParam<std::vector<Real>>("energy_boundaries",
      "Energy group boundaries in ascending order");
  MooseEnum groups = getEnergyGroupsEnum();
  params.addParam<MooseEnum>(
      "group_structure", groups, "Named energy group structure");

  params.addParam<std::vector<FunctionName>>("sigma_t_functions",
      "Total cross section functions (one per material). Each is a PiecewiseConstant "
      "mapping group index (g) to sigma_t[g].");
  params.addParam<std::vector<FunctionName>>("sigma_s_functions",
      "Total scattering cross section functions (one per material). Each is a "
      "PiecewiseConstant mapping group index (g) to sigma_s[g].");

  params.addParam<std::vector<std::string>>("response_channels",
      "Names of response cross section channels for the direct effect term");

  params.addParam<std::vector<FunctionName>>("sigma_r_functions",
      "Response cross section functions (one per channel per material). "
      "Ordered as: [chan0_mat0, chan0_mat1, ..., chan1_mat0, chan1_mat1, ...]. "
      "Each is a PiecewiseConstant mapping group index (g) to sigma_r[g].");

  params.addParam<bool>("load_transport_from_mgxs", false,
      "Load sigma_t and sigma_s from the OpenMC MGXS library (requires MG mode). "
      "Group structure is taken from the MGXS library.");
  params.addParam<std::vector<int>>("material_ids",
      "OpenMC material IDs corresponding to material_names. "
      "Required when load_transport_from_mgxs is true.");

#ifdef ENABLE_OPENMC_COUPLING
  params.addParam<std::vector<std::string>>("response_nuclides",
      "Nuclide names for CE response cross section collapse. "
      "When specified, response cross sections are collapsed from continuous-energy "
      "data using the group boundaries, rather than read from explicit functions.");
  params.addParam<Real>("temperature", 300.0,
      "Temperature in K for CE cross section data lookup");
#endif

  params.addClassDescription("Computes multi-group topology optimization sensitivity "
      "for multiple materials using forward and adjoint fluxes.");

  return params;
}

MultigroupSensitivityAux::MultigroupSensitivityAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _num_groups(0),
    _num_materials(0),
    _from_mat(0),
    _to_mat(0),
    _load_transport(getParam<bool>("load_transport_from_mgxs"))
#ifdef ENABLE_OPENMC_COUPLING
    ,
    _temperature(getParam<Real>("temperature"))
#endif
{
  // ---- resolve group structure ----
  _energy_bnds = resolveGroupBoundaries();
  _num_groups = _energy_bnds.size() - 1;

  // ---- material names and from/to indices ----
  auto material_names = getParam<std::vector<std::string>>("material_names");
  _num_materials = material_names.size();
  if (_num_materials < 2)
    mooseError("At least two materials are required.");

  std::string from_name = getParam<std::string>("from_material");
  std::string to_name = getParam<std::string>("to_material");
  bool found_from = false, found_to = false;

  for (unsigned int m = 0; m < _num_materials; ++m)
  {
    if (material_names[m] == from_name)
    {
      _from_mat = m;
      found_from = true;
    }
    if (material_names[m] == to_name)
    {
      _to_mat = m;
      found_to = true;
    }
  }

  if (!found_from)
    mooseError("from_material '", from_name, "' not found in material_names.");
  if (!found_to)
    mooseError("to_material '", to_name, "' not found in material_names.");

  // ---- allocate cross section storage ----
  _sigma_t.assign(_num_materials, std::vector<Real>(_num_groups, 0.0));
  _sigma_s.assign(_num_materials, std::vector<Real>(_num_groups, 0.0));

  if (isParamValid("response_channels"))
    _response_channels = getParam<std::vector<std::string>>("response_channels");

  _sigma_r.resize(_response_channels.size(),
      std::vector<std::vector<Real>>(_num_materials,
          std::vector<Real>(_num_groups, 0.0)));

  // ---- load transport cross sections ----
  if (_load_transport)
  {
#ifdef ENABLE_OPENMC_COUPLING
    loadOpenMGMGXS();
#else
    mooseError("load_transport_from_mgxs requires OpenMC coupling "
               "(ENABLE_OPENMC_COUPLING).");
#endif
  }
  else
    computeExplicitXS();

  // ---- load response cross sections ----
  if (_response_channels.size() > 0)
  {
    if (isParamValid("sigma_r_functions"))
      computeExplicitResponseXS();
#ifdef ENABLE_OPENMC_COUPLING
    else if (isParamValid("response_nuclides"))
    {
      _response_nuclides = getParam<std::vector<std::string>>("response_nuclides");
      _response_mts.resize(_response_channels.size());
      for (unsigned int c = 0; c < _response_channels.size(); ++c)
        _response_mts[c] = openmc::reaction_mt(_response_channels[c]);
      for (unsigned int c = 0; c < _response_channels.size(); ++c)
        for (unsigned int m = 0; m < _num_materials; ++m)
          collapseCEResponseXS(c, m, _sigma_r[c][m]);
    }
#endif
  }

  // ---- couple flux variables ----
  auto n_forward = coupledComponents("forward_fluxes");
  auto n_adjoint = coupledComponents("adjoint_fluxes");

  if (n_forward != _num_groups)
    mooseError("Number of forward_fluxes (", n_forward, ") does not match "
               "number of groups (", _num_groups, ").");
  if (n_adjoint != _num_groups)
    mooseError("Number of adjoint_fluxes (", n_adjoint, ") does not match "
               "number of groups (", _num_groups, ").");

  for (unsigned int i = 0; i < _num_groups; ++i)
  {
    _forward_fluxes.push_back(&coupledValue("forward_fluxes", i));
    _adjoint_fluxes.push_back(&coupledValue("adjoint_fluxes", i));
  }
}

Real
MultigroupSensitivityAux::computeValue()
{
  Real sensitivity = 0.0;

  for (unsigned int g = 0; g < _num_groups; ++g)
  {
    Real phi_f_g = (*_forward_fluxes[g])[_qp];
    Real phi_adj_g = (*_adjoint_fluxes[g])[_qp];

    // response channels: direct effect
    for (unsigned int c = 0; c < _response_channels.size(); ++c)
    {
      Real ds_r = _sigma_r[c][_to_mat][g] - _sigma_r[c][_from_mat][g];
      sensitivity += ds_r * phi_f_g;
    }

    // removal: adj * delta_sigma_t * forward
    // sigma_t already includes total scatter-out, so this captures absorption + out-scatter
    Real ds_t = _sigma_t[_to_mat][g] - _sigma_t[_from_mat][g];
    sensitivity -= phi_adj_g * ds_t * phi_f_g;
  }

  return sensitivity;
}

std::vector<Real>
MultigroupSensitivityAux::resolveGroupBoundaries()
{
  bool has_bnds = isParamValid("energy_boundaries");
  bool has_struct = isParamValid("group_structure");

  if (has_bnds == has_struct)
    mooseError("Specify exactly one of 'energy_boundaries' or 'group_structure'.");

  if (has_bnds)
  {
    auto bnds = getParam<std::vector<Real>>("energy_boundaries");
    if (bnds.size() < 2)
      mooseError("At least two energy boundaries are required.");
    std::sort(bnds.begin(), bnds.end());
    if (std::adjacent_find(bnds.begin(), bnds.end()) != bnds.end())
      mooseError("Duplicate energy boundaries are not allowed.");
    return bnds;
  }

  using namespace energyfilter;
  using namespace groupstructures;
  auto gs = getParam<MooseEnum>("group_structure").getEnum<GroupStructureEnum>();

  switch (gs)
  {
    case CASMO_2:
      return std::vector<Real>(S_CASMO_2, S_CASMO_2 + 3);
    case CASMO_4:
      return std::vector<Real>(S_CASMO_4, S_CASMO_4 + 5);
    case CASMO_8:
      return std::vector<Real>(S_CASMO_8, S_CASMO_8 + 9);
    case CASMO_16:
      return std::vector<Real>(S_CASMO_16, S_CASMO_16 + 17);
    case CASMO_25:
      return std::vector<Real>(S_CASMO_25, S_CASMO_25 + 26);
    case CASMO_40:
      return std::vector<Real>(S_CASMO_40, S_CASMO_40 + 41);
    case VITAMINJ_42:
      return std::vector<Real>(S_VITAMINJ_42, S_VITAMINJ_42 + 43);
    case SCALE_44:
      return std::vector<Real>(S_SCALE_44, S_SCALE_44 + 45);
    case MPACT_51:
      return std::vector<Real>(S_MPACT_51, S_MPACT_51 + 52);
    case MPACT_60:
      return std::vector<Real>(S_MPACT_60, S_MPACT_60 + 61);
    case MPACT_69:
      return std::vector<Real>(S_MPACT_69, S_MPACT_69 + 70);
    case CASMO_70:
      return std::vector<Real>(S_CASMO_70, S_CASMO_70 + 71);
    case XMAS_172:
      return std::vector<Real>(S_XMAS_172, S_XMAS_172 + 173);
    case VITAMINJ_175:
      return std::vector<Real>(S_VITAMINJ_175, S_VITAMINJ_175 + 176);
    case SCALE_252:
      return std::vector<Real>(S_SCALE_252, S_SCALE_252 + 253);
    case TRIPOLI_315:
      return std::vector<Real>(S_TRIPOLI_315, S_TRIPOLI_315 + 316);
    case SHEM_361:
      return std::vector<Real>(S_SHEM_361, S_SHEM_361 + 362);
    case CCFE_709:
      return std::vector<Real>(S_CCFE_709, S_CCFE_709 + 710);
    case UKAEA_1102:
      return std::vector<Real>(S_UKAEA_1102, S_UKAEA_1102 + 1103);
    case ECCO_1968:
      return std::vector<Real>(S_ECCO_1968, S_ECCO_1968 + 1969);
    default:
      mooseError("Internal error: unknown GroupStructureEnum.");
  }

  return {};
}

void
MultigroupSensitivityAux::computeExplicitXS()
{
  auto st_names = getParam<std::vector<FunctionName>>("sigma_t_functions");
  auto ss_names = getParam<std::vector<FunctionName>>("sigma_s_functions");

  if (st_names.size() != _num_materials)
    mooseError("sigma_t_functions must have exactly one entry per material (",
              _num_materials, "). Got ", st_names.size(), ".");
  if (ss_names.size() != _num_materials)
    mooseError("sigma_s_functions must have exactly one entry per material (",
              _num_materials, "). Got ", ss_names.size(), ".");

  _sigma_t_funcs.resize(_num_materials);
  _sigma_s_funcs.resize(_num_materials);
  for (unsigned int m = 0; m < _num_materials; ++m)
  {
    _sigma_t_funcs[m] = &getFunctionByName(st_names[m]);
    _sigma_s_funcs[m] = &getFunctionByName(ss_names[m]);
  }

  for (unsigned int m = 0; m < _num_materials; ++m)
  {
    for (unsigned int g = 0; g < _num_groups; ++g)
      _sigma_t[m][g] = _sigma_t_funcs[m]->value(static_cast<Real>(g), Point());

    for (unsigned int g = 0; g < _num_groups; ++g)
      _sigma_s[m][g] = _sigma_s_funcs[m]->value(static_cast<Real>(g), Point());
  }
}

void
MultigroupSensitivityAux::computeExplicitResponseXS()
{
  auto sr_names = getParam<std::vector<FunctionName>>("sigma_r_functions");
  unsigned int expected = _response_channels.size() * _num_materials;
  if (sr_names.size() != expected)
    mooseError("sigma_r_functions must have one entry per channel per material (",
              expected, "). Got ", sr_names.size(), ".");

  _sigma_r_funcs.resize(_response_channels.size());
  for (unsigned int c = 0; c < _response_channels.size(); ++c)
  {
    _sigma_r_funcs[c].resize(_num_materials);
    for (unsigned int m = 0; m < _num_materials; ++m)
    {
      unsigned int idx = c * _num_materials + m;
      _sigma_r_funcs[c][m] = &getFunctionByName(sr_names[idx]);
    }
  }

  for (unsigned int c = 0; c < _response_channels.size(); ++c)
    for (unsigned int m = 0; m < _num_materials; ++m)
      for (unsigned int g = 0; g < _num_groups; ++g)
        _sigma_r[c][m][g] =
            _sigma_r_funcs[c][m]->value(static_cast<Real>(g), Point());
}

#ifdef ENABLE_OPENMC_COUPLING

void
MultigroupSensitivityAux::loadOpenMGMGXS()
{
  if (openmc::settings::run_CE)
    mooseError("load_transport_from_mgxs requires multi-group mode "
               "(run_CE = false).");

  auto mat_ids = getParam<std::vector<int>>("material_ids");
  if (mat_ids.size() != _num_materials)
    mooseError("material_ids must have one entry per material (",
              _num_materials, "). Got ", mat_ids.size(), ".");

  if (openmc::data::mg.macro_xs_.size() == 0)
    mooseError("OpenMC MGXS library has no macroscopic cross sections. "
               "Ensure the MGXS file was loaded correctly.");

  if (openmc::data::mg.num_energy_groups_ != static_cast<int>(_num_groups))
    mooseError("MGXS library has ", openmc::data::mg.num_energy_groups_,
              " groups but the group structure specifies ", _num_groups,
              " groups. These must match when loading from MGXS.");

  // Resolve OpenMC material indices from user-provided IDs
  _openmc_mat_indices.resize(_num_materials);
  for (unsigned int m = 0; m < _num_materials; ++m)
  {
    auto it = openmc::model::material_map.find(mat_ids[m]);
    if (it == openmc::model::material_map.end())
      mooseError("Material ID ", mat_ids[m], " not found in OpenMC model.");
    _openmc_mat_indices[m] = it->second;
  }

  // Load sigma_t and total sigma_s from MGXS library.
  // OpenMC MGXS convention: group 0 = highest energy.
  // Our convention: group 0 = lowest energy (ascending boundaries).
  // sigma_s[g] = sum_{g'} sigma_s(g -> g') i.e. total scattering out of group g.
  unsigned int N = _num_groups;
  for (unsigned int m = 0; m < _num_materials; ++m)
  {
    auto & macro = openmc::data::mg.macro_xs_[_openmc_mat_indices[m]];

    for (unsigned int g = 0; g < N; ++g)
    {
      int mg_g = static_cast<int>(N - 1 - g);
      _sigma_t[m][g] = macro.get_xs(openmc::MgxsType::TOTAL, mg_g, 0, 0);
      _sigma_s[m][g] = macro.get_xs(openmc::MgxsType::SCATTER, mg_g, 0, 0);
    }
  }
}

void
MultigroupSensitivityAux::collapseCEResponseXS(
    unsigned int channel, unsigned int mat, std::vector<Real> & result)
{
  if (openmc::settings::run_CE)
  {
    int mat_idx = _openmc_mat_indices[mat];
    auto & openmc_mat = openmc::model::materials[mat_idx];
    int mt = _response_mts[channel];

    for (int nuc_idx : openmc_mat->nuclides())
    {
      auto & nuc = *openmc::data::nuclides[nuc_idx];

      for (unsigned int g = 0; g < _num_groups; ++g)
      {
        double E0 = _energy_bnds[g];
        double E1 = _energy_bnds[g + 1];
        std::vector<double> e = {E0, E1};
        std::vector<double> f = {1.0};
        double rate = nuc.collapse_rate(
            mt, _temperature,
            openmc::span<const double>(e.data(), e.size()),
            openmc::span<const double>(f.data(), f.size()));
        double dE = E1 - E0;
        result[g] += dE > 0.0 ? rate / dE : 0.0;
      }
    }
  }
  else
  {
    mooseError("CE response collapse is only available in continuous-energy mode.");
  }
}

#endif
