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

#include "NekScalarValue.h"
#include "NekRSProblemBase.h"
#include "NekInterface.h"

registerMooseObject("CardinalApp", NekScalarValue);

InputParameters
NekScalarValue::validParams()
{
  InputParameters params = ThreadedGeneralUserObject::validParams();
  params.addRequiredParam<Real>("value", "Scalar value to pass into NekRS");
  params.addParam<unsigned int>("usrwrk_slot", "Slot in nrs->usrwrk into which to write the value; "
    "if not specified, this defaults to the first unused slot");
  params.declareControllable("value");
  params.addClassDescription("Writes a scalar value from MOOSE into NekRS's scratch space");
  return params;
}

NekScalarValue::NekScalarValue(const InputParameters & parameters)
  : ThreadedGeneralUserObject(parameters),
    _value(getParam<Real>("value"))
{
  const NekRSProblemBase * nek_problem = dynamic_cast<const NekRSProblemBase *>(&_fe_problem);
  if (!nek_problem)
  {
    std::string extra_help = _fe_problem.type() == "FEProblem" ? " (the default)" : "";
    mooseError("This user object can only be used with wrapped Nek cases!\n"
               "You need to change the problem type from '" +
               _fe_problem.type() + "'" + extra_help +
               " to a Nek-wrapped problem.\n\n"
               "options: 'NekRSProblem', 'NekRSSeparateDomainProblem', 'NekRSStandaloneProblem'");
  }

  // pick a reasonable default if not specified
  if (isParamValid("usrwrk_slot"))
    _usrwrk_slot = getParam<unsigned int>("usrwrk_slot");
  else
    _usrwrk_slot = nek_problem->minimumScratchSizeForCoupling();

  // check that we're not writing into space that's definitely being used for coupling
  if (nek_problem->minimumScratchSizeForCoupling() > 0)
    if (_usrwrk_slot < nek_problem->minimumScratchSizeForCoupling())
      mooseError("Cannot write into a scratch space slot reserved for Nek-MOOSE coupling!\n"
        "For this case, you must set 'usrwrk_slot' greater than or equal to ",
        Moose::stringify(nek_problem->minimumScratchSizeForCoupling()));

  if (_usrwrk_slot >= nek_problem->nUsrWrkSlots())
    paramError("usrwrk_slot", "This parameter cannot exceed the available pre-allocated slots (",
      Moose::stringify(nek_problem->nUsrWrkSlots()), ").\nPlease either change this parameter, "
      "or increase 'n_usrwrk_slots' in the [Problem] block.");
}

void
NekScalarValue::setValue()
{
  nrs_t * nrs = (nrs_t *) nekrs::nrsPtr();
  nrs->usrwrk[_usrwrk_slot * nekrs::scalarFieldOffset() + _counter] = _value;
}

#endif