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
  params.addParam<bool>("SAMtoNekRS_boundary", false, "SAM -> NekRS boundary");
  params.addParam<bool>("NekRStoSAM_boundary", false, "NekRS -> SAM boundary");
  params.addParam<bool>("SAMtoNekRS_temperature", false, "SAM -> NekRS temperature transfer");
  return params;
}

SAMNekRSProblem::SAMNekRSProblem(const InputParameters &params) : NekRSProblemBase(params),
    _serialized_solution(NumericVector<Number>::build(_communicator).release()), 
    _moving_mesh(getParam<bool>("moving_mesh")),
    _minimize_transfers_in(getParam<bool>("minimize_transfers_in")),
    _minimize_transfers_out(getParam<bool>("minimize_transfers_out")),
    _SAMtoNekRS(getParam<bool>("SAMtoNekRS_boundary")),
    _NekRStoSAM(getParam<bool>("NekRStoSAM_boundary")),
    _SAMtoNekRS_temperature(getParam<bool>("SAMtoNekRS_temperature"))
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
    "is being sent between SAM and NekRS, atleast one of SAMtoNekRS_boundary \n", 
    "or NekRStoSAM_boundary must be set to true\n");

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

        // get velocity from Nek to SAM 
//      if (_boundary)
//        getBoundaryVelocityNek();

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

  _console << "Sending velocity to NekRS for first boundary of " << Moose::stringify(*_boundary) << std::endl;

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

  _console << "Sending temperature to NekRS for first boundary of " << Moose::stringify(*_boundary) << std::endl;

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

//  if (_SAMtoNekRS)
//  {
//  }

}

//void
//SAMNekRSProblem::getBoundaryTemperatureFromNek()
//{
//  _console << "Extracting NekRS temperature from boundary " << Moose::stringify(*_boundary) << std::endl;
//
//  // Get the temperature solution from nekRS. Note that nekRS performs a global communication
//  // here such that each nekRS process has all the boundary temperature information. That is,
//  // every process knows the full boundary temperature solution
//  nekrs::boundarySolution(_nek_mesh->order(), _needs_interpolation, field::temperature, _T);
//}

//void
//SAMNekRSProblem::addExternalVariables()
//{
//  NekRSProblemBase::addExternalVariables();
//  auto var_params = getExternalVariableParameters();
//
//  addAuxVariable("MooseVariable", "temp", var_params);
//  _temp_var = _aux->getFieldVariable<Real>(0, "temp").number();
//
//  if (_boundary)
//  {
//    // Likewise, because this flux represents the reconstruction of the flux variable
//    // that becomes a boundary condition in the nekRS model, we set the order to match
//    // the desired order of the surface. Note that this does _not_ imply anything
//    // about the order of the surface flux in the MOOSE app (such as BISON) coupled
//    // to nekRS. This is just the variable that nekRS reads from - MOOSE's transfer
//    // classes handle any additional interpolations needed from the flux on the
//    // sending app (such as BISON) into 'avg_flux'.
//    addAuxVariable("MooseVariable", "avg_flux", var_params);
//    _avg_flux_var = _aux->getFieldVariable<Real>(0, "avg_flux").number();
//
//    // add the postprocessor that receives the flux integral for normalization
//    auto pp_params = _factory.getValidParams("Receiver");
//    addPostprocessor("Receiver", "flux_integral", pp_params);
//  }
//
//  if (_volume && _has_heat_source)
//  {
//    addAuxVariable("MooseVariable", "heat_source", var_params);
//    _heat_source_var = _aux->getFieldVariable<Real>(0, "heat_source").number();
//
//    // add the postprocessor that receives the source integral for normalization
//    auto pp_params = _factory.getValidParams("Receiver");
//    addPostprocessor("Receiver", "source_integral", pp_params);
//  }
//
//  // add the displacement aux variables from the solid mechanics solver; these will
//  // be needed regardless of whether the displacement is boundary- or volume-based
//  if (_moving_mesh)
//  {
//    addAuxVariable("MooseVariable", "disp_x", var_params);
//    _disp_x_var = _aux->getFieldVariable<Real>(0, "disp_x").number();
//
//    addAuxVariable("MooseVariable", "disp_y", var_params);
//    _disp_y_var = _aux->getFieldVariable<Real>(0, "disp_y").number();
//
//    addAuxVariable("MooseVariable", "disp_z", var_params);
//    _disp_z_var = _aux->getFieldVariable<Real>(0, "disp_z").number();
//  }
//
//  if (_minimize_transfers_in)
//  {
//    auto pp_params = _factory.getValidParams("Receiver");
//    pp_params.set<std::vector<OutputName>>("outputs") = {"none"};
//    addPostprocessor("Receiver", "transfer_in", pp_params);
//  }
//}
