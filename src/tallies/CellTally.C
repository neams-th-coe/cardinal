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
#include "CellTally.h"

registerMooseObject("CardinalApp", CellTally);

InputParameters
CellTally::validParams()
{
  auto params = TallyBase::validParams();
  params.addClassDescription("A class which implements distributed cell tallies.");
  params.addParam<std::vector<SubdomainName>>(
      "tally_blocks",
      "Subdomains for which to add tallies in OpenMC. If not provided, cell "
      "tallies will be applied over the entire mesh.");

  return params;
}

CellTally::CellTally(const InputParameters & parameters)
  : TallyBase(parameters)
{
  if (isParamValid("tally_blocks"))
  {
    auto block_names = getParam<std::vector<SubdomainName>>("tally_blocks");
    if (block_names.empty())
      mooseError("Subdomain names must be provided if using 'tally_blocks'!");

    auto block_ids = _mesh.getSubdomainIDs(block_names);
    std::copy(block_ids.begin(), block_ids.end(), std::inserter(_tally_blocks, _tally_blocks.end()));

    // Check to make sure all of the blocks are in the mesh.
    const auto & subdomains = _mesh.meshSubdomains();
    for (std::size_t b = 0; b < block_names.size(); ++b)
      if (subdomains.find(block_ids[b]) == subdomains.end())
        mooseError("Block '" + block_names[b] + "' specified in 'tally_blocks' not found in mesh!");
  }
  else
  {
    // Tally over all mesh blocks if no blocks are provided.
    for (const auto & s : _mesh.meshSubdomains())
      _tally_blocks.insert(s);
  }
}

std::vector<std::string>
CellTally::generateAuxVarNames()
{
  // TODO: Add energy filters.
  return _tally_score;
}

void
CellTally::initializeTally()
{
  // Clear cached results.
  _local_sum_tally.resize(_tally_score.size());
  _local_mean_tally.resize(_tally_score.size());

  _current_tally.resize(_tally_score.size());
  _current_raw_tally.resize(_tally_score.size());
  _current_raw_tally_std_dev.resize(_tally_score.size());
  _previous_tally.resize(_tally_score.size());

  // Create the cell filter.
  _filter_index = openmc::model::tally_filters.size();
  _cell_filter = dynamic_cast<openmc::CellFilter *>(openmc::Filter::create("cell"));

  // TODO: Append to this to add an energy filter.
  std::vector<openmc::Filter *> filters = {_cell_filter};

  // Create the tally, assign the required filters and apply the triggers.
  _local_tally_index = openmc::model::tallies.size();
  _local_tally = openmc::Tally::create();
  _local_tally->set_scores(_tally_score);
  _local_tally->estimator_ = _estimator;
  _local_tally->set_filters(filters);
  applyTriggersToLocalTally(_local_tally);

  // Check to make sure we can map tallies to the mesh subdomains requested in tally_blocks.
  checkCellMappedSubdomains();
}

void
CellTally::resetTally()
{
  // Erase the tally.
  openmc::model::tallies.erase(openmc::model::tallies.begin() + _local_tally_index);

  // Erase the filter(s).
  openmc::model::tally_filters.erase(openmc::model::tally_filters.begin() + _filter_index);
}

Real
CellTally::storeResults(const std::vector<unsigned int> & var_numbers, unsigned int score)
{
  Real total = 0.0;

  int i = 0;
  for (const auto & c : _openmc_problem.cellToElem())
  {
    auto cell_info = c.first;

    // if this cell doesn't have any tallies, skip it
    if (!_cell_has_tally[cell_info])
      continue;

    Real local = _current_tally[score](i++);

    // divide each tally value by the volume that it corresponds to in MOOSE
    // because we will apply it as a volumetric tally
    Real volumetric_power = local * _openmc_problem.tallyMultiplier(score)
                          / _openmc_problem.getCellToElementVol(cell_info);
    total += local;

    fillElementalAuxVariable(var_numbers[score], c.second, volumetric_power);
  }

  return total;
}

void
CellTally::checkCellMappedSubdomains()
{
  // If the OpenMC cell maps to multiple subdomains that _also_ have different
  // tally settings, we need to error because we are unsure of whether to add tallies or not;
  // both of these need to be true to error
  for (const auto & c : _openmc_problem.cellToElem())
  {
    bool at_least_one_in_tallies = false;
    bool at_least_one_not_in_tallies = false;
    int block_in_tallies, block_not_in_tallies;

    auto cell_info = c.first;
    auto cell_subdomains = _openmc_problem.getCellToElementSub(cell_info);
    for (const auto & s : cell_subdomains)
    {
      if (!at_least_one_in_tallies)
      {
        at_least_one_in_tallies = _tally_blocks.count(s) != 0;
        block_in_tallies = s;
      }

      if (!at_least_one_not_in_tallies)
      {
        at_least_one_not_in_tallies = _tally_blocks.count(s) == 0;
        block_not_in_tallies = s;
      }

      // can cut the search early if we've already hit multiple tally settings
      if (at_least_one_in_tallies && at_least_one_not_in_tallies)
        break;
    }

    if (at_least_one_in_tallies && at_least_one_not_in_tallies)
      mooseError("cell " + _openmc_problem.printCell(cell_info) +
                 " maps to blocks with different tally settings!\n"
                 "Block " +
                 Moose::stringify(block_in_tallies) +
                 " is in 'tally_blocks', but "
                 "block " +
                 Moose::stringify(block_not_in_tallies) + " is not.");

    _cell_has_tally[cell_info] = at_least_one_in_tallies;
  }
}
#endif
