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

#include "NekPostprocessor.h"

InputParameters
NekPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  params.addParam<MooseEnum>(
      "mesh", getNekMeshEnum(), "NekRS mesh to compute postprocessor on");
  return params;
}

NekPostprocessor::NekPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mesh(_subproblem.mesh()),
    _pp_mesh(getParam<MooseEnum>("mesh").getEnum<nek_mesh::NekMeshEnum>())
{
  _nek_problem = dynamic_cast<const NekRSProblemBase *>(&_fe_problem);
  if (!_nek_problem)
  {
    std::string extra_help = _fe_problem.type() == "FEProblem" ? " (the default)" : "";
    mooseError("This postprocessor can only be used with wrapped Nek cases!\n"
               "You need to change the problem type from '" +
               _fe_problem.type() + "'" + extra_help +
               " to a Nek-wrapped problem.\n\n"
               "options: 'NekRSProblem', 'NekRSSeparateDomainProblem', 'NekRSStandaloneProblem'");
  }

  // NekRSProblem enforces that we then use NekRSMesh, so we don't need to check that
  // this pointer isn't NULL
  _nek_mesh = dynamic_cast<const NekRSMesh *>(&_mesh);

  if (isParamSetByUser("use_displaced_mesh"))
    mooseWarning("'use_displaced_mesh' is unused, because this postprocessor acts directly\n"
      "on the NekRS internal mesh");
}

#endif
