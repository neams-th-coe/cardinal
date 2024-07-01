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

#include "OpenMCTallyGeneratorAction.h"
#include "OpenMCTallyNuclides.h"
#include "OpenMCProblemBase.h"
#include "openmc/tallies/tally.h"

registerMooseAction("CardinalApp", OpenMCTallyGeneratorAction, "add_tally_generator");

InputParameters
OpenMCTallyGeneratorAction::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<std::vector<std::string>>("ids", "Tally IDs to create in OpenMC");
  params.addRequiredParam<bool>("nuclides", "Whether or not to create a UserObject to modify nuclides in the new OpenMC tallies");
  params.addClassDescription("An OpenMC tally generation action");
  return params;
}

OpenMCTallyGeneratorAction::OpenMCTallyGeneratorAction(const InputParameters & parameters)
  : Action(parameters), _ids(getParam<std::vector<std::string>>("ids")), _nuclides_uo(getParam<bool>("nuclides"))
{
  const OpenMCProblemBase * openmc_problem = dynamic_cast<const OpenMCProblemBase *>(_problem.get());
  // if (!openmc_problem)
  // {
  //   std::string extra_help = _fe_problem.type() == "FEProblem" ? " (the default)" : "";
  //   mooseError("This user object can only be used with wrapped OpenMC cases! "
  //              "You need to change the\nproblem type from '" +
  //              _fe_problem.type() + "'" + extra_help + " to OpenMCCellAverageProblem.");
  // }

}

void
OpenMCTallyGeneratorAction::act()
{
  for (const auto & id : _ids)
  {
    openmc::Tally* tally = openmc::Tally::create(std::stoi(id));
    mooseInfo("Created Tally ID:" + std::to_string(tally->id()));
    if (_nuclides_uo)
    {
      const std::string uo_type = "OpenMCTallyNuclides";
      InputParameters params = _factory.getValidParams(uo_type);
      int32_t tally_id = std::stoi(id);
      params.set<int32_t>("tally_id") = tally_id;
      params.set<std::vector<std::string>>("names") =  {};
      mooseWarning("Creating OpenMCTallyNuclides tally_" + id + "_nuclides for tally with ID " + id);
      _problem->addUserObject(uo_type, "tally_" + id + "_nuclides", params);
    }
  }
  _ids.clear();
}

#endif
