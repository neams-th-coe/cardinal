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
      "blocks",
      "Subdomains for which to add tallies in OpenMC. If not provided, cell "
      "tallies will be applied over the entire mesh.");
  params.addParam<bool>(
      "check_equal_mapped_tally_volumes",
      false,
      "Whether to check if the tallied cells map to regions in the mesh of equal volume. "
      "This can be helpful to ensure that the volume normalization of OpenMC's tallies doesn't "
      "introduce any unintentional distortion just because the mapped volumes are different. "
      "You should only set this to true if your OpenMC tally cells are all the same volume!");
  params.addRangeCheckedParam<Real>("equal_tally_volume_abs_tol",
                                    1e-8,
                                    "equal_tally_volume_abs_tol > 0",
                                    "Absolute tolerance for comparing tally volumes");

  return params;
}

CellTally::CellTally(const InputParameters & parameters)
  : TallyBase(parameters),
    _check_equal_mapped_tally_volumes(getParam<bool>("check_equal_mapped_tally_volumes")),
    _equal_tally_volume_abs_tol(getParam<Real>("equal_tally_volume_abs_tol"))
{
  if (isParamValid("blocks"))
  {
    auto block_names = getParam<std::vector<SubdomainName>>("blocks");
    if (block_names.empty())
      mooseError("Subdomain names must be provided if using 'blocks'!");

    auto block_ids = _mesh.getSubdomainIDs(block_names);
    std::copy(
        block_ids.begin(), block_ids.end(), std::inserter(_tally_blocks, _tally_blocks.end()));

    // Check to make sure all of the blocks are in the mesh.
    const auto & subdomains = _mesh.meshSubdomains();
    for (std::size_t b = 0; b < block_names.size(); ++b)
      if (subdomains.find(block_ids[b]) == subdomains.end())
        mooseError("Block '" + block_names[b] + "' specified in 'blocks' not found in mesh!");
  }
  else
  {
    // Tally over all mesh blocks if no blocks are provided.
    for (const auto & s : _mesh.meshSubdomains())
      _tally_blocks.insert(s);
  }
}

std::pair<unsigned int, openmc::Filter *>
CellTally::spatialFilter()
{
  // Check to make sure we can map tallies to the mesh subdomains requested in tally_blocks.
  checkCellMappedSubdomains();

  if (_openmc_problem.cellToElem().size() == 0)
    mooseError("Did not find any overlap between MOOSE elements and OpenMC cells for "
               "the specified blocks!");

  auto tally_cells = getTallyCells();
  std::vector<openmc::CellInstance> cells;
  for (const auto & c : tally_cells)
    cells.push_back(
        {gsl::narrow_cast<gsl::index>(c.first), gsl::narrow_cast<gsl::index>(c.second)});

  _cell_filter = dynamic_cast<openmc::CellInstanceFilter *>(openmc::Filter::create("cellinstance"));
  _cell_filter->set_cell_instances(cells);

  return std::make_pair(openmc::model::tally_filters.size() - 1, _cell_filter);
}

Real
CellTally::storeResultsInner(const std::vector<unsigned int> & var_numbers,
                             unsigned int local_score,
                             unsigned int global_score,
                             std::vector<xt::xtensor<double, 1>> tally_vals,
                             bool norm_by_src_rate)
{
  Real total = 0.0;

  for (unsigned int ext_bin = 0; ext_bin < _num_ext_filter_bins; ++ext_bin)
  {
    int i = 0;
    for (const auto & c : _openmc_problem.cellToElem())
    {
      auto cell_info = c.first;

      // if this cell doesn't have any tallies, skip it
      if (!_cell_has_tally[cell_info])
        continue;

      Real power_fraction = tally_vals[local_score](ext_bin * _cell_filter->n_bins() + i++);

      // divide each tally value by the volume that it corresponds to in MOOSE
      // because we will apply it as a volumetric tally
      Real volumetric_power = power_fraction;
      volumetric_power *= norm_by_src_rate ? _openmc_problem.tallyMultiplier(global_score) /
                                                 _openmc_problem.cellMappedVolume(cell_info)
                                           : 1.0;
      total += power_fraction;

      auto var = var_numbers[_num_ext_filter_bins * local_score + ext_bin];
      fillElementalAuxVariable(var, c.second, volumetric_power);
    }
  }

  return total;
}

void
CellTally::checkCellMappedSubdomains()
{
  _cell_has_tally.clear();

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
                 " is in 'blocks', but "
                 "block " +
                 Moose::stringify(block_not_in_tallies) + " is not.");

    _cell_has_tally[cell_info] = at_least_one_in_tallies;
  }
}

std::vector<OpenMCCellAverageProblem::cellInfo>
CellTally::getTallyCells() const
{
  bool is_first_tally_cell = true;
  OpenMCCellAverageProblem::cellInfo first_tally_cell;
  Real mapped_tally_volume;

  std::vector<OpenMCCellAverageProblem::cellInfo> tally_cells;

  for (const auto & c : _openmc_problem.cellToElem())
  {
    auto cell_info = c.first;

    if (_cell_has_tally.at(cell_info))
    {
      tally_cells.push_back(cell_info);

      if (is_first_tally_cell)
      {
        is_first_tally_cell = false;
        first_tally_cell = cell_info;
        mapped_tally_volume = _openmc_problem.cellMappedVolume(first_tally_cell);
      }

      if (_check_equal_mapped_tally_volumes)
      {
        Real diff = std::abs(mapped_tally_volume - _openmc_problem.cellMappedVolume(cell_info));
        bool absolute_diff = diff > _equal_tally_volume_abs_tol;
        bool relative_diff = diff / mapped_tally_volume > 1e-3;
        if (absolute_diff && relative_diff)
        {
          std::stringstream msg;
          msg << "Detected un-equal mapped tally volumes!\n cell "
              << _openmc_problem.printCell(first_tally_cell) << " maps to a volume of "
              << Moose::stringify(_openmc_problem.cellMappedVolume(first_tally_cell))
              << " (cm3)\n cell " << _openmc_problem.printCell(cell_info) << " maps to a volume of "
              << Moose::stringify(_openmc_problem.cellMappedVolume(cell_info))
              << " (cm3).\n\n"
                 "If the tallied cells in your OpenMC model are of identical volumes, this means "
                 "that you can get\n"
                 "distortion of the volumetric tally output. For instance, suppose you have "
                 "two equal-size OpenMC\n"
                 "cells which have the same volume - but each OpenMC cell maps to a MOOSE region "
                 "of different volume\n"
                 "just due to the nature of the centroid mapping scheme. Even if those two tallies "
                 "do actually have the\n"
                 "same value, the volumetric tally will be different because you'll be "
                 "dividing each tally by a\n"
                 "different mapped MOOSE volume.\n\n";

          if (_openmc_problem.hasPointTransformations())
            msg << "NOTE: You have imposed symmetry, which means that you'll hit this error if any "
                   "of your tally\n"
                   "cells are cut by symmetry planes. If your tally cells would otherwise be the "
                   "same volume if NOT\n"
                   "imposing symmetry, or if your tally cells are not the same volume regardless, "
                   "you need to set\n"
                   "'check_equal_mapped_tally_volumes = false'.";
          else
            msg << "We recommend re-creating the mesh mirror to have an equal volume mapping of "
                   "MOOSE elements to each\n"
                   "OpenMC cell. Or, you can disable this check by setting "
                   "'check_equal_mapped_tally_volumes = false'.";

          mooseError(msg.str());
        }
      }
    }
  }

  return tally_cells;
}
#endif
