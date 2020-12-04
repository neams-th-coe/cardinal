//
// Created by Ronald Rahaman on 2019-01-23.
//

#include "NekProblem.h"
#include "Moose.h"
#include "AuxiliarySystem.h"
#include "Transient.h"
#include "TimeStepper.h"

#include "nekrs.hpp"
#include "nekInterface/nekInterfaceAdapter.hpp"

registerMooseObject("NekApp", NekProblem);

template<>
InputParameters
validParams<NekProblem>()
{
  InputParameters params = validParams<ExternalProblem>();
  return params;
}

NekProblem::NekProblem(const InputParameters &params) : ExternalProblem(params),
    _serialized_solution(NumericVector<Number>::build(_communicator).release()),
    _outputStep(nekrs::outputStep()),
    _time(nekrs::startTime())
{
  // If the simulation start time is not zero, the app's time must be shifted
  // relative to its master app (if any). Until this is implemented, make sure
  // a start time of zero is used.
  if (_time != 0.0)
    mooseError("A non-zero start time is not yet available for 'NekProblem'! "
      "Change the 'startTime' parameter in your .par file to zero.");
}

NekProblem::~NekProblem()
{
  if (!_app.isUltimateMaster() && !isOutputStep())
  {
    // copy nekRS solution from device to host
    nekrs::copyToNek(_time, _tstep);

    // write nekRS solution to output
    nekrs::nekOutfld();
  }
}

void
NekProblem::initialSetup()
{
  ExternalProblem::initialSetup();

  auto executioner = _app.getExecutioner();
  Transient * transient_executioner = dynamic_cast<Transient *>(executioner);

  // nekRS only supports transient simulations - therefore, it does not make
  // sense to use anything except a Transient-derived executioner
  if (!transient_executioner)
    mooseError("A Transient-type executioner should be used for nekRS!");

  TimeStepper * stepper = transient_executioner->getTimeStepper();
  _timestepper = dynamic_cast<NekTimeStepper *>(stepper);

  // To get the correct time stepping information on the MOOSE side, we also
  // must use the NekTimeStepper
  if (!_timestepper)
    mooseError("The 'NekTimeStepper' stepper must be used with 'NekProblem'!");
}

bool
NekProblem::isOutputStep() const
{
  Real n_steps = _timestepper->getNumTimeSteps();

  bool is_output_step = false;
  if (_outputStep > 0) {
    if (_tstep % _outputStep == 0 || _tstep == n_steps)
      is_output_step = true;
  }

  return is_output_step;
}

void NekProblem::externalSolve()
{
  ++_tstep;

  // The _dt member of NekProblem reflects the time step that MOOSE wants NekApp to
  // take. For instance, if NekApp is controlled by a master app and subcycling is used,
  // NekApp must advance to the time interval taken by the master app. If the time step
  // that MOOSE wants nekRS to take (i.e. _dt) does not match the time step that nekRS
  // has used to construct all the coefficient matrices, etc. for nekRS's internal time
  // stepping, an additional step would need to be added below to ensure that nekRS can
  // _correctly_ take a variable time step. This infrastructure is currently lacking from
  // nekRS, even though the nekrs::runStep function looks at a high level to be capable of
  // accepting a variable time step size as input.
  if (std::abs(_dt - nekrs::dt()) > 1e-8)
    mooseError("nekRS does not currently allow adaptive time stepping! NekApp is trying to use "
      "a time step of " + std::to_string(_dt) + ", but nekRS's time step is " + std::to_string(nekrs::dt()));

  bool is_output_step = isOutputStep();

  nekrs::runStep(_time, _dt, _tstep);

  nekrs::copyToNek(_time + _dt, _tstep);

  nekrs::udfExecuteStep(_time + _dt, _tstep, is_output_step);

  if (is_output_step)
    nekrs::nekOutfld();

  _time += _dt;
}

void NekProblem::syncSolutions(ExternalProblem::Direction direction)
{
 
  switch(direction)
  {

    case ExternalProblem::Direction::TO_EXTERNAL_APP:
    {
      auto & mesh = _mesh.getMesh();

      double *n_nekrs = &nekData.cbscnrs[0];  
      //std::cout << "N. elements A: " << n_nekrs[0];
      int num_elems =  0;
      num_elems = (int) n_nekrs[0]; 
      //std::cout << "N. elements B: " << num_elems;
 
      //num_elems = 0;

      // num_elems = nekData.cbscnrs[0];  
      // auto num_elems = Nek5000::tot_surf_.nw_dbt;

      double *nek_flux = &nekData.cbscnrs[1+num_elems*4*4];
      //auto nek_flux = Nek5000::point_cloudf_.pc_f;

      auto & solution = _aux->solution();

      auto sys_number = _aux->number();

      static bool first = true;

      if (first)
      {
        _serialized_solution->init(_aux->sys().n_dofs(), false, SERIAL);
        first = false;
      }

      solution.localize(*_serialized_solution);

      // Here's how this works:
      // We are reading Quad4s so each one has 4 nodes
      // So loop over the elements and pull out the nodes... easy

      for (unsigned int e = 0; e < num_elems; e++)
      {
        auto elem_ptr = mesh.elem_ptr(e);

        for (unsigned int n = 0; n < 4; n++)
        {
          auto node_ptr = elem_ptr->node_ptr(n);

          auto node_offset = (e * 4) + n;
          auto dof_idx = node_ptr->dof_number(sys_number, _avg_flux_var, 0);

          nek_flux[node_offset] = (*_serialized_solution)(dof_idx);
        }
      }

      auto total_flux = getPostprocessorValue("total_flux");

      std::cout << "Total flux going to Nek: " << total_flux << std::endl;

      nekData.cbscnrs[1+num_elems*4*5]=total_flux;

      int _isOutputStep = 0;
      nekrs::udfExecuteStep(_time, _tstep, _isOutputStep);
 //   Nek5000::test_passing_.flux_moose = total_flux;

 //   Nek5000::FORTRAN_CALL(flux_reconstruction)();
    }

    break;
    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    { 
     
   //   Nek5000::FORTRAN_CALL(nek_interpolation)();
      auto & mesh = _mesh.getMesh();

      double *n_nekrs = &nekData.cbscnrs[0];
      int num_elems =  0;
      num_elems = (int) n_nekrs[0];

      // auto num_elems = Nek5000::tot_surf_.nw_dbt;

      auto nek_temperature = &nekData.cbscnrs[1+num_elems*4*3];
      //auto nek_temperature = Nek5000::point_cloudt_.pc_t;

      std::cout << "Test Temperature: " <<nek_temperature[3];

      
      auto & solution = _aux->solution();

      auto sys_number = _aux->number();

      auto pid = _communicator.rank();

      // Here's how this works:
      // We are reading Quad4s so each one has 4 nodes
      // So loop over the elements and pull out the nodes... easy

      for (unsigned int e = 0; e < num_elems; e++)
      {
        auto elem_ptr = mesh.elem_ptr(e);

        for (unsigned int n = 0; n < 4; n++)
        {
          auto node_ptr = elem_ptr->node_ptr(n);

          if (node_ptr->processor_id() == pid)
          {
            auto node_offset = (e * 4) + n;

            auto dof_idx = node_ptr->dof_number(sys_number, _temp_var, 0);

 //           solution.set(dof_idx, 573.0);
              solution.set(dof_idx, nek_temperature[node_offset]);
          }
        }
      }

      solution.close();

      break;
    }
    default:
      mooseError("Shouldn't get here!");
  }
 
}

void
NekProblem::addExternalVariables()
{
//  FEType elemental(CONSTANT, MONOMIAL);
  FEType nodal(FIRST, LAGRANGE);

  addAuxVariable("temp", nodal);
  _temp_var = _aux->getFieldVariable<Real>(0, "temp").number();

  addAuxVariable("avg_flux", nodal);
  _avg_flux_var = _aux->getFieldVariable<Real>(0, "avg_flux").number();
}
