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

#include "NekRSSeparateDomainProblem.h"
#include "Moose.h"
#include "AuxiliarySystem.h"
#include "TimeStepper.h"
#include "NekInterface.h"
#include "TimedPrint.h"
#include "MooseUtils.h"
#include "CardinalUtils.h"

#include "nekrs.hpp"
#include "nekInterface/nekInterfaceAdapter.hpp"

registerMooseObject("CardinalApp", NekRSSeparateDomainProblem);

bool NekRSSeparateDomainProblem::_first = true;

InputParameters
NekRSSeparateDomainProblem::validParams()
{
  InputParameters params = NekRSProblemBase::validParams();
  params.addParam<bool>("minimize_transfers_in", false, "Whether to only synchronize nekRS "
    "for the direction TO_EXTERNAL_APP on multiapp synchronization steps");
  params.addParam<bool>("minimize_transfers_out", false, "Whether to only synchronize nekRS "
    "for the direction FROM_EXTERNAL_APP on multiapp synchronization steps");
  params.addParam<bool>("moving_mesh", false, "Moving mesh");
  params.addParam<bool>("toNekRS_interface", false, "External app -> NekRS interface present?");
  params.addParam<bool>("toNekRS_temperature", false, "External app -> NekRS temperature transfer?");
  params.addParam<bool>("fromNekRS_interface", false, "NekRS -> external app interface present?");
  params.addParam<bool>("fromNekRS_temperature", false, "NekRS -> external app temperature transfer?");
  params.addRequiredParam<std::vector<int>>("outlet_boundary", "NekRS outlet boundary ID");
  params.addRequiredParam<std::vector<int>>("inlet_boundary", "NekRS inlet boundary ID");

  return params;
}

NekRSSeparateDomainProblem::NekRSSeparateDomainProblem(const InputParameters &params) : NekRSProblemBase(params),
    _serialized_solution(NumericVector<Number>::build(_communicator).release()), 
    _moving_mesh(getParam<bool>("moving_mesh")),
    _minimize_transfers_in(getParam<bool>("minimize_transfers_in")),
    _minimize_transfers_out(getParam<bool>("minimize_transfers_out")),
    _toNekRS(getParam<bool>("toNekRS_interface")),
    _toNekRS_temperature(getParam<bool>("toNekRS_temperature")),
    _fromNekRS(getParam<bool>("fromNekRS_interface")),
    _fromNekRS_temperature(getParam<bool>("fromNekRS_temperature")),
    _outlet_boundary(&getParam<std::vector<int>>("outlet_boundary")),
    _inlet_boundary(&getParam<std::vector<int>>("inlet_boundary"))
{

  if (!_toNekRS && !_fromNekRS)
    mooseError("This problem type needs atleast one of 'toNekRS_interface' \n", 
    "or 'fromNekRS_interface' to be set to true.");

  // check outlet boundary supplied
  if (_outlet_boundary->size() != 1)
    mooseError("'outlet_boundary' can only have a single ID listed \n",
    "but 'outlet_boundary' has " + std::to_string(_outlet_boundary->size()) + " IDs listed.");

  else {
    int invalid_id, n_boundaries;
    bool valid_ids = nekrs::mesh::validBoundaryIDs(*_outlet_boundary, invalid_id, n_boundaries);
  
    if (!valid_ids)
      mooseError("Invalid 'outlet_boundary' entry: ", invalid_id, "\n\n"
        "NekRS assumes the boundary IDs are ordered contiguously beginning at 1. "
        "For this problem, NekRS has ", n_boundaries, " boundaries. "
        "Did you enter a valid 'outlet_boundary'?");
  }

  // check inlet boundary supplied
  if (_inlet_boundary->size() != 1)
    mooseError("'inlet_boundary' can only have a single ID listed \n",
    "but 'inlet_boundary' has " + std::to_string(_inlet_boundary->size()) + " IDs listed.");

  else {
    int invalid_id, n_boundaries;
    bool valid_ids = nekrs::mesh::validBoundaryIDs(*_inlet_boundary, invalid_id, n_boundaries);
  
    if (!valid_ids)
      mooseError("Invalid 'inlet_boundary' entry: ", invalid_id, "\n\n"
        "NekRS assumes the boundary IDs are ordered contiguously beginning at 1. "
        "For this problem, NekRS has ", n_boundaries, " boundaries. "
        "Did you enter a valid 'inlet_boundary'?");
  }

  // make sure that inlet boundary is in NekRSMesh boundary IDs provided
  if( std::find(_boundary->begin(), _boundary->end(), _inlet_boundary->front()) == _boundary->end() )
      mooseError("Invalid 'inlet_boundary' entry: " + Moose::stringify(*_inlet_boundary) + " \n",
      "'inlet_boundary' must be in 'boundary' supplied to NekRSMesh, but 'boundary' = " + Moose::stringify(*_boundary) + ".");

}

NekRSSeparateDomainProblem::~NekRSSeparateDomainProblem()
{
  nekrs::freeScratch();

}

void
NekRSSeparateDomainProblem::initialSetup()
{
  if (nekrs::buildOnly())
    return;

  NekRSProblemBase::initialSetup();

  if (_minimize_transfers_in)
    _transfer_in = &getPostprocessorValueByName("transfer_in");

  if (_toNekRS)
    _toNekRS_velocity = &getPostprocessorValueByName("inlet_V");

  if (_toNekRS_temperature)
    _toNekRS_temp = &getPostprocessorValueByName("inlet_T");


}

void NekRSSeparateDomainProblem::syncSolutions(ExternalProblem::Direction direction)
{
  if (nekrs::buildOnly())
    return;

  switch(direction)
  {
    case ExternalProblem::Direction::TO_EXTERNAL_APP:
    {
      if (!synchronizeIn())
        return;

      if (_toNekRS)
        sendBoundaryVelocityToNek();

      if (_toNekRS_temperature)
        sendBoundaryTemperatureToNek();

      // copy scratch to device
      nekrs::copyScratchToDevice();
      
      break;
    }

    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    {
      if (!synchronizeOut())
        return;

      extractOutputs();

      break;
    }
    default:
      mooseError("Unhandled 'Transfer::DIRECTION' enum!");
  }
}

bool
NekRSSeparateDomainProblem::synchronizeIn()
{
  bool synchronize = true;
  static bool first = true;

  if (_minimize_transfers_in)
  {
    // comments from NekRSProblem
    if (first && *_transfer_in == false)
      mooseError("The default value for the 'transfer_in' postprocessor received by nekRS "
        "must not be false! Make sure that the master application's "
        "postprocessor is not zero.");

    if (*_transfer_in == false)
      synchronize = false;
    else
      setPostprocessorValueByName("transfer_in", false, 0);
  }

  first = false;
  return synchronize;
}

bool
NekRSSeparateDomainProblem::synchronizeOut()
{
  bool synchronize = true;

  if (_minimize_transfers_out)
  {
    if (std::abs(_time - _dt - _transient_executioner->getTargetTime()) > _transient_executioner->timestepTol())
      synchronize = false;
  }

  return synchronize;
}


void
NekRSSeparateDomainProblem::sendBoundaryVelocityToNek()
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

  _console << "Sending velocity to NekRS boundary " << Moose::stringify(*_boundary) << std::endl;

  for (unsigned int e = 0; e < _n_surface_elems; e++)
    {
      velocity(e, *_toNekRS_velocity);
    }

  _console << "done" << std::endl;
}

void
NekRSSeparateDomainProblem::sendBoundaryTemperatureToNek()
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

  _console << "Sending temperature to NekRS boundary " << Moose::stringify(*_boundary) << std::endl;

  for (unsigned int e = 0; e < _n_surface_elems; e++)
    {
      temperature(e, *_toNekRS_temp);
    }

  _console << "done" << std::endl;
}

void
NekRSSeparateDomainProblem::addExternalVariables()
{
  NekRSProblemBase::addExternalVariables();
  auto var_params = getExternalVariableParameters(); //not needed?

  // inlet NekRS pressure
  auto pp_params = _factory.getValidParams("NekSideAverage");
  pp_params.set<MooseEnum>("field")= "pressure"; 
  pp_params.set<std::vector<int>>("boundary") = *_inlet_boundary;
  pp_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  addPostprocessor("NekSideAverage", "inlet_P", pp_params);

  // outlet NekRS pressure
  pp_params = _factory.getValidParams("NekSideAverage");
  pp_params.set<MooseEnum>("field")= "pressure"; 
  pp_params.set<std::vector<int>>("boundary") = *_outlet_boundary;
  pp_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  addPostprocessor("NekSideAverage", "outlet_P", pp_params);

  // calculate pressure drop over NekRS
  pp_params = _factory.getValidParams("ParsedPostprocessor");
  pp_params.set<std::string>("function") = "outlet_P + inlet_P";
  pp_params.set<std::vector<PostprocessorName>>("pp_names") = {"outlet_P", "inlet_P"};
  pp_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  addPostprocessor("ParsedPostprocessor", "dP", pp_params);

  // NekRS -> 1d code, velocity interface
  if (_fromNekRS)
  {
    auto pp_params = _factory.getValidParams("NekSideAverage");
    pp_params.set<MooseEnum>("field")= "velocity"; 
    pp_params.set<std::vector<int>>("boundary") = *_outlet_boundary;
    pp_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
    addPostprocessor("NekSideAverage", "outlet_V", pp_params);
  }

  // NekRS -> 1d code, temperature interface
  if (_fromNekRS_temperature)
  {
    auto pp_params = _factory.getValidParams("NekSideAverage");
    pp_params.set<MooseEnum>("field")= "temperature"; 
    pp_params.set<std::vector<int>>("boundary") = *_outlet_boundary;
    pp_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
    addPostprocessor("NekSideAverage", "outlet_T", pp_params);
  }

  // 1d code -> NekRS, velocity interface
  if (_toNekRS)
  {
    auto pp_params = _factory.getValidParams("Receiver");
    addPostprocessor("Receiver", "inlet_V", pp_params);
  }

  // 1d code -> NekRS, temperature interface
  if (_toNekRS_temperature)
  {
    auto pp_params = _factory.getValidParams("Receiver");
    addPostprocessor("Receiver", "inlet_T", pp_params);
  }
}

void
NekRSSeparateDomainProblem::velocity(const int elem_id, const double velocity_1dCode)
{

  const auto & bc = _nek_mesh->boundaryCoupling();

  // We can only write into the nekRS scratch space if that face is "owned" by the current process
  if (nekrs::commRank() == bc.processor_id(elem_id))
  {
    nrs_t * nrs = (nrs_t *) nekrs::nrsPtr();
    mesh_t * mesh = nekrs::entireMesh();

    int end_1d = mesh->Nq;
    int start_1d = _nek_mesh->order() + 2;
    int end_2d = end_1d * end_1d;

    int e = bc.element[elem_id];
    int f = bc.face[elem_id];

    int offset = e * mesh->Nfaces * mesh->Nfp + f * mesh->Nfp;
    for (int i = 0; i < end_2d; ++i)
    {
      int id = mesh->vmapM[offset + i];
      nrs->usrwrk[id] = velocity_1dCode; // send single velocity value to NekRS
    }
  }
}

void
NekRSSeparateDomainProblem::temperature(const int elem_id, const double temperature_1dCode)
{

  const auto & bc = _nek_mesh->boundaryCoupling();

  // We can only write into the nekRS scratch space if that face is "owned" by the current process
  if (nekrs::commRank() == bc.processor_id(elem_id))
  {
    nrs_t * nrs = (nrs_t *) nekrs::nrsPtr();
    mesh_t * mesh = nekrs::temperatureMesh();

    int scalarFieldOffset = nekrs::scalarFieldOffset();

    int end_1d = mesh->Nq;
    int start_1d = _nek_mesh->order() + 2;
    int end_2d = end_1d * end_1d;

    int e = bc.element[elem_id];
    int f = bc.face[elem_id];

    int offset = e * mesh->Nfaces * mesh->Nfp + f * mesh->Nfp;
    for (int i = 0; i < end_2d; ++i)
    {
      int id = mesh->vmapM[offset + i];
      nrs->usrwrk[id + scalarFieldOffset] = temperature_1dCode; // send single temperature value to NekRS
    }
  }
}
