//
// Created by Ronald Rahaman on 2019-01-23.
//

#include "NekProblem.h"
#include "Moose.h"
#include "AuxiliarySystem.h"

#include "NekInterface.h"
#include "nekrs.hpp"

registerMooseObject("NekApp", NekProblem);

template<>
InputParameters
validParams<NekProblem>()
{
  InputParameters params = validParams<ExternalProblem>();
  // No required parameters
  return params;
}

NekProblem::NekProblem(const InputParameters &params) : ExternalProblem(params),
                                                        _serialized_solution(NumericVector<Number>::build(_communicator).release()),
    _dt(nekrs::dt()),
    _output_step(nekrs::outputStep()),
    _n_timesteps(nekrs::NtimeSteps()),
    _start_time(nekrs::startTime()),
    _final_time(nekrs::finalTime())
{
}


void NekProblem::externalSolve()
{
  auto time = _start_time;
  int tstep = 1;
  // 1e-10 is from nekrs main(), not sure why
  while (time + 1e-10 < _final_time) {
    auto is_output_step = (_output_step > 0) && 
        (tstep % _output_step == 0 || tstep == _n_timesteps);

    nekrs::runStep(time, _dt, tstep);
    time += _dt;

    if (is_output_step) {
      nekrs::copyToNek(time, tstep);
    }

    nekrs::udfExecuteStep(time, tstep, (int) is_output_step);

    if (is_output_step) {
      nekrs::nekOutfld();
    }

    ++tstep;
  }
}

void NekProblem::syncSolutions(ExternalProblem::Direction direction)
{
  /*
  switch(direction)
  {

    case ExternalProblem::Direction::TO_EXTERNAL_APP:
    {
      auto & mesh = _mesh.getMesh();

      auto num_elems = Nek5000::tot_surf_.nw_dbt;

      auto nek_flux = Nek5000::point_cloudf_.pc_f;

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

      Nek5000::test_passing_.flux_moose = total_flux;

      Nek5000::FORTRAN_CALL(flux_reconstruction)();
    }

    break;
    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    {
      Nek5000::FORTRAN_CALL(nek_interpolation)();

      auto & mesh = _mesh.getMesh();

      auto num_elems = Nek5000::tot_surf_.nw_dbt;

      auto nek_temperature = Nek5000::point_cloudt_.pc_t;

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
  */
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
