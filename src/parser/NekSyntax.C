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
}

}
