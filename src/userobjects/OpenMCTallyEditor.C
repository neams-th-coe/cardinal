/********************************************************************/
/*                  SOFTWARE COPYRIGHT NOTIFICATION                 */
/*                             Cardinal                             */
/*                                                                  */
/*                  (c) 2024 UChicago Argonne, LLC                  */
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

#include "OpenMCTallyEditor.h"
#include "OpenMCProblemBase.h"
#include "openmc/tallies/tally.h"

registerMooseObject("CardinalApp", OpenMCTallyEditor);

InputParameters
OpenMCTallyEditor::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addParam<bool>("create_tally", false, "Whether to create the tally if it doesn't exist");
  params.addRequiredParam<int32_t>("tally_id", "The ID of the tally to modify");
  params.addRequiredParam<std::vector<std::string>>("scores", "The scores to modify in the tally");
  params.addRequiredParam<std::vector<std::string>>("nuclides",
                                                    "The nuclides to modify in the tally");
  params.addRequiredParam<std::vector<std::string>>("filter_ids",
                                                    "The filter IDs to modify in the tally");
  params.addParam<bool>(
      "multiply_density", true, "Whether to multiply the tally by the atom density");
  params.declareControllable("scores");
  params.declareControllable("nuclides");
  params.declareControllable("filter_ids");
  params.declareControllable("multiply_density");
  params.addClassDescription("A UserObject for creating and managing OpenMC tallies");
  return params;
}

OpenMCTallyEditor::OpenMCTallyEditor(const InputParameters & parameters)
  : OpenMCUserObject(parameters), _tally_id(getParam<int32_t>("tally_id"))
{
}

void
OpenMCTallyEditor::initialize()
{
  // this is placed here, instead of the constructor, because Cardinal initializes
  // some tallies. Depending on the order of initialization of UserObjects vs.
  // other classes, those tallies might not exist yet in OpenMC's data space
  // (but they will by the time this method is called).
  bool create_tally = getParam<bool>("create_tally");
  bool tally_exists = openmc::model::tally_map.find(_tally_id) != openmc::model::tally_map.end();

  if (create_tally)
  {
    if (tally_exists)
    {
      mooseWarning(long_name() + ": Tally " + std::to_string(_tally_id) +
                   " already exists in the OpenMC model");
    }
    else
    {
      openmc_problem()->_console << long_name() << ": Creating tally " << _tally_id << std::endl;
      openmc::Tally::create(_tally_id);
    }
  }
  else
  {
    if (!tally_exists)
    {
      mooseError(long_name() + ": Tally " + std::to_string(_tally_id) +
                 " does not exist in the OpenMC model");
    }
  }
}

void
OpenMCTallyEditor::first_execution()
{
  if (!_first_execution)
    return;
  initialize();
  _first_execution = false;
}

int32_t
OpenMCTallyEditor::tally_index() const
{
  return openmc::model::tally_map.at(_tally_id);
}

void
OpenMCTallyEditor::execute()
{
  first_execution();
  openmc::Tally * tally = openmc::model::tallies[tally_index()].get();

  std::vector<std::string> scores = getParam<std::vector<std::string>>("scores");
  if (scores.size() > 0)
  {
    try
    {
      tally->set_scores(scores);
    }
    catch (const std::exception & e)
    {
      mooseError("In attempting to set tally scores in the '" + name() +
                 "' UserObject, OpenMC reported:\n\n" + e.what());
    }
  }

  std::vector<std::string> nuclides = getParam<std::vector<std::string>>("nuclides");
  if (nuclides.size() > 0)
  {
    try
    {
      tally->set_nuclides(nuclides);
    }
    catch (const std::exception & e)
    {
      mooseError("In attempting to set tally nuclides in the '" + name() +
                 "' UserObject, OpenMC reported:\n\n" + e.what());
    }
  }

  std::vector<std::string> filter_ids = getParam<std::vector<std::string>>("filter_ids");
  if (filter_ids.size() > 0)
  {
    tally->set_filters({});
    for (const auto & fid : filter_ids)
    {
      int32_t filter_index = openmc::model::filter_map.at(std::stoi(fid));
      tally->add_filter(openmc::model::tally_filters[filter_index].get());
    }
  }

  tally->set_multiply_density(getParam<bool>("multiply_density"));
}

#endif
