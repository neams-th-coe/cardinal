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

#include "NekSyntax.h"
#include "ActionFactory.h"
#include "Syntax.h"

namespace Nek
{

void
associateSyntax(Syntax & syntax, ActionFactory & /*action_factory*/)
{
  registerMooseObjectTask("nek_init", Problem, true);
  registerSyntax("NekInitAction", "Problem");
  addTaskDependency("meta_action", "nek_init");
}

} // namespace Nek

#endif
