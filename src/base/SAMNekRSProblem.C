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

#include "SAMNekRSProblem.h"
#include "Moose.h"
#include "AuxiliarySystem.h"
#include "TimeStepper.h"
#include "NekInterface.h"
#include "TimedPrint.h"
#include "MooseUtils.h"
#include "CardinalUtils.h"

#include "nekrs.hpp"
#include "nekInterface/nekInterfaceAdapter.hpp"

registerMooseObject("CardinalApp", SAMNekRSProblem);

bool SAMNekRSProblem::_first = true;

InputParameters
SAMNekRSProblem::validParams()
{
  InputParameters params = NekRSProblemBase::validParams();
  params.addParam<bool>("minimize_transfers_in", false, "Whether to only synchronize nekRS "
    "for the direction TO_EXTERNAL_APP on multiapp synchronization steps");
  params.addParam<bool>("minimize_transfers_out", false, "Whether to only synchronize nekRS "
    "for the direction FROM_EXTERNAL_APP on multiapp synchronization steps");
  params.addParam<bool>("moving_mesh", false, "Moving mesh");
  params.addParam<bool>("SAMtoNekRS_interface", false, "SAM -> NekRS interface present?");
  params.addParam<bool>("SAMtoNekRS_temperature", false, "SAM -> NekRS temperature transfer?");
  params.addParam<bool>("NekRStoSAM_interface", false, "NekRS -> SAM interface present?");
  params.addParam<bool>("NekRStoSAM_temperature", false, "NekRS -> SAM temperature transfer?");
  params.addRequiredParam<std::vector<FileName>>("SAM_filename", "SAM input file name");
  params.addParam<std::vector<int>>("NekRStoSAM_boundary", "Boundary ID through which nekRS will be coupled to SAM");
  params.addParam<std::vector<int>>("NekRS_inlet_boundary", "NekRS Boundary ID for Pressure drop calculation");

  return params;
}

SAMNekRSProblem::SAMNekRSProblem(const InputParameters &params) : NekRSProblemBase(params),
    _serialized_solution(NumericVector<Number>::build(_communicator).release()), 
    _moving_mesh(getParam<bool>("moving_mesh")),
    _minimize_transfers_in(getParam<bool>("minimize_transfers_in")),
    _minimize_transfers_out(getParam<bool>("minimize_transfers_out")),
    _SAMtoNekRS(getParam<bool>("SAMtoNekRS_interface")),
    _SAMtoNekRS_temperature(getParam<bool>("SAMtoNekRS_temperature")),
    _NekRStoSAM(getParam<bool>("NekRStoSAM_interface")),
    _NekRStoSAM_temperature(getParam<bool>("NekRStoSAM_temperature")),
    _SAM_filename(getParam<std::vector<FileName>>("SAM_filename")),
    _NekRStoSAM_boundary(isParamValid("NekRStoSAM_boundary") ? &getParam<std::vector<int>>("NekRStoSAM_boundary") : nullptr),
    _NekRS_inlet_boundary(isParamValid("NekRS_inlet_boundary") ? &getParam<std::vector<int>>("NekRS_inlet_boundary") : nullptr)
{
}

SAMNekRSProblem::~SAMNekRSProblem()
{
  nekrs::freeScratch();

}

void
SAMNekRSProblem::initialSetup()
{
  if (nekrs::buildOnly())
    return;

  NekRSProblemBase::initialSetup();

  if (_minimize_transfers_in)
    _transfer_in = &getPostprocessorValueByName("transfer_in");
 
  if (!_SAMtoNekRS && !_NekRStoSAM)
    mooseError("SAMNekRSProblem requires specification of how data\n",
    "is being sent between SAM and NekRS, atleast one of SAMtoNekRS_interface \n", 
    "or NekRStoSAM_interface must be set to true\n");

  if (_SAMtoNekRS)
    _SAMtoNekRS_velocity = &getPostprocessorValueByName("SAMtoNekRS_velocity");

  if (_SAMtoNekRS_temperature)
    _SAMtoNekRS_temp = &getPostprocessorValueByName("SAMtoNekRS_temperature");


}

void SAMNekRSProblem::syncSolutions(ExternalProblem::Direction direction)
{
  if (nekrs::buildOnly())
    return;

  switch(direction)
  {
    case ExternalProblem::Direction::TO_EXTERNAL_APP:
    {
      if (!synchronizeIn())
        return;

      if (_SAMtoNekRS)
        sendBoundaryVelocityToNek();

      if (_SAMtoNekRS_temperature)
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
SAMNekRSProblem::synchronizeIn()
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
SAMNekRSProblem::synchronizeOut()
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
SAMNekRSProblem::sendBoundaryVelocityToNek()
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

  _console << "Sending velocity to NekRS for boundary " << Moose::stringify(*_boundary) << std::endl;

  for (unsigned int e = 0; e < _n_surface_elems; e++)
    {
      nekrs::velocity(e, _nek_mesh->order(), _SAMtoNekRS_velocity);
    }

  _console << "done" << std::endl;
}

void
SAMNekRSProblem::sendBoundaryTemperatureToNek()
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

  _console << "Sending temperature to NekRS for boundary " << Moose::stringify(*_boundary) << std::endl;

  for (unsigned int e = 0; e < _n_surface_elems; e++)
    {
      nekrs::temperature(e, _nek_mesh->order(), _SAMtoNekRS_temp);
    }

  _console << "done" << std::endl;
}

void
SAMNekRSProblem::addExternalVariables()
{
  NekRSProblemBase::addExternalVariables();
  auto var_params = getExternalVariableParameters();

  // create MultiApp for SAM
  auto multiapp_params = _factory.getValidParams("TransientMultiApp");
  multiapp_params.set<MooseEnum>("app_type") = "SamApp";
  multiapp_params.set<std::vector<FileName>>("input_files") = _SAM_filename;
  multiapp_params.set<unsigned int>("max_procs_per_app") = 1; // only let SAM run with one processor
  multiapp_params.set<ExecFlagEnum>("execute_on") = EXEC_TIMESTEP_BEGIN; // run SAM first
  addMultiApp("TransientMultiApp", "SAM", multiapp_params);

  // create NekRS pressure drop postprocessor
  auto pp_params = _factory.getValidParams("NekSideAverage");
  pp_params.set<MooseEnum>("field")= "pressure"; 
  pp_params.set<std::vector<int>>("boundary") = *_NekRS_inlet_boundary;
  pp_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  addPostprocessor("NekSideAverage", "NekRS_pressureDrop", pp_params);

  // create Transfer for NekRS pressure drop to SAM
  auto trans_params = _factory.getValidParams("MultiAppPostprocessorTransfer");
  trans_params.set<MultiAppName>("multi_app") = "SAM";
  trans_params.set<MultiMooseEnum>("direction") = "to_multiapp";
  trans_params.set<PostprocessorName>("from_postprocessor") = "NekRS_pressureDrop"; // requires this PP in SAM input file
  trans_params.set<PostprocessorName>("to_postprocessor") = "NekRS_pressureDrop";
  addTransfer("MultiAppPostprocessorTransfer", "NekRS_pressureDrop_trans", trans_params);


  // NekRS -> SAM interface
  if (_NekRStoSAM)
  {
    auto pp_params = _factory.getValidParams("NekSideAverage");
    pp_params.set<MooseEnum>("field")= "velocity"; 
    pp_params.set<std::vector<int>>("boundary") = *_NekRStoSAM_boundary;
    pp_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
    addPostprocessor("NekSideAverage", "NekRStoSAM_velocity", pp_params);

    // create Transfer for NekRS velocity to SAM
    auto trans_params = _factory.getValidParams("MultiAppPostprocessorTransfer");
    trans_params.set<MultiAppName>("multi_app") = "SAM";
    trans_params.set<MultiMooseEnum>("direction") = "to_multiapp";
    trans_params.set<PostprocessorName>("from_postprocessor") = "NekRStoSAM_velocity";
    trans_params.set<PostprocessorName>("to_postprocessor") = "NekRStoSAM_velocity";  // requires this PP in SAM input file
    addTransfer("MultiAppPostprocessorTransfer", "NekRStoSAM_velocity_trans", trans_params);
  }

  // NekRS -> SAM temperature interface
  if (_NekRStoSAM_temperature)
  {
    auto pp_params = _factory.getValidParams("NekSideAverage");
    pp_params.set<MooseEnum>("field")= "temperature"; 
    pp_params.set<std::vector<int>>("boundary") = *_NekRStoSAM_boundary;
    pp_params.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_END};
    addPostprocessor("NekSideAverage", "NekRStoSAM_temperature", pp_params);

    // create Transfer for NekRS temperature to SAM
    auto trans_params = _factory.getValidParams("MultiAppPostprocessorTransfer");
    trans_params.set<MultiAppName>("multi_app") = "SAM";
    trans_params.set<MultiMooseEnum>("direction") = "to_multiapp";
    trans_params.set<PostprocessorName>("from_postprocessor") = "NekRStoSAM_temperature";
    trans_params.set<PostprocessorName>("to_postprocessor") = "NekRStoSAM_temperature";  // requires this PP in SAM input file
    addTransfer("MultiAppPostprocessorTransfer", "NekRStoSAM_temperature_trans", trans_params);
  }

  // SAM -> NekRS interface
  if (_SAMtoNekRS)
  {
    auto pp_params = _factory.getValidParams("Receiver");
    addPostprocessor("Receiver", "SAMtoNekRS_velocity", pp_params);

    // create Transfer for SAM velocity to NekRS
    auto trans_params = _factory.getValidParams("MultiAppPostprocessorTransfer");
    trans_params.set<MultiAppName>("multi_app") = "SAM";
    trans_params.set<MultiMooseEnum>("direction") = "from_multiapp";
    trans_params.set<MooseEnum>("reduction_type") = "average";
    trans_params.set<PostprocessorName>("from_postprocessor") = "SAMtoNekRS_velocity"; // requires this PP in SAM input file
    trans_params.set<PostprocessorName>("to_postprocessor") = "SAMtoNekRS_velocity";
    addTransfer("MultiAppPostprocessorTransfer", "SAMtoNekRS_velocity_trans", trans_params);

  }

  // SAM -> NekRS temperature interface
  if (_SAMtoNekRS_temperature)
  {
    auto pp_params = _factory.getValidParams("Receiver");
    addPostprocessor("Receiver", "SAMtoNekRS_temperature", pp_params);

    // create Transfer for SAM temperature to NekRS
    auto trans_params = _factory.getValidParams("MultiAppPostprocessorTransfer");
    trans_params.set<MultiAppName>("multi_app") = "SAM";
    trans_params.set<MultiMooseEnum>("direction") = "from_multiapp";
    trans_params.set<MooseEnum>("reduction_type") = "average";
    trans_params.set<PostprocessorName>("from_postprocessor") = "SAMtoNekRS_temperature"; // requires this PP in SAM input file
    trans_params.set<PostprocessorName>("to_postprocessor") = "SAMtoNekRS_temperature";
    addTransfer("MultiAppPostprocessorTransfer", "SAMtoNekRS_temperature_trans", trans_params);
  }

}
