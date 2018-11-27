#include "MultiAppPolynomialToNek.h"
#include "NekInterface.h"

// MOOSE includes
#include "MooseTypes.h"
#include "FEProblem.h"
#include "MultiApp.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"

// libMesh includes
#include "libmesh/meshfree_interpolation.h"
#include "libmesh/system.h"


// Define the input parameters
template<>
InputParameters validParams<MultiAppPolynomialToNek>()
{
  InputParameters params = validParams<MultiAppTransfer>();
  params.addRequiredParam<std::vector<VariableName> >("source_variable", "The auxiliary scalar variable to read values from");
  params.addRequiredParam<std::vector<VariableName> >("to_aux_scalar", "The name of the scalar Aux variable in the MultiApp to transfer the value to.");
  return params;
}

MultiAppPolynomialToNek::MultiAppPolynomialToNek(const InputParameters & parameters) :
    MultiAppTransfer(parameters),
    _source_variable_names(getParam<std::vector<VariableName> >("source_variable")),
    _to_aux_names(getParam<std::vector<VariableName> >("to_aux_scalar"))
{
}

void
MultiAppPolynomialToNek::execute()
{
  _console << "Beginning PolynomialToNekTransfer " << name() << std::endl;

  // Perform action based on the transfer direction
  switch (_direction)
  {
    // MasterApp -> SubApp
  case TO_MULTIAPP:
  {
    FEProblemBase & from_problem = _multi_app->problemBase();

    std::vector<MooseVariableScalar *> source_variables(_source_variable_names.size());
    for (auto i = beginIndex(_source_variable_names); i < _source_variable_names.size(); ++i)
    {
      source_variables[i] = &from_problem.getScalarVariable(_tid, _source_variable_names[i]);
      source_variables[i]->reinit();
    }

    // Loop through each of the sub apps
    for (unsigned int i=0; i<_multi_app->numGlobalApps(); i++)
      if (_multi_app->hasLocalApp(i))
      {
        // TODO: Access Nek Common blocks and write values
        for (auto i = beginIndex(_source_variable_names); i < _source_variable_names.size(); ++i)
        {
          _console << _source_variable_names[i] << '\n';

          auto & solution_values = source_variables[i]->sln();
          for (auto j = beginIndex(solution_values); j < solution_values.size(); ++j)
          {
            _console << solution_values[j] << ' ';
            Nek5000::expansion_fcoef_.coeff_fij[i*100+j] = solution_values[j];
          }
          _console << '\n';
        }
      }
    FORTRAN_CALL(Nek5000::flux_reconstruction)();

    break;
  }

    // SubApp -> MasterApp
  case FROM_MULTIAPP:
  {
    FORTRAN_CALL(Nek5000::nek_expansion)();
    // The number of sub applications
    unsigned int num_apps = _multi_app->numGlobalApps();

    std::vector<MooseVariableScalar *> to_variables(_to_aux_names.size());
    for (auto i = beginIndex(_to_aux_names); i < _to_aux_names.size(); ++i)
    {
      to_variables[i] = &_multi_app->problemBase().getScalarVariable(_tid, _to_aux_names[i]);
      to_variables[i]->reinit();
    }

    for (auto i = beginIndex(_to_aux_names); i < _to_aux_names.size(); ++i)
    {
      // The dof indices for the scalar variable of interest
      std::vector<dof_id_type> & dof = to_variables[i]->dofIndices();

//      // Error if there is a size mismatch between the scalar AuxVariable and the number of sub apps
//      if (num_apps != scalar.sln().size())
//        mooseError("The number of sub apps (" << num_apps << ") must be equal to the order of the scalar AuxVariable (" << scalar.order() << ")");

      // Loop over each sub-app and populate the AuxVariable values from the postprocessors
//        for (unsigned int i=0; i<_multi_app->numGlobalApps(); i++)

//        if (_multi_app->hasLocalApp(i) && _multi_app->isRootProcessor())
      // Note: This can't be done using MooseScalarVariable::insert() because different processors will be setting dofs separately.
      auto & solution_values = to_variables[i]->sln();
      for (auto j = beginIndex(solution_values); j < solution_values.size(); ++j)
      {
        to_variables[i]->sys().solution().set(dof[j], Nek5000::expansion_tcoef_.coeff_tij[i*100+j]);
        to_variables[i]->sys().solution().close();
      }
    }

    break;
  }
  }

  _console << "Finished PolynomialToNekTransfer" << name() << std::endl;
}
