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

#include "CardinalProblem.h"
#include "NonlinearSystem.h"

InputParameters
CardinalProblem::validParams()
{
  InputParameters params = ExternalProblem::validParams();

  // these parameters don't do anything for ExternalProblem, and instead just
  // clutter the MooseDocs
  params.suppressParameter<bool>("allow_invalid_solution");
  params.suppressParameter<bool>("boundary_restricted_elem_integrity_check");
  params.suppressParameter<bool>("boundary_restricted_node_integrity_check");
  params.suppressParameter<bool>("check_uo_aux_state");
  params.suppressParameter<bool>("error_on_jacobian_nonzero_reallocation");
  params.suppressParameter<std::vector<std::vector<TagName>>>("extra_tag_matrices");
  params.suppressParameter<std::vector<TagName>>("extra_tag_solutions");
  params.suppressParameter<std::vector<std::vector<TagName>>>("extra_tag_vectors");
  params.suppressParameter<bool>("force_restart");
  params.suppressParameter<bool>("fv_bcs_integrity_check");
  params.suppressParameter<bool>("material_dependency_check");
  params.suppressParameter<unsigned int>("near_null_space_dimension");
  params.suppressParameter<unsigned int>("null_space_dimension");
  params.suppressParameter<unsigned int>("transpose_null_space_dimension");
  params.suppressParameter<bool>("immediately_print_invalid_solution");
  params.suppressParameter<bool>("identify_variable_groups_in_nl");
  params.suppressParameter<std::vector<LinearSystemName>>("linear_sys_names");

  return params;
}

CardinalProblem::CardinalProblem(const InputParameters & params)
  : ExternalProblem(params)
{
}

void
CardinalProblem::checkDuplicateVariableName(const std::string & name) const
{
  // TODO: eventually remove this
  std::string extra;
  if (name == "cell_id" || name == "cell_instance")
    extra = "\n\nCardinal recently added the CellIDAux (cell_id) and CellInstanceAux "
            "(cell_instance) as automatic outputs when using OpenMC, so you no longer need to "
            "include these auxkernels manually.";

  if (_aux.get()->hasVariable(name))
    mooseError("Cardinal is trying to add an auxiliary variable named '",
               name,
               "', but you already have a variable by this name. Please choose a different name "
               "for the auxiliary variable you are adding." +
                   extra);

  if (_nl[0].get()->hasVariable(name))
    mooseError("Cardinal is trying to add a nonlinear variable named '", name,
      "', but you already have a variable by this name. Please choose a different name "
      "for the nonlinear variable you are adding.");
}

bool
CardinalProblem::stringHasEnding(std::string const & full, std::string const & ending) const
{
  if (full.length() >= ending.length())
      return 0 == full.compare(full.length() - ending.length(), ending.length(), ending);

  return false;
}
