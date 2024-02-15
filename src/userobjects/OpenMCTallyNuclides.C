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

#ifdef ENABLE_OPENMC_COUPLING

#include "OpenMCTallyNuclides.h"
#include "OpenMCProblemBase.h"
#include "openmc/tallies/tally.h"

registerMooseObject("CardinalApp", OpenMCTallyNuclides);

InputParameters
OpenMCTallyNuclides::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<int32_t>("tally_id", "ID of tally to change nuclides");
  params.addRequiredParam<std::vector<std::string>>("names", "Names of the nuclides");
  params.declareControllable("names");
  params.addClassDescription("Updates nuclides in an OpenMC tally");
  return params;
}

OpenMCTallyNuclides::OpenMCTallyNuclides(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _tally_id(getParam<int32_t>("tally_id")),
    _names(getParam<std::vector<std::string>>("names"))
{
  const OpenMCProblemBase * openmc_problem = dynamic_cast<const OpenMCProblemBase *>(&_fe_problem);
  if (!openmc_problem)
  {
    std::string extra_help = _fe_problem.type() == "FEProblem" ? " (the default)" : "";
    mooseError("This user object can only be used with wrapped OpenMC cases! "
               "You need to change the\nproblem type from '" +
               _fe_problem.type() + "'" + extra_help + " to OpenMCCellAverageProblem.");
  }

  openmc_problem->catchOpenMCError(openmc_get_tally_index(_tally_id, &_tally_index),
                                   "get the tally index for tally with ID " +
                                       std::to_string(_tally_id));
}

void
OpenMCTallyNuclides::setValue()
{
  try
  {
    openmc::model::tallies[_tally_index]->set_nuclides(_names);
  }
  catch (const std::exception & e)
  {
    mooseError("In attempting to set tally nuclides in the '" + name() +
               "' UserObject, OpenMC reported:\n\n" + e.what());
  }
}

#endif
