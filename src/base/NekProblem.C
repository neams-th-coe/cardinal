//
// Created by Ronald Rahaman on 2019-01-23.
//

#include "NekProblem.h"
#include "Moose.h"
#include "AuxiliarySystem.h"

#include "NekInterface.h"

registerMooseObject("NekApp", NekProblem);

template<>
InputParameters
validParams<NekProblem>()
{
  InputParameters params = validParams<ExternalProblem>();
  // No required parameters
  return params;
}

NekProblem::NekProblem(const InputParameters &params) : ExternalProblem(params)
{
}


void NekProblem::externalSolve()
{
  _console << "Beginning Nek5000 external solve";
  Nek5000::FORTRAN_CALL(nek_init_step)();
  Nek5000::FORTRAN_CALL(nek_step)();
  Nek5000::FORTRAN_CALL(nek_finalize_step)();
}

void NekProblem::syncSolutions(ExternalProblem::Direction direction)
{
  switch(direction)
  {
    case ExternalProblem::Direction::TO_EXTERNAL_APP:
      break;
    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    {
      Nek5000::FORTRAN_CALL(nek_pointscloud)();

      auto & mesh = _mesh.getMesh();

      auto num_elems = Nek5000::tot_surf_.nw_dbt;

      auto nek_temperature = Nek5000::point_temp_.pc_t;

      auto & solution = _aux->solution();

      auto sys_number = _aux->number();

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

          auto dof_idx = node_ptr->dof_number(sys_number, _temp_var, 0);

          solution.set(dof_idx, nek_temperature[node_offset]);
        }
      }

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
}
