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
  this->initialize();
}

void
OpenMCTallyEditor::initialize()
{
  // the tally creation/existence check is placed here, instead of the
  // constructor, because Cardinal initializes some of it's own tallies.
  // Depending on the order of initialization of UserObjects vs. other classes,
  // those tallies might not exist yet in OpenMC's data space (but they will by
  // the time this method is called).
  bool create_tally = getParam<bool>("create_tally");

  if (create_tally)
  {
    if (tallyExists())
    {
      mooseWarning(longName() + ": Tally " + std::to_string(_tally_id) +
                   " already exists in the OpenMC model and will be overriden"
                   " by this UserObject");
    }
    else
    {
      _console << longName() << ": Creating tally " << _tally_id << std::endl;
      openmc::Tally::create(_tally_id);
    }
  }
}

bool
OpenMCTallyEditor::tallyExists() const
{
  return openmc::model::tally_map.find(_tally_id) != openmc::model::tally_map.end();
}

int32_t
OpenMCTallyEditor::tallyIndex() const
{
  return openmc::model::tally_map.at(_tally_id);
}

void
OpenMCTallyEditor::execute()
{
  if (!tallyExists())
    paramError("tally_id",
               "Tally " + std::to_string(_tally_id) + " does not exist in the OpenMC model");

  openmc::Tally * tally = openmc::model::tallies[tallyIndex()].get();

  std::vector<std::string> scores = getParam<std::vector<std::string>>("scores");
  if (scores.size() > 0)
  {
    try
    {
      tally->set_scores(scores);
    }
    catch (const std::exception & e)
    {
      std::string s = e.what();
      paramError("scores", "In attempting to set tally scores, OpenMC reported:\n\n" + s);
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
      std::string s = e.what();
      paramError("nuclides", "In attempting to set tally nuclides, OpenMC reported:\n\n" + s);
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

void
OpenMCTallyEditor::duplicateTallyError(const int32_t & id) const
{
  paramError("tally_id",
             "Duplicate tally ID (" + std::to_string(id) +
                 ") found in multiple OpenMCTallyEditors");
}

void
OpenMCTallyEditor::mappedTallyError(const int32_t & id) const
{
  paramError(
      "tally_id",
      "This tally ID (" + std::to_string(id) +
          ") is a tally which Cardinal is automatically creating and controlling from the "
          "Problem/Tallies block. OpenMCTallyEditor cannot be used for these types of tallies.");
}

#endif
