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

#include "OpenMCTallyGenerator.h"
#include "OpenMCTallyNuclides.h"
#include "OpenMCProblemBase.h"
#include "openmc/tallies/tally.h"

registerMooseObject("CardinalApp", OpenMCTallyGenerator);

InputParameters
OpenMCTallyGenerator::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<bool>("nuclides", "Whether or not to create a UserObject to modify nuclides in the new OpenMC tally");
  params.addClassDescription("An OpenMC tally generation UserObject");
  return params;
}

OpenMCTallyGenerator::OpenMCTallyGenerator(const InputParameters & parameters)
  : GeneralUserObject(parameters), _nuclides_uo(getParam<bool>("nuclides"))
{
  const OpenMCProblemBase * openmc_problem = dynamic_cast<const OpenMCProblemBase *>(&_fe_problem);
  if (!openmc_problem)
  {
    std::string extra_help = _fe_problem.type() == "FEProblem" ? " (the default)" : "";
    mooseError("This user object can only be used with wrapped OpenMC cases! "
               "You need to change the\nproblem type from '" +
               _fe_problem.type() + "'" + extra_help + " to OpenMCCellAverageProblem.");
  }

}

void
OpenMCTallyGenerator::execute()
{
  const openmc::Tally* tally = openmc::Tally::create();
  _created_tally_ids.push_back(tally->id());

  if (_nuclides_uo) {
    InputParameters params = OpenMCTallyNuclides::validParams();
    params.set<int32_t>("tally_id", tally->id());
    params.set<bool>("nuclides", true);
    _fe_problem.addUserObject("OpenMCTallyNuclides", "my_tally_mine_and_no_one_elses", params);
  }
}

#endif
