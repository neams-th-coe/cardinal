//
// Created by Ronald Rahaman on 2019-01-23.
//

#include "NearestPointReceiver.h"
#include "AuxiliarySystem.h"
#include "DelimitedFileReader.h"

#include "mpi.h"
#include "OpenMCProblem.h"
#include "openmc/capi.h"
#include "openmc/cell.h"
#include "openmc/constants.h"
#include "openmc/error.h"
#include "openmc/particle.h"
#include "openmc/geometry.h"
#include "openmc/message_passing.h"
#include "openmc/settings.h"
#include "openmc/summary.h"
#include "xtensor/xarray.hpp"
#include "xtensor/xview.hpp"

registerMooseObject("CardinalApp", OpenMCProblem);

template<>
InputParameters
validParams<OpenMCProblem>()
{
  InputParameters params = validParams<ExternalProblem>();
  params.addRequiredRangeCheckedParam<Real>("power", "power >= 0.0",
    "specified power for normalizing OpenMC kappa fission tally");

  params.addParam<std::vector<Point>>("centers", "Coordinates of cell centers to transfer data with");
  params.addParam<std::vector<FileName>>("centers_file", "Alternative way to provide the coordinates of cells "
    "to transfer data with");

  params.addParam<std::vector<Real>>("volumes", "volumes of pebbles");
  params.addRequiredParam<MooseEnum>("tally_type", getTallyTypeEnum(), "type of tally to use in OpenMC");
  params.addRequiredParam<int>("pebble_cell_level", "Level of pebble cells in the OpenMC model");
  params.addParam<std::string>("mesh_template", "mesh tally template for OpenMC");
  params.addParam<bool>("check_tally_sum", true, "Check heating tally consistency between timesteps");
  return params;
}

OpenMCProblem::OpenMCProblem(const InputParameters &params) :
  ExternalProblem(params),
  _pebble_cell_level(getParam<int>("pebble_cell_level")),
  _power(getParam<Real>("power")),
  _volumes(getParam<std::vector<Real>>("volumes")),
  _tallyType(getParam<MooseEnum>("tally_type").getEnum<tally::TallyTypeEnum>()),
  _check_tally_sum(getParam<bool>("check_tally_sum"))
{
  if (isParamValid("centers") == isParamValid("centers_file"))
    mooseError("OpenMCProblem requires either 'centers' or 'centers_file' - you have specified "
      "either both or none");

  // get the coordinates for each cell center that we wish to transfer data with
  fillCenters();

  if (_tallyType == tally::cell)
  {
    if (!isParamValid("volumes"))
      paramError("volumes", "Wth cell-based tallies, the volumes of each MOOSE receiving mesh "
        "must be provided!");

    _volumes = getParam<std::vector<Real>>("volumes");

    // number of supplied volumes must either be 1 (in which case each cell is assumed
    // to be the same volume) or have a length equal to the number of centers provided
    if ((_volumes.size() > 1) &&  (_volumes.size() != _centers.size()))
      paramError("volumes", "When more than one volume is provided, the length must match "
        "the length of the 'centers' provided!");

    // if only one volume was provided, fill out the _volumes array for heat source normalization
    // agnostic to whether one or N volumes were provided by the user
    if (_volumes.size() == 1)
    {
      Real vol = _volumes[0];
      _volumes = std::vector<double>(_centers.size(), vol);
    }
  }

  if (_tallyType == tally::mesh)
  {
    if (!isParamValid("mesh_template"))
      paramError("mesh_template", "When using a mesh tally, the 'mesh_template' must be provided!");

    _meshTemplateFilename = getParam<std::string>("mesh_template");

    if (_meshTemplateFilename.empty())
      paramError("mesh_template", "When using a mesh tally, the 'mesh_template' parameter cannot be empty!");
  }

  auto& m = mesh().getMesh();

  if (openmc::settings::libmesh_comm)
    mooseWarning("libMesh communicator already set in OpenMC.");

  openmc::settings::libmesh_comm = &m.comm();

  // Find cell for each pebble center
  for (const auto & c : _centers)
  {
    openmc::Particle p {};
    p.r() = {c(0), c(1), c(2)};
    p.u() = {0., 0., 1.};

    if (!openmc::exhaustive_find_cell(p))
      mooseError("Cannot find OpenMC cell at position (x, y, z) = (" + Moose::stringify(c(0)) + ", " +
        Moose::stringify(c(1)) + ", " + Moose::stringify(c(2)) + ")");

    _cellIndices.push_back(p.coord(_pebble_cell_level).cell);
    _cellInstances.push_back(p.cell_instance());
  }

  // ensure that the _cellIndices are unique
  auto unique_cell_indices = std::set<int32_t>(_cellIndices.begin(), _cellIndices.end());
  if (_cellIndices.size() != unique_cell_indices.size()) {
    mooseError("The cells found in the geometry for the centers provided at level '"
               + Moose::stringify(_pebble_cell_level) +
               "' are not unique. Please check the centers and the 'pebble_cell_level' parameter." );
  }

  setupTallies();
}

OpenMCProblem::~OpenMCProblem()
{
  openmc_finalize();
}

void
OpenMCProblem::fillCenters()
{
  if (isParamValid("centers"))
  {
    _centers = getParam<std::vector<Point>>("centers");

    // no need to also check if we have the correct length, since using Point ensures that already
    if (_centers.empty())
      paramError("centers", "'centers' cannot be empty.");
  }
  else if (isParamValid("centers_file"))
  {
    std::vector<FileName> centers_file = getParam<std::vector<FileName>>("centers_file");

    if (centers_file.empty())
      paramError("centers_file", "'centers_file' cannot be empty.");

    for (const auto & f : centers_file)
    {
      MooseUtils::DelimitedFileReader file(f, &_communicator);
      file.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::ROWS);
      file.read();

      const std::vector<std::vector<double>> & data = file.getData();

      if (file.numEntries() % DIMENSION != 0)
        paramError("centers_file", "Number of entries in 'centers_file' ",
          f, " must be divisible by 3 to give x, y, and z coordinates");

      for (unsigned int i = 0; i < data.size(); ++i)
      {
        Point position;

        if (data[i].size() != DIMENSION)
          paramError("centers_file", "All entries in 'centers_file' ", f,
            " must contain exactly ", DIMENSION, " coordinates.");

        for (unsigned int j = 0; j < DIMENSION; j++)
          position(j) = data.at(i).at(j);

        _centers.push_back(position);
      }
    }
  }
}

void OpenMCProblem::setupTallies() {

  // create a global 'kappa-fission' tally
  if (_check_tally_sum) {
    auto tally = openmc::Tally::create();
    tally->set_scores({"kappa-fission"});
    _kappa_fission_tally = tally;
    // make sure the global tally estimator matches
    // when using a mesh tally
    if (_tallyType == tally::mesh) tally->estimator_ = openmc::TallyEstimator::COLLISION;
  }

  switch (_tallyType)
    {
      case tally::cell:
        setupCellTally();
        break;
      case tally::mesh:
        setupMeshTallies();
        break;
      default:
        mooseError("Unhandled TallyTypeEnum in OpenMCProblem!");
    }

}

void OpenMCProblem::setupCellTally() {
  // create a cell filter
  auto cellFilter = dynamic_cast<openmc::CellFilter*>(openmc::Filter::create("cell"));
  _cellFilters.push_back(cellFilter);
  cellFilter->set_cells(_cellIndices);

  // create a new tally
  auto tally = openmc::Tally::create();
  _tallies.push_back(tally);

  std::vector<openmc::Filter*> tally_filters = {cellFilter};
  tally->set_filters(tally_filters);
  tally->set_scores({"kappa-fission"});
}

void OpenMCProblem::setupMeshTallies() {
  // create the OpenMC mesh instance
  int meshId = 0;
  for (const auto& mesh : openmc::model::meshes) { meshId = std::max(meshId, mesh->id_); }
  auto mesh = std::make_unique<openmc::LibMesh>(_meshTemplateFilename);
  mesh->id_ = ++meshId;
  mesh->output_ = false;

  _meshTemplate = mesh.get();

  int32_t mesh_index = openmc::model::meshes.size();
  openmc::model::mesh_map[mesh->id_] = mesh_index;
  openmc::model::meshes.push_back(std::move(mesh));

  // setup a new mesh filter
  int32_t meshFilterId;
  openmc_get_filter_next_id(&meshFilterId);

  int32_t meshTallyId;
  openmc_get_tally_next_id(&meshTallyId);

  for (auto& c : _centers) {
    auto meshFilter = dynamic_cast<openmc::MeshFilter*>(openmc::Filter::create("mesh"));
    meshFilter->set_mesh(mesh_index);
    meshFilter->set_translation({c(0), c(1), c(2)});

    _meshFilters.push_back(meshFilter);

    std::vector<openmc::Filter*> tally_filters = {meshFilter};

    // apply the mesh filter to a tally
    auto tally = openmc::Tally::create();
    tally->set_filters(tally_filters);
    tally->set_scores({"kappa-fission"});
    tally->estimator_ = openmc::TallyEstimator::COLLISION;
    _tallies.push_back(tally);
  }

  if (!_check_tally_sum) {
    // performance optimization - assume the mesh tallies are spatially separate
    openmc::settings::assume_separate = true;
  }

  // warn user that this setting is present if other tallies existin
  // (in the tallies.xml for example)
  if (openmc::model::tallies.size() > _tallies.size()) {
    mooseWarning("Additional tallies exist in the problem. "
                 "If spatial overlaps exist, tally data may be inaccurate");
  }
}

double OpenMCProblem::kappa_fission_total() const
{
  auto tally_results = _kappa_fission_tally->results_;
  double total = tally_results(0, 0, static_cast<int>(openmc::TallyResult::SUM));
  return total;
}

void OpenMCProblem::addExternalVariables()
{
  auto var_params = _factory.getValidParams("MooseVariable");
  var_params.set<MooseEnum>("family") = "MONOMIAL";
  var_params.set<MooseEnum>("order") = "CONSTANT";

  addAuxVariable("MooseVariable", "heat_source", var_params);
  _heat_source_var = _aux->getFieldVariable<Real>(0, "heat_source").number();

  auto receiver_params = _factory.getValidParams("NearestPointReceiver");
  receiver_params.set<std::vector<Point>>("positions") = _centers;

  // Will receive values from the master
  addUserObject("NearestPointReceiver", "average_temp", receiver_params);

  // Find cell for each pebble center and get initial temperatures
  // as set on the OpenMC model
  std::vector<double> initial_temperatures;
  for (const auto & c : _centers)
  {
    openmc::Particle p {};
    p.r() = {c(0), c(1), c(2)};
    p.u() = {0., 0., 1.};

    if (!openmc::exhaustive_find_cell(p))
      mooseError("Cannot find OpenMC cell at position (x, y, z) = (" + Moose::stringify(c(0)) + ", " +
        Moose::stringify(c(1)) + ", " + Moose::stringify(c(2)) + ")");

    auto& cell = openmc::model::cells[p.coord(p.n_coord() - 1).cell];
    initial_temperatures.push_back(cell->temperature(p.cell_instance()));

  }

  // initialize the user object with these temperatures
  auto& average_temp = getUserObject<NearestPointReceiver>("average_temp");
  average_temp.setValues(initial_temperatures);
}

void
OpenMCProblem::initialSetup()
{
  ExternalProblem::initialSetup();

  int n_centers = _centers.size();
  _console << "Initializing OpenMC problem with " << Moose::stringify(n_centers) << " coupling points..." << std::endl;
}

void OpenMCProblem::externalSolve()
{
  int err = openmc_run();
  if (err) openmc::fatal_error(openmc_err_msg);
}

void OpenMCProblem::syncSolutions(ExternalProblem::Direction direction)
{
  switch (direction)
  {
    case ExternalProblem::Direction::TO_EXTERNAL_APP:
    {
      _console << "Sending temperature to OpenMC... ";

      // find the max/min temperatures sent to OpenMC (for diagnostics)
      Real maximum = std::numeric_limits<Real>::min();
      Real minimum = std::numeric_limits<Real>::max();

      auto & average_temp = getUserObject<NearestPointReceiver>("average_temp");

      for (std::size_t i = 0; i < _cellIndices.size(); ++i)
      {
        auto& cell = openmc::model::cells[_cellIndices[i]];
        double T = average_temp.spatialValue(_centers[i]);
         //std::cout << "Temperature at location: "
         //          << _centers[i](0) << ' '
         //          << _centers[i](1) << ' '
         //          << _centers[i](2) <<
         //          " Temp: " << T << std::endl;

        maximum = std::max(maximum, T);
        minimum = std::min(minimum, T);

        cell->set_temperature(T, _cellInstances[i], true);
      }

      /*
      for (auto& cell : openmc::model::cells) {
        if (cell->type_ == openmc::Fill::MATERIAL) {
          auto val = cell->sqrtkT_[0];
          // std::cout << val*val / openmc::K_BOLTZMANN << ' ';
        }
      }
      */

      _console << "done" << std::endl;

      _console << "Temperature min/max values from MOOSE: " << minimum << ", " << maximum << std::endl;

      break;
    }
    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    {
      _console << "Extracting heat source from OpenMC... ";

      auto& solution = _aux->solution();

      auto sys_number = _aux->number();

      // get the transfer mesh
      auto & transfer_mesh = _mesh.getMesh();

      switch (_tallyType)
      {
        case tally::cell:
        {
          auto cell_heat = cellHeatSource();
          // Debug
          // std::cout << "Cell heat source: " << std::endl;
          // for (auto val : cell_heat) { std::cout << val << " " << std::endl; }
          unsigned int elems_per_sphere = mesh().nElem() / _centers.size();

          for (unsigned int i = 0; i < _centers.size(); ++i ) {
            unsigned int offset = i * elems_per_sphere;
            for (unsigned int e = 0; e < elems_per_sphere; ++e) {
              auto elem_ptr = transfer_mesh.query_elem_ptr(offset + e);
              if (elem_ptr) {
                auto dof_idx = elem_ptr->dof_number(sys_number, _heat_source_var, 0);
                // set every element in this pebble with the same heating value
                solution.set(dof_idx, cell_heat.at(i));
              }
            }
          }

          break;
        }
        case tally::mesh:
        {
          auto mesh_heat = meshHeatSource();

          // Debug
          // std::cout << "Mesh heat source: " << std::endl;
          // for (auto val : mesh_heat) { std::cout << val << " " << std::endl; }


          // set the heat source directly on the mesh elements
          for (unsigned int i = 0; i < _meshFilters.size(); ++i) {
            auto& mesh_filter = _meshFilters[i];
            unsigned int offset = i * mesh_filter->n_bins();

            for (decltype(mesh_filter->n_bins()) e = 0; e < mesh_filter->n_bins(); ++e) {
              auto elem_ptr = transfer_mesh.query_elem_ptr(offset + e);
              if (elem_ptr) {
                auto dof_idx = elem_ptr->dof_number(sys_number, _heat_source_var, 0);
                solution.set(dof_idx, mesh_heat.at(offset + e));
              }
            }
          }

        _console << "done" << std::endl;

        break;
      }

      default:
        mooseError("Unhandled TallyTypeEnum in OpenMCProblem!");
    }

    solution.close();
    break;
    }
    default:
    {
      mooseError("Unhandled DirectionEnum in OpenMCProblem!");
      break;
    }
  }
}

void OpenMCProblem::checkTallySum(double tally_sum) const {
  double kf_total = kappa_fission_total();
  if ( std::abs(kf_total - tally_sum) / kf_total > openmc::FP_REL_PRECISION) {
    mooseError("Heating tally kappa-fission does not match the global kappa-fission value.\n"
               "Global value: " + Moose::stringify(kf_total) + "\nTally sum: " + Moose::stringify(tally_sum));
  }
}

std::vector<double> OpenMCProblem::meshHeatSource() {
  // determine the size of the heat source
  size_t heat_source_size = 0;
  for (const auto& t : _tallies) { heat_source_size += t->n_filter_bins(); }
  xt::xarray<double> heat = xt::zeros<double> ({heat_source_size});

  size_t idx = 0;
  double tally_sum = 0.0;
  for (std::size_t i = 0; i < _tallies.size(); i++) {
    const auto& tally = _tallies[i];
    // Determine number of realizations for normalizing tallies
    auto tally_mean = xt::view(tally->results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM));
    // double tally_sum = std::accumulate(tally_mean.begin(), tally_mean.end(), 0.0);

    int m = tally->n_realizations_;
    // TODO: Change OpenMC so that it's correct on all ranks
    MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);

    // normalize by volume
    for (int bin = 0; bin < tally->n_filter_bins(); bin++) {
      heat(idx) = tally_mean(bin) * JOULE_PER_EV / m;
      tally_sum += tally_mean(bin);
      idx++;
    }
  }

  if (_check_tally_sum) checkTallySum(tally_sum);

  double total_heat = xt::sum(heat)();
  double f = _power / total_heat;
  if (total_heat != 0.0) {
    // normalize heat generation using power level
    int idx = 0;
    for (const auto& tally : _tallies) {
      for (int bin = 0; bin < tally->n_filter_bins(); bin++){
        heat(idx) *=  f / _meshTemplate->volume(bin);
        idx++;
      }
    }
  } else {
    heat = xt::zeros_like(heat);
  }

  // std::cout << "Heat source: ";
  // for (auto val : heat) { std::cout << val << ' '; }
  // std::cout << std::endl;

  return std::vector<double>(heat.begin(), heat.end());
}

std::vector<double> OpenMCProblem::cellHeatSource()
{
  auto tally = _tallies.at(0);

  // Determine number of realizations for normalizing tallies
  int m = tally->n_realizations_;
  // Broadcast number of realizations
  // TODO: Change OpenMC so that it's correct on all ranks
  MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Determine energy production in each material
  auto meanValue = xt::view(tally->results_, xt::all(), 0, static_cast<int>(openmc::TallyResult::SUM));

  double tally_sum = xt::sum(meanValue)();

  if (_check_tally_sum) checkTallySum(tally_sum);

  xt::xarray<double> heat = meanValue;
  heat *= JOULE_PER_EV;
  heat /= m;

  // Get total heat production [J/source]
  double total_heat = xt::sum(heat)();

  if (total_heat != 0.0) {
    // Normalize heat source in each material and collect in an array
    for (std::size_t i = 0; i < _cellIndices.size(); ++i) {
      double V = _volumes[i];
      // Convert heat from [J/source] to [W/cm^3]. Dividing by total_heat gives
      // the fraction of heat deposited in each material. Multiplying by power
      // givens an absolute value in W
      heat(i) *= _power / (total_heat * V);
    }
  } else {
    heat = xt::zeros_like(heat);
  }

  // std::cout << "Heat source: ";
  // for (auto val : heat) { std::cout << val << ' '; }
  // std::cout << std::endl;

  return std::vector<double>(heat.begin(), heat.end());
}

double OpenMCProblem::getCellVolume(int cellIndex) {
  int fillType = static_cast<int>(openmc::Fill::MATERIAL);
  int32_t *matIndices = nullptr;
  int nMat = 0;
  openmc_cell_get_fill(cellIndex, &fillType, &matIndices, &nMat);
  double vol = 0;
  for (int i = 0; i < nMat; ++i) {
    double thisVol = 0;
    openmc_material_get_volume(matIndices[i], &thisVol);
    vol += thisVol;
  }
  return vol;
}
