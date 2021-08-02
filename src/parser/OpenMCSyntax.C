#include "OpenMCSyntax.h"
#include "ActionFactory.h"
#include "Syntax.h"

namespace OpenMC
{

void
associateSyntax(Syntax & syntax, ActionFactory & /*action_factory*/)
{
  registerMooseObjectTask("openmc_init", Problem, true);
  registerSyntax("OpenMCInitAction", "Problem");
}

}
