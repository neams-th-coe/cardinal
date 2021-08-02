#include "OpenMCSyntax.h"
#include "ActionFactory.h"
#include "Syntax.h"

namespace OpenMC
{

void
associateSyntax(Syntax & syntax, ActionFactory & /*action_factory*/)
{
  registerMooseObjectTask("cardinal:init", Problem, true);
  registerSyntax("OpenMCInitAction", "Problem");
}

}
