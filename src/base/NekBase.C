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

#include "NekBase.h"

InputParameters
NekBase::validParams()
{
  InputParameters params = emptyInputParameters();
  return params;
}

NekBase::NekBase(const MooseObject * moose_object, const InputParameters & parameters)
  : _nek_problem(dynamic_cast<NekRSProblem *>(&moose_object->getMooseApp().feProblem()))
{
  if (!_nek_problem)
  {
    std::string extra_help =
        moose_object->getMooseApp().feProblem().type() == "FEProblem" ? " (the default)" : "";
    mooseError(
        moose_object->type() +
        " can only be used with NekRS-wrapped cases! You need to change the problem type from '" +
        moose_object->getMooseApp().feProblem().type() + "'" + extra_help + " to NekRSProblem.");
  }

  // NekRSProblem enforces that we then use NekRSMesh, so we don't need to check that
  // this pointer isn't NULL
  _nek_mesh = dynamic_cast<const NekRSMesh *>(&moose_object->getMooseApp().feProblem().mesh());

  if (moose_object->isParamSetByUser("use_displaced_mesh"))
    mooseWarning("'use_displaced_mesh' is unused, because this postprocessor acts directly\n"
                 "on the NekRS internal mesh");
}

#endif
