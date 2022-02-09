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
  params.addRequiredParam<std::vector<FileName>>("ExternalApp_filename", "External app input file name");
  MooseEnum app_type("SamApp THMApp");
  params.addRequiredParam<MooseEnum>("ExternalApp_type", app_type, "External app type");
  params.addParam<std::vector<int>>("fromNekRS_boundary", "Boundary ID through which nekRS will be coupled to external app");
  params.addParam<std::vector<int>>("NekRS_inlet_boundary", "NekRS Boundary ID for Pressure drop calculation");

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
    _ExternalApp_filename(getParam<std::vector<FileName>>("ExternalApp_filename")),
    _ExternalApp_type(getParam<MooseEnum>("ExternalApp_type")),
    _fromNekRS_boundary(isParamValid("fromNekRS_boundary") ? &getParam<std::vector<int>>("fromNekRS_boundary") : nullptr),
    _NekRS_inlet_boundary(isParamValid("NekRS_inlet_boundary") ? &getParam<std::vector<int>>("NekRS_inlet_boundary") : nullptr)
{

  if (!_toNekRS && !_fromNekRS)
    mooseError("This problem type needs atleast one of toNekRS_interface \n", 
    "or fromNekRS_interface to be set to true\n");

  if (_fromNekRS)
    {
    if (_fromNekRS_boundary->size() != 1)
      mooseError("fromNekRS_boundary can only have a single ID listed \n",
      "but fromNekRS_boundary has " + std::to_string(_fromNekRS_boundary->size()) + " IDs listed.");
    }

  if (_NekRS_inlet_boundary->size() != 1)
    mooseError("NekRS_inlet_boundary can only have a single ID listed \n",
    "but NekRS_inlet_boundary has " + std::to_string(_NekRS_inlet_boundary->size()) + " IDs listed.");

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
    _toNekRS_velocity = &getPostprocessorValueByName("toNekRS_velocity");

  if (_toNekRS_temperature)
    _toNekRS_temp = &getPostprocessorValueByName("toNekRS_temperature");


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
//      nekrs::velocity(e, _nek_mesh->order(), _toNekRS_velocity);
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
//      nekrs::temperature(e, _nek_mesh->order(), _toNekRS_temp);
    }

  _console << "done" << std::endl;
}

void
NekRSSeparateDomainProblem::addExternalVariables()
{
  NekRSProblemBase::addExternalVariables();
  auto var_params = getExternalVariableParameters(); //not needed?

  // create MultiApp for external app
  auto multiapp_params = _factory.getValidParams("TransientMultiApp");
  multiapp_params.set<MooseEnum>("app_type") = _ExternalApp_type;
  multiapp_params.set<std::vector<FileName>>("input_files") = _ExternalApp_filename;
  multiapp_params.set<unsigned int>("max_procs_per_app") = 1; // only let external app run with one processor
  multiapp_params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN; // run external app first
  addMultiApp("TransientMultiApp", "ExternalApp", multiapp_params);

  // create NekRS pressure drop postprocessor
  auto pp_params = _factory.getValidParams("NekSideAverage");
  pp_params.set<MooseEnum>("field")= "pressure"; 
  pp_params.set<std::vector<int>>("boundary") = *_NekRS_inlet_boundary;
  pp_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  addPostprocessor("NekSideAverage", "NekRS_pressureDrop", pp_params);

  // create Transfer for NekRS pressure drop to external app
  auto trans_params = _factory.getValidParams("MultiAppPostprocessorTransfer");
  trans_params.set<MultiAppName>("multi_app") = "ExternalApp";
  trans_params.set<MultiMooseEnum>("direction") = "to_multiapp";
  trans_params.set<PostprocessorName>("from_postprocessor") = "NekRS_pressureDrop";
  trans_params.set<PostprocessorName>("to_postprocessor") = "NekRS_pressureDrop"; // requires this PP in external app input file
  addTransfer("MultiAppPostprocessorTransfer", "NekRS_pressureDrop_trans", trans_params);


  // NekRS -> external app interface
  if (_fromNekRS)
  {
    auto pp_params = _factory.getValidParams("NekSideAverage");
    pp_params.set<MooseEnum>("field")= "velocity"; 
    pp_params.set<std::vector<int>>("boundary") = *_fromNekRS_boundary;
    pp_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
    addPostprocessor("NekSideAverage", "fromNekRS_velocity", pp_params);

    // create Transfer for NekRS velocity to external app
    auto trans_params = _factory.getValidParams("MultiAppPostprocessorTransfer");
    trans_params.set<MultiAppName>("multi_app") = "ExternalApp";
    trans_params.set<MultiMooseEnum>("direction") = "to_multiapp";
    trans_params.set<PostprocessorName>("from_postprocessor") = "fromNekRS_velocity";
    trans_params.set<PostprocessorName>("to_postprocessor") = "fromNekRS_velocity";  // requires this PP in external app input file
    addTransfer("MultiAppPostprocessorTransfer", "fromNekRS_velocity_trans", trans_params);
  }

  // NekRS -> external app temperature interface
  if (_fromNekRS_temperature)
  {
    auto pp_params = _factory.getValidParams("NekSideAverage");
    pp_params.set<MooseEnum>("field")= "temperature"; 
    pp_params.set<std::vector<int>>("boundary") = *_fromNekRS_boundary;
    pp_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
    addPostprocessor("NekSideAverage", "fromNekRS_temperature", pp_params);

    // create Transfer for NekRS temperature to external app
    auto trans_params = _factory.getValidParams("MultiAppPostprocessorTransfer");
    trans_params.set<MultiAppName>("multi_app") = "ExternalApp";
    trans_params.set<MultiMooseEnum>("direction") = "to_multiapp";
    trans_params.set<PostprocessorName>("from_postprocessor") = "fromNekRS_temperature";
    trans_params.set<PostprocessorName>("to_postprocessor") = "fromNekRS_temperature";  // requires this PP in external app input file
    addTransfer("MultiAppPostprocessorTransfer", "fromNekRS_temperature_trans", trans_params);
  }

  // external app -> NekRS interface
  if (_toNekRS)
  {
    auto pp_params = _factory.getValidParams("Receiver");
    addPostprocessor("Receiver", "toNekRS_velocity", pp_params);

    // create Transfer for external app velocity to NekRS
    auto trans_params = _factory.getValidParams("MultiAppPostprocessorTransfer");
    trans_params.set<MultiAppName>("multi_app") = "ExternalApp";
    trans_params.set<MultiMooseEnum>("direction") = "from_multiapp";
    trans_params.set<MooseEnum>("reduction_type") = "average";
    trans_params.set<PostprocessorName>("from_postprocessor") = "toNekRS_velocity"; // requires this PP in external app input file
    trans_params.set<PostprocessorName>("to_postprocessor") = "toNekRS_velocity";
    addTransfer("MultiAppPostprocessorTransfer", "toNekRS_velocity_trans", trans_params);

  }

  // external app -> NekRS temperature interface
  if (_toNekRS_temperature)
  {
    auto pp_params = _factory.getValidParams("Receiver");
    addPostprocessor("Receiver", "toNekRS_temperature", pp_params);

    // create Transfer for external app temperature to NekRS
    auto trans_params = _factory.getValidParams("MultiAppPostprocessorTransfer");
    trans_params.set<MultiAppName>("multi_app") = "ExternalApp";
    trans_params.set<MultiMooseEnum>("direction") = "from_multiapp";
    trans_params.set<MooseEnum>("reduction_type") = "average";
    trans_params.set<PostprocessorName>("from_postprocessor") = "toNekRS_temperature"; // requires this PP in external app input file
    trans_params.set<PostprocessorName>("to_postprocessor") = "toNekRS_temperature";
    addTransfer("MultiAppPostprocessorTransfer", "toNekRS_temperature_trans", trans_params);
  }

}
