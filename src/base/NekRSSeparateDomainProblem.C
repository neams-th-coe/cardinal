/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2021 UChicago Argonne, LLC                  */
/*                        ALL RIGHTS RESERVED                       */
/*                                                                  */
/*                 Prepared by UChicago Argonne, LLC                */
/*               Under Contract No. DE-AC02-06CH11357               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*                With the U. S. Department of Energy               */
/*                                                                  */
/*                 See LICENSE for full restrictions                */
/********************************************************************/

#ifdef ENABLE_NEK_COUPLING

#include "NekRSSeparateDomainProblem.h"

registerMooseObject("CardinalApp", NekRSSeparateDomainProblem);

extern nekrs::usrwrkIndices indices;

InputParameters
NekRSSeparateDomainProblem::validParams()
{
  InputParameters params = NekRSProblemBase::validParams();

  MultiMooseEnum coupling_types("inlet outlet");
  params.addRequiredParam<MultiMooseEnum>(
      "coupling_type", coupling_types, "NekRS boundary types to couple to a 1-D T/H code");

  MultiMooseEnum coupled_scalars("scalar01 scalar02 scalar03");
  params.addParam<MultiMooseEnum>(
      "coupled_scalars", coupled_scalars, "NekRS scalars to couple to a 1-D T/H code");

  params.addRequiredParam<std::vector<int>>("outlet_boundary", "NekRS outlet boundary ID");
  params.addRequiredParam<std::vector<int>>("inlet_boundary", "NekRS inlet boundary ID");

  return params;
}

NekRSSeparateDomainProblem::NekRSSeparateDomainProblem(const InputParameters & params)
  : NekRSProblemBase(params),
    _coupling_type(getParam<MultiMooseEnum>("coupling_type")),
    _outlet_boundary(getParam<std::vector<int>>("outlet_boundary")),
    _inlet_boundary(getParam<std::vector<int>>("inlet_boundary")),
    _inlet_coupling(false),
    _outlet_coupling(false),
    _coupled_scalars(getParam<MultiMooseEnum>("coupled_scalars")),
    _scalar01_coupling(false),
    _scalar02_coupling(false),
    _scalar03_coupling(false)
{
  if (_nek_mesh->exactMirror())
    mooseError("An exact mesh mirror is not yet supported for NekRSSeparateDomainProblem!");

  if (nekrs::hasMovingMesh())
    mooseWarning("NekRSSeparateDomainProblem currently does not transfer mesh displacements "
                 "from NekRS to Cardinal. The [Mesh] object in Cardinal won't reflect "
                 "NekRS's internal mesh changes. This may affect your postprocessor values.");

  // check scalar01-03_coupling provided
  for (const auto & s : _coupled_scalars)
  {
    if (s == "scalar01")
      _scalar01_coupling = true;
    if (s == "scalar02")
      _scalar02_coupling = true;
    if (s == "scalar03")
      _scalar03_coupling = true;
  }

  // check coupling_type provided
  for (const auto & c : _coupling_type)
  {
    if (c == "inlet")
      _inlet_coupling = true;
    if (c == "outlet")
      _outlet_coupling = true;
  }

  // check outlet boundary supplied
  if (_outlet_boundary.size() != 1)
  {
    mooseError("'outlet_boundary' can only have a single ID listed \n",
               "but 'outlet_boundary' has " + std::to_string(_outlet_boundary.size()) +
                   " IDs listed.");
  }
  else
  {
    int invalid_id, n_boundaries;
    bool valid_ids = nekrs::validBoundaryIDs(_outlet_boundary, invalid_id, n_boundaries);

    if (!valid_ids)
      mooseError("Invalid 'outlet_boundary' entry: ", invalid_id, "\n\n"
                 "NekRS assumes the boundary IDs are ordered contiguously beginning at 1. "
                 "For this problem, NekRS has ", n_boundaries, " boundaries. "
                 "Did you enter a valid 'outlet_boundary'?");
  }

  // check inlet boundary supplied
  if (_inlet_boundary.size() != 1)
  {
    mooseError("'inlet_boundary' can only have a single ID listed \n",
               "but 'inlet_boundary' has " + std::to_string(_inlet_boundary.size()) +
                   " IDs listed.");
  }
  else
  {
    int invalid_id, n_boundaries;
    bool valid_ids = nekrs::validBoundaryIDs(_inlet_boundary, invalid_id, n_boundaries);

    if (!valid_ids)
      mooseError("Invalid 'inlet_boundary' entry: ", invalid_id, "\n\n"
                 "NekRS assumes the boundary IDs are ordered contiguously beginning at 1. "
                 "For this problem, NekRS has ", n_boundaries, " boundaries. "
                 "Did you enter a valid 'inlet_boundary'?");
  }

  if (!_boundary)
    mooseError("In order to initialize the separate domain coupling, 'boundary' "
               "should contain,\nat a minimum, all boundaries listed in 'inlet_boundary'");

  // make sure that inlet boundary is in NekRSMesh boundary IDs provided
  if (std::find(_boundary->begin(), _boundary->end(), _inlet_boundary.front()) == _boundary->end())
    mooseError("Invalid 'inlet_boundary' entry: " + Moose::stringify(_inlet_boundary) + " \n",
               "'inlet_boundary' must be in 'boundary' supplied to NekRSMesh, but 'boundary' = " +
                   Moose::stringify(*_boundary) + ".");


  // TODO, add checks of BCs if using inlet coupling

  // Determine an appropriate default usrwrk indexing; the ordering will always be as
  // follows (except that unused terms will be deleted if not needed for coupling)
  //   velocity         (if _inlet_coupling is true)
  //   temperature      (if _inlet_coupling is true and NekRS has temperature solve)
  //   scalar01         (if _inlet_coupling is true and _scalar01_coupling is true)
  //   scalar02         (if _inlet_coupling is true and _scalar02_coupling is true)
  //   scalar03         (if _inlet_coupling is true and _scalar03_coupling is true)
  int start = _usrwrk_indices.size();
  if (_inlet_coupling)
  {
    indices.boundary_velocity = start++ * nekrs::scalarFieldOffset();
    _usrwrk_indices.push_back("velocity");

    if (nekrs::hasTemperatureSolve())
    {
      indices.boundary_temperature = start++ * nekrs::scalarFieldOffset();
      _usrwrk_indices.push_back("temperature");
    }

    if (_scalar01_coupling)
    {
      indices.boundary_scalar01 = start++ * nekrs::scalarFieldOffset();
      _usrwrk_indices.push_back("scalar01");
    }

    if (_scalar02_coupling)
    {
      indices.boundary_scalar02 = start++ * nekrs::scalarFieldOffset();
      _usrwrk_indices.push_back("scalar02");
    }

    if (_scalar03_coupling)
    {
      indices.boundary_scalar03 = start++ * nekrs::scalarFieldOffset();
      _usrwrk_indices.push_back("scalar03");
    }
  }

  _minimum_scratch_size_for_coupling = _usrwrk_indices.size() - _first_reserved_usrwrk_slot;
}

NekRSSeparateDomainProblem::~NekRSSeparateDomainProblem() { nekrs::freeScratch(); }

void
NekRSSeparateDomainProblem::initialSetup()
{
  if (nekrs::buildOnly())
    return;

  NekRSProblemBase::initialSetup();

  if (_inlet_coupling)
  {
    _toNekRS_velocity = &getPostprocessorValueByName("inlet_V");

    if (nekrs::hasTemperatureSolve())
      _toNekRS_temp = &getPostprocessorValueByName("inlet_T");

    if (_scalar01_coupling)
      _toNekRS_scalar01 = &getPostprocessorValueByName("inlet_S01");
    if (_scalar02_coupling)
      _toNekRS_scalar02 = &getPostprocessorValueByName("inlet_S02");
    if (_scalar03_coupling)
      _toNekRS_scalar03 = &getPostprocessorValueByName("inlet_S03");
  }
}

void
NekRSSeparateDomainProblem::syncSolutions(ExternalProblem::Direction direction)
{
  auto & solution = _aux->solution();

  if (!isDataTransferHappening(direction))
    return;

  switch (direction)
  {
    case ExternalProblem::Direction::TO_EXTERNAL_APP:
    {
      // transfer data to wrk array for NekRS inlet BCs
      if (_inlet_coupling)
      {
        sendBoundaryVelocityToNek(_pp_mesh);

        if (nekrs::hasTemperatureSolve())
          sendBoundaryTemperatureToNek(_pp_mesh);

        if (_scalar01_coupling)
          sendBoundaryScalarToNek(_pp_mesh, 1, *_toNekRS_scalar01);
        if (_scalar02_coupling)
          sendBoundaryScalarToNek(_pp_mesh, 2, *_toNekRS_scalar02);
        if (_scalar03_coupling)
          sendBoundaryScalarToNek(_pp_mesh, 3, *_toNekRS_scalar03);
      }

      sendScalarValuesToNek();

      // copy scratch to device
      copyScratchToDevice();

      break;
    }

    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    {
      // extract desired data from NekRS solution
      extractOutputs();
      break;
    }
    default:
      mooseError("Unhandled 'Transfer::DIRECTION' enum!");
  }

  solution.close();
  _aux->system().update();
}

void
NekRSSeparateDomainProblem::sendBoundaryVelocityToNek(const nek_mesh::NekMeshEnum pp_mesh)
{
  auto & solution = _aux->solution();
  auto sys_number = _aux->number();

  if (_first)
  {
    _serialized_solution->init(_aux->sys().n_dofs(), false, SERIAL);
    _first = false;
  }

  solution.localize(*_serialized_solution);

  auto & mesh = _nek_mesh->getMesh();

  _console << "Sending velocity of " << *_toNekRS_velocity << " to NekRS boundary "
           << Moose::stringify(_inlet_boundary) << std::endl;

  for (unsigned int e = 0; e < _n_surface_elems; e++)
    velocity(pp_mesh, e, *_toNekRS_velocity);
}

void
NekRSSeparateDomainProblem::sendBoundaryTemperatureToNek(const nek_mesh::NekMeshEnum pp_mesh)
{
  auto & solution = _aux->solution();
  auto sys_number = _aux->number();

  if (_first)
  {
    _serialized_solution->init(_aux->sys().n_dofs(), false, SERIAL);
    _first = false;
  }

  solution.localize(*_serialized_solution);

  auto & mesh = _nek_mesh->getMesh();

  _console << "Sending temperature of " << *_toNekRS_temp << " to NekRS boundary "
           << Moose::stringify(_inlet_boundary) << std::endl;

  for (unsigned int e = 0; e < _n_surface_elems; e++)
    temperature(pp_mesh, e, *_toNekRS_temp);
}

void
NekRSSeparateDomainProblem::sendBoundaryScalarToNek(const nek_mesh::NekMeshEnum pp_mesh,
                                                    const int scalarId,
                                                    const double scalarValue)
{
  auto & solution = _aux->solution();
  auto sys_number = _aux->number();

  if (_first)
  {
    _serialized_solution->init(_aux->sys().n_dofs(), false, SERIAL);
    _first = false;
  }

  solution.localize(*_serialized_solution);

  auto & mesh = _nek_mesh->getMesh();

  _console << "Sending scalar0" << Moose::stringify(scalarId) << " of "
           << scalarValue << " to NekRS boundary "
           << Moose::stringify(_inlet_boundary) << std::endl;

  for (unsigned int e = 0; e < _n_surface_elems; e++)
    scalar(pp_mesh, e, scalarId, scalarValue);
}

void
NekRSSeparateDomainProblem::addExternalVariables()
{
  NekRSProblemBase::addExternalVariables();

  // inlet NekRS pressure
  auto pp_params = _factory.getValidParams("NekSideAverage");
  pp_params.set<MooseEnum>("field") = "pressure";
  pp_params.set<std::vector<int>>("boundary") = _inlet_boundary;
  pp_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  pp_params.set<MooseEnum>("mesh") = _pp_mesh;
  addPostprocessor("NekSideAverage", "inlet_P", pp_params);

  // outlet NekRS pressure
  pp_params = _factory.getValidParams("NekSideAverage");
  pp_params.set<MooseEnum>("field") = "pressure";
  pp_params.set<std::vector<int>>("boundary") = _outlet_boundary;
  pp_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  pp_params.set<MooseEnum>("mesh") = _pp_mesh;
  addPostprocessor("NekSideAverage", "outlet_P", pp_params);

  // calculate pressure drop over NekRS
  pp_params = _factory.getValidParams("ParsedPostprocessor");
  pp_params.set<std::string>("function") = "inlet_P - outlet_P";
  pp_params.set<std::vector<PostprocessorName>>("pp_names") = {"outlet_P", "inlet_P"};
  pp_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  addPostprocessor("ParsedPostprocessor", "dP", pp_params);

  // NekRS -> 1d code, velocity interface
  if (_outlet_coupling)
  {
    auto pp_params = _factory.getValidParams("NekSideAverage");
    pp_params.set<MooseEnum>("field") = "velocity";
    pp_params.set<std::vector<int>>("boundary") = _outlet_boundary;
    pp_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
    pp_params.set<MooseEnum>("mesh") = _pp_mesh;
    addPostprocessor("NekSideAverage", "outlet_V", pp_params);

    // NekRS -> 1d code, temperature interface
    if (nekrs::hasTemperatureSolve())
    {
      auto pp_params = _factory.getValidParams("NekSideAverage");
      pp_params.set<MooseEnum>("field") = "temperature";
      pp_params.set<std::vector<int>>("boundary") = _outlet_boundary;
      pp_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
      pp_params.set<MooseEnum>("mesh") = _pp_mesh;
      addPostprocessor("NekSideAverage", "outlet_T", pp_params);
    }
    // NekRS -> 1d code, scalar interfaces
    if (_scalar01_coupling)
    {
      auto pp_params = _factory.getValidParams("NekSideAverage");
      pp_params.set<MooseEnum>("field") = "scalar01";
      pp_params.set<std::vector<int>>("boundary") = _outlet_boundary;
      pp_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
      pp_params.set<MooseEnum>("mesh") = _pp_mesh;
      addPostprocessor("NekSideAverage", "outlet_S01", pp_params);
    }
    if (_scalar02_coupling)
    {
      auto pp_params = _factory.getValidParams("NekSideAverage");
      pp_params.set<MooseEnum>("field") = "scalar02";
      pp_params.set<std::vector<int>>("boundary") = _outlet_boundary;
      pp_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
      pp_params.set<MooseEnum>("mesh") = _pp_mesh;
      addPostprocessor("NekSideAverage", "outlet_S02", pp_params);
    }
    if (_scalar03_coupling)
    {
      auto pp_params = _factory.getValidParams("NekSideAverage");
      pp_params.set<MooseEnum>("field") = "scalar03";
      pp_params.set<std::vector<int>>("boundary") = _outlet_boundary;
      pp_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
      pp_params.set<MooseEnum>("mesh") = _pp_mesh;
      addPostprocessor("NekSideAverage", "outlet_S03", pp_params);
    }
  }

  // 1d code -> NekRS, velocity interface
  if (_inlet_coupling)
  {
    auto pp_params = _factory.getValidParams("Receiver");
    addPostprocessor("Receiver", "inlet_V", pp_params);

    // 1d code -> NekRS, temperature interface
    if (nekrs::hasTemperatureSolve())
      addPostprocessor("Receiver", "inlet_T", pp_params);

    // 1d code -> NekRS, scalar interfaces
    if (_scalar01_coupling)
      addPostprocessor("Receiver", "inlet_S01", pp_params);
    if (_scalar02_coupling)
      addPostprocessor("Receiver", "inlet_S02", pp_params);
    if (_scalar03_coupling)
      addPostprocessor("Receiver", "inlet_S03", pp_params);
  }
}

void
NekRSSeparateDomainProblem::velocity(const nek_mesh::NekMeshEnum pp_mesh,
                                     const int elem_id,
                                     const double velocity)
{
  const auto & bc = _nek_mesh->boundaryCoupling();

  // We can only write into the nekRS scratch space if that face is "owned" by the current process
  if (nekrs::commRank() == bc.processor_id(elem_id))
  {
    nrs_t * nrs = (nrs_t *)nekrs::nrsPtr();
    mesh_t * mesh = nekrs::getMesh(pp_mesh);

    int end_1d = mesh->Nq;
    int end_2d = end_1d * end_1d;

    int e = bc.element[elem_id];
    int f = bc.face[elem_id];

    int offset = e * mesh->Nfaces * mesh->Nfp + f * mesh->Nfp;
    for (int i = 0; i < end_2d; ++i)
    {
      int id = mesh->vmapM[offset + i];
      nrs->usrwrk[indices.boundary_velocity + id] = velocity; // send single velocity value to NekRS
    }
  }
}

void
NekRSSeparateDomainProblem::temperature(const nek_mesh::NekMeshEnum pp_mesh,
                                        const int elem_id,
                                        const double temperature)
{
  const auto & bc = _nek_mesh->boundaryCoupling();

  // We can only write into the nekRS scratch space if that face is "owned" by the current process
  if (nekrs::commRank() == bc.processor_id(elem_id))
  {
    nrs_t * nrs = (nrs_t *)nekrs::nrsPtr();
    mesh_t * mesh = nekrs::getMesh(pp_mesh);

    int end_1d = mesh->Nq;
    int end_2d = end_1d * end_1d;

    int e = bc.element[elem_id];
    int f = bc.face[elem_id];

    int offset = e * mesh->Nfaces * mesh->Nfp + f * mesh->Nfp;
    for (int i = 0; i < end_2d; ++i)
    {
      int id = mesh->vmapM[offset + i];
      nrs->usrwrk[indices.boundary_temperature + id] = temperature; // send single temperature value to NekRS
    }
  }
}

void
NekRSSeparateDomainProblem::scalar(const nek_mesh::NekMeshEnum pp_mesh,
                                   const int elem_id,
                                   const int scalarId,
                                   const double scalar)
{
  const auto & bc = _nek_mesh->boundaryCoupling();

  // We can only write into the nekRS scratch space if that face is "owned" by the current process
  if (nekrs::commRank() == bc.processor_id(elem_id))
  {
    nrs_t * nrs = (nrs_t *)nekrs::nrsPtr();
    mesh_t * mesh = nekrs::getMesh(pp_mesh);

    std::vector<int> offsets = {indices.boundary_scalar01, indices.boundary_scalar02, indices.boundary_scalar03};

    int end_1d = mesh->Nq;
    int end_2d = end_1d * end_1d;

    int e = bc.element[elem_id];
    int f = bc.face[elem_id];

    int offset = e * mesh->Nfaces * mesh->Nfp + f * mesh->Nfp;
    for (int i = 0; i < end_2d; ++i)
    {
      int id = mesh->vmapM[offset + i];
      nrs->usrwrk[offsets[scalarId - 1] + id] = scalar; // send single scalar value to NekRS
    }
  }
}

#endif
