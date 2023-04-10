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

#pragma once

#include "OpenMCProblemBase.h"
#include "SymmetryPointGenerator.h"
#include "OpenMCVolumeCalculation.h"

#include "openmc/tallies/filter_mesh.h"

#ifdef ENABLE_DAGMC
#include "MoabSkinner.h"
#include "DagMC.hpp"
#endif

/**
 * Mapping of OpenMC to a collection of MOOSE elements, with temperature feedback
 * on solid cells and both temperature and density feedback on fluid cells. The
 * mapping is established automatically based on looping over all the MOOSE elements
 * and finding the OpenMC cell at each element's centroid.
 *
 * All feedback into OpenMC is performed via element averages, while all tallies
 * in OpenMC are averaged over cells. The 'fluid_blocks' parameter is used to
 * indicate which blocks in the MOOSE domain should be used to send density and
 * temperature to OpenMC, while the 'solid_blocks' parameter is used to indicate
 * which blocks in the MOOSE domain should be used to send temperature (and *not*
 * density) to OpenMC. Tallies are automatically added to the OpenMC cells that
 * correspond to the 'tally_blocks' parameter. Therefore, you can individually
 * control where multiphysics feedback is sent to OpenMC, and where tally
 * feedback is received by MOOSE. If you have a single-level OpenMC geometry,
 * you can omit 'tally_blocks', in which case tallies are added to all MOOSE blocks.
 *
 * There are a number of limitations to this class (all of which are tested):
 *  - Each OpenMC cell shall only map to a single "phase", or else it is unclear
 *    whether that cell should receive temperature *and* density feedback, or
 *    only temperature feedback.
 *     TODO: If this is too restrictive in the future, we could implement some type
 *           of weighted averaging process. Also, if a cell maps to a phase and an
 *           unmapped region, perhaps we want to allow that.
 *  - Each OpenMC cell shall only map to a single tally type (on/off), or else
 *    it is unclear whether that cell should be tallied. For instance, if an OpenMC
 *    cell maps to elements that are in both fuel and cladding, and we've only added
 *    tallies to the fuel, we should error.
 *
 * Other limitations that are not tested (you won't error if you encounter these), are:
 *  - For the fluid, all the cells that we want to tally are on the same level in
 *    the OpenMC geometry. This level is specified with the 'fluid_cell_level' parameter.
 *    A similar restriction applies to the solid, with the 'solid_cell_level' parameter.
 *
 * Other considerations you should be aware of:
 *  - The density being transferred into OpenMC from MOOSE is in units of kg/m3; this
 *    is the unit employed by the MOOSE fluid properties module.
 *  - The temperature being transferred into OpenMC from MOOSE is in units of K; this
 *    is the unit employed by the MOOSE fluid properties module.
 *  - You will get some extra error checking at your disposal if your OpenMC geometry consists
 *    of a single coordinate level.
 */
class OpenMCCellAverageProblem : public OpenMCProblemBase
{
public:
  OpenMCCellAverageProblem(const InputParameters & params);
  static InputParameters validParams();

  /**
   * Initialize the mapping of OpenMC to the MooseMesh and perform any additional setup actions
   * like creating tallies.
   */
  void setupProblem();

  virtual void initialSetup() override;

  /**
   * Add the tally variable (name determined by 'tally_name'),
   * 'temp', and, if any fluid blocks are specified, a
   * 'density' variable. These are used to communicate OpenMC's solution with MOOSE,
   * and for MOOSE to communicate its solution with OpenMC.
   */
  virtual void addExternalVariables() override;

  /**
   * Re-generate the OpenMC materials in-place, needed for skinning operation where
   * we create new OpenMC materials on-the-fly in order to receive density feedback.
   */
  virtual void updateMaterials();

  virtual void externalSolve() override;

  virtual void syncSolutions(ExternalProblem::Direction direction) override;

  virtual bool converged() override { return true; }

  /**
   * Get the actual cell volume
   * @param[in] cell_info cell index, instance
   * @return stochastic OpenMC cell volume calculation
   */
  virtual Real cellVolume(const cellInfo & cell_info) const;

  /**
   * Reference to stochastic volume calculation
   * @return reference to stochastic volume calculation
   */
  virtual const OpenMCVolumeCalculation * volumeCalculation() const { return _volume_calc; }

  /**
   * Get the mapping of cells to MOOSE elements
   * @return mapping of cells to MOOSE elements
   */
  virtual const std::map<cellInfo, std::vector<unsigned int>> cellToElem() const { return _cell_to_elem; }

  /**
   * Get a list of each material in the problem, sorted by subdomain. This function also checks
   * that there is just one OpenMC material in each subdomain, necessary for the DAGMC skinning.
   * @return material in each subdomain
   */
  std::vector<std::string> getMaterialInEachSubdomain() const;

  /**
   * Whether transformations are applied to the [Mesh] points when mapping to OpenMC
   * @return whether transformations are applied
   */
  virtual bool hasPointTransformations() const { return _symmetry != nullptr; }

  /**
   * Get all the scores added to the tally
   * @return scores
   */
  virtual const std::vector<std::string> & getTallyScores() const { return _tally_score; }

  /**
   * Apply transformations to point
   * @param[in] point
   * @return transformed point
   */
  virtual Point transformPoint(const Point & pt) const
  {
    return this->hasPointTransformations() ? _symmetry->transformPoint(pt) : pt;
  }

  /**
   * Apply transformations and scale point from MOOSE into the OpenMC domain
   * @param[in] point
   * @return transformed point
   */
  Point transformPointToOpenMC(const Point & pt) const
  {
    Point pnt_out = transformPoint(pt);

    // scale point to OpenMC domain
    pnt_out *= _scaling;

    return pnt_out;
  }

  /**
   * Read from an OpenMC cell tally and write into an elemental aux variable
   * @param[in] var_num variable name to write
   * @param[in] tally tally values to write
   * @param[in] score tally score
   * @param[in] print_table whether to print the diagnostic table showing tally values by bin
   * @return sum of the tally
   */
  Real getCellTally(const unsigned int & var_num, const std::vector<xt::xtensor<double, 1>> & tally,
    const unsigned int & score, const bool & print_table);

  /**
   * Read from an OpenMC mesh tally and write into an elemental aux variable
   * @param[in] var_num variable name to write
   * @param[in] tally tally values to write
   * @param[in] score tally score
   * @param[in] print_table whether to print the diagnostic table showing tally values by bin
   * @return sum of the tally
   */
  Real getMeshTally(const unsigned int & var_num, const std::vector<xt::xtensor<double, 1>> & tally,
    const unsigned int & score, const bool & print_table);

  /**
   * Extract the (cell or mesh) tally from OpenMC and then apply to the corresponding MOOSE elements.
   * We also check that the tally normalization gives a total tally sum of 1.0 (when normalized against
   * the total tally value).
   * @param[in] var_num variable name to write
   * @param[in] tally tally values to write
   * @param[in] score tally score
   * @param[in] print_table whether to print the diagnostic table showing tally values by bin
   */
  void getTally(const unsigned int & var_num, const std::vector<xt::xtensor<double, 1>> & tally,
    const unsigned int & score, const bool & print_table);

  /**
   * Get the mesh filter(s) for tallies automatically constructed by Cardinal.
   * Multiple mesh filters are only created if the mesh template feature is used.
   * @return mesh filters
   */
  std::vector<openmc::Filter *> meshFilter();

  /**
   * This class uses elem->volume() in order to normalize the fission power produced
   * by OpenMC to conserve the specified power. However, as discussed on the MOOSE
   * slack channel,
   *
   * "elem->volume() is expensive, so whenever we do integration in moose we set
   *  _current_elem_volume to the volume as set by the sum of the quadrature weights"
   *
   * "The quadrature rule that we provide when you only have CONSTANT MONOMIALS is "
   * "insufficient for exactly integrating the element Jacobian mapping type (which "
   * "is FIRST LAGRANGE for a first order element), so you get an error relative to "
   * "the libmesh volume computation"
   *
   * So, we need to make sure that a minimum order quadrature rule is used with this
   * class so that the total tally as computed by an
   * ElementIntegralVariablePostprocessor actually matches the specified 'power'
   * (for low quadrature orders, there can be an error up to about 5% or so in total
   * power). This override simply forces the volume quadrature order to be 2 or higher
   * when using Gauss (default), monomial, or Gauss-Lobatto quadrature.
   *
   * For other quadrature rules, the approximations made in elem->volume() are never
   * going to match the volume integrations in MOOSE (unless the quadrature order is
   * very very high). For these orders, we print an error message informing the user
   * that they should switch to a different order.
   */
  virtual void createQRules(QuadratureType type,
                            Order order,
                            Order volume_order,
                            Order face_order,
                            SubdomainID block,
                            bool allow_negative_weights = true) override;

  /**
   * Type definition for cells contained within a parent cell; the first value
   * is the cell index, while the second is the set of cell instances
   */
  typedef std::unordered_map<int32_t, std::vector<int32_t>> containedCells;

  /**
   * Get the cell index from the element ID; will return UNMAPPED for unmapped elements
   * @param[in] elem_id element ID
   * @return cell index
   */
  int32_t elemToCellIndex(const int & elem_id) const { return elemToCellInfo(elem_id).first; }

  /**
   * Get the cell ID from the element ID. Note that this function requires that the elem_id
   * maps to an OpenMC cell, or else an error will be raised from OpenMC in cellID.
   * @param[in] elem_id element ID
   * @return cell ID
   */
  int32_t elemToCellID(const int & elem_id) const { return cellID(elemToCellIndex(elem_id)); }

  /**
   * Get the cell instance from the element ID; will return UNMAPPED for unmapped elements
   * @param[in] elem_id element ID
   * @return cell instance
   */
  int32_t elemToCellInstance(const int & elem_id) const { return elemToCellInfo(elem_id).second; }

  /**
   * Get the cell index, instance pair from element ID; if the element doesn't map to an OpenMC
   * cell, the index and instance are both set to UNMAPPED
   * @param[in] elem_id element ID
   * @return cell index, instance pair
   */
  cellInfo elemToCellInfo(const int & elem_id) const { return _elem_to_cell[elem_id]; }

  /**
   * Get the cell material index based on index, instance pair. Note that this function requires
   * a valid instance, index pair for cellInfo - you cannot pass in an unmapped cell, i.e.
   * (UNMAPPED, UNMAPPED)
   * @param[in] cell_info cell index, instance pair
   * @return material index
   */
  int32_t cellToMaterialIndex(const cellInfo & cell_info) { return _cell_to_material[cell_info]; }

  /**
   * Get the first material cell contained in the given cell
   * @param[in] cell_info cell index, instance pair
   * @return material cell index, instance pair
   */
  cellInfo containedMaterialCell(const cellInfo & cell_info);

  /**
   * Get the fields coupled for each cell; because we require that each cell map to a single phase,
   * we simply look up the coupled fields of the first element that this cell maps to. Note that
   * this function requires a valid instance, index pair for cellInfo - you cannot pass in an
   * unmapped cell, i.e. (UNMAPPED, UNMAPPED)
   * @param[in] cell_info cell index, instance pair
   * @return coupling fields
   */
  coupling::CouplingFields cellCouplingFields(const cellInfo & cell_info);

  const std::vector<openmc::Tally *> & getLocalTally() const { return _local_tally; }

  /**
   * Get the temperature of a cell; for cells not filled with materials, this will return
   * the temperature of the first material-type cell
   * @param[in] cell_info cell ID, instance
   */
  double cellTemperature(const cellInfo & cell_info);

  /**
   * Get the volume that each OpenMC cell mapped to
   * param[in] cell_info cell ID, instance
   */
  double cellMappedVolume(const cellInfo & cell_info);

  /// Reconstruct the DAGMC geometry after skinning
  void reloadDAGMC();

  /// Constant flag to indicate that a cell/element was unmapped
  static constexpr int32_t UNMAPPED{-1};

protected:
  /// Loop over the mapped cells, and build a map between subdomains to OpenMC materials
  void subdomainsToMaterials();

  /**
   * Get a set of all subdomains that have at least 1 element coupled to an OpenMC cell
   * @return subdomains with at least 1 element coupled to OpenMC
   */
  std::set<SubdomainID> coupledSubdomains();

  /**
   * Gather a vector of values to be summed for each cell
   * @param[in] local local values to be summed for the cells
   * @param[out] global global mapping of the summed values to the cells
   */
  template <typename T>
  void gatherCellSum(std::vector<T> & local, std::map<cellInfo, T> & global);

  /**
   * Gather a vector of values to be pushed back to for each cell
   * @param[in] local local values to be pushed back for the cells
   * @param[in] n_local number of local values contributed to each cell
   * @param[out] global global mapping of the pushed back values to the cells
   */
  template <typename T>
  void gatherCellVector(std::vector<T> & local, std::vector<unsigned int> & n_local, std::map<cellInfo, std::vector<T>> & global);

  /**
   * Get the feedback which this element provides to OpenMC
   * @param[in] elem
   * @return coupling phase
   */
  coupling::CouplingFields elemFeedback(const Elem * elem) const;

  /**
   * Read the parameters needed for triggers
   * @param[in] params input parameters
   */
  void getTallyTriggerParameters(const InputParameters & params);

  /**
   * Read the block parameters and tally information based on user settings
   * @param[in] name phase that these blocks will be mapped to
   * @param[in] blocks list of block ids to write
   * @param[out] names subdomain names
   */
  void readBlockParameters(const std::string name, std::unordered_set<SubdomainID> & blocks, std::vector<SubdomainName> & names);

  /// Read the parameters for 'fluid_blocks'
  void readFluidBlocks() { readBlockParameters("fluid", _fluid_blocks, _fluid_block_names); }

  /// Read the parameters for 'solid_blocks'
  void readSolidBlocks() { readBlockParameters("solid", _solid_blocks, _solid_block_names); }

  /// Read the parameters for 'tally_blocks'
  void readTallyBlocks() { readBlockParameters("tally", _tally_blocks, _tally_block_names); }

  /**
   * Cache the material cells contained within each coupling cell;
   * depending on user settings, this may attempt to take shortcuts
   * by assuming each tally cell has the same fills
   */
  void cacheContainedCells();

  /**
   * Fill the cached contained cells data structure for a given cell
   * @param[in] cell_info cell to find contained material cells for
   * @param[in] hint location hint used to accelerate the search
   * @param[out] map contained cell map
   */
  void setContainedCells(const cellInfo & cell_info,
                         const Point & hint,
                         std::map<cellInfo, containedCells> & map);

  /**
   * Check that the structure of the contained material cells for two tally cells matches;
   * i.e. this checks that the keys are the same and that the *number* of instances
   * of each filling material cell match.
   * @param[in] cell_info tally cell information for printing error messages
   * @param[in] reference map we want to check against
   * @param[in] compare map we want to check
   */
  void checkContainedCellsStructure(const cellInfo & cell_info,
                                    containedCells & reference,
                                    containedCells & compare);

  /**
   * Set a minimum order for a volume quadrature rule
   * @param[in] volume_order order of the volume quadrature rule
   * @param[in] type string type of quadrature rule for printing a console message
   */
  void setMinimumVolumeQRules(Order & volume_order, const std::string & type);

  /// For keeping the output neat when using verbose
  std::string printNewline()
  {
    if (_verbose)
      return "\n";
    else
      return "";
  }

  /**
   * Check whether a vector extracted with getParam is empty
   * @param[in] vector vector
   * @param[in] name name to use for printing error if empty
   */
  template <typename T>
  void checkEmptyVector(const std::vector<T> & vector, const std::string & name) const;

  /**
   * Read the mesh translations from file data
   * @param[in] data
   */
  void readMeshTranslations(const std::vector<std::vector<double>> & data);

  /**
   * Check the setup of the mesh template and translations. Because a simple copy transfer
   * is used to write a mesh tally onto the [Mesh], we require that the
   * meshes are identical - both in terms of the element ordering and the actual dimensions of
   * each element. This function performs as many checks as possible to ensure that the meshes
   * are indeed identical.
   */
  void checkMeshTemplateAndTranslations();

  /**
   * Read the phase cell level and check against the maximum level across the OpenMC domain
   * @param[in] name phase to read the cell level for
   * @param[out] cell_level cell coordinate level
   */
  void getCellLevel(const std::string name, unsigned int & cell_level);

  /**
   * Loop over the elements in the MOOSE mesh and store whether that element corresponds
   * to fluid, solid, or neither based on the settings in the 'fluid_blocks' and 'solid_blocks'.
   */
  void storeElementPhase();

  /**
   * Relax the tally and normalize it so that it has units of power fraction (i.e. an
   * integral of unity, where that "integral" is over the entire OpenMC domain) if you set
   * 'normalize_by_global_tally = true', but only over the Cardinal-created tallies if you
   * instead set 'normalize_by_global_tally = false'.
   *
   * NOTE: This function relaxes the power _distribution_, and not the actual magnitude of the
   * power. That is, we relax the power distribution and then multiply it by the power
   * (for k-eigenvalue) or source strength (for fixed source) of _the current step_ before
   * applying it to MOOSE. If the magnitude of the power is constant in time, there is zero
   * error in this. But for fixed source simulations where the actual magnitude of the tally
   * can vary based on simulation (b/c we don't renormalize it in the sense that we do
   * for k-eigenvalue simulations), we are basically relaxing the distribution of the tally,
   * but then multiplying it by the _current_ mean tally magnitude.
   *
   * There will be very small errors in these approximations unless the power/source strength
   * change dramatically with iteration. But because relaxation is itself a numerical approximation,
   * this is still inconsequential at the end of the day as long as your problem has converged
   * the relaxed tally to the raw (unrelaxed) tally.
   * @param[in] t tally index within local tally
   * @param[in] score score
   */
  void relaxAndNormalizeTally(const int & t, const unsigned int & score);

  /**
   * Loop over all the OpenMC cells and count the number of MOOSE elements to which the cell
   * is mapped based on phase.
   */
  void getCellMappedPhase();

  /**
   * This function is used to ensure that each OpenMC cell only maps
   * to a single phase. This function is also used to check that if 'fluid_blocks' is specified,
   * that we map to at least one OpenMC cell (or else this is probably a mistake). The same check
   * is also performed for the 'solid_blocks'.
   */
  void checkCellMappedPhase();

  /**
   * Loop over all the OpenMC cells and find those for which we should add tallies. If the cell
   * doesn't have fissile material, we will also print a warning for single-level geoemtries.
   */
  void storeTallyCells();

  /**
   * Check that the same MOOSE block ID doesn't apepar in both the 'fluid_blocks' and
   * 'solid_blocks', or else it's not clear whether that block should exchange temperature and
   * density with MOOSE or just temperature alone.
   */
  void checkBlockOverlap();

  /// Loop over all the OpenMC cells and get the element subdomain IDs that map to each cell
  void getCellMappedSubdomains();

  /**
   * Loop over all the OpenMC cells and determine if a cell maps to more than one subdomain
   * that also has different tally settings (i.e. we would not know whether to add or not to
   * add tallies to the cell).
   */
  void checkCellMappedSubdomains();

  /**
   * Loop over all the OpenMC cells and compute the volume of the MOOSE elements that each
   * cell maps to
   */
  void computeCellMappedVolumes();

  /// Set up the mapping from MOOSE elements to OpenMC cells
  void initializeElementToCellMapping();

  /// Populate maps of MOOSE elements to OpenMC cells
  void mapElemsToCells();

  /// Add OpenMC tallies to facilitate the coupling
  void initializeTallies();

  /**
   * Reset any tallies previously added by Cardinal, by deleting them from OpenMC.
   * Also delete any mesh filters and meshes added to OpenMC for mesh filters.
   */
  void resetTallies();

  /// Find the material filling each fluid cell
  void getMaterialFills();

  /**
   * Get one point inside each cell, for accelerating the particle search routine.
   * This function will get the centroid of the first global element in the lowest
   * rank in the cell.
   */
  void getPointInCell();

  /**
   * Check whether the power in a particular tally bin is zero, which will throw
   * an error if 'check_zero_tallies = true'.
   * @param[in] power_fraction fractional power of the bin
   * @param[in] descriptor string to use in formatting the error message content
   * @param[in] score score to check
   */
  void checkZeroTally(const Real & power_fraction, const std::string & descriptor,
    const unsigned int & score) const;

  /**
   * Compute the product of volume with a field across ranks and sum into a global map
   * @param[in] var_num variable to weight with volume, mapped by subdomain ID
   * @param[in] phase phase to compute the operation for
   * @return volume-weighted field for each cell, in a global sense
   */
  std::map<cellInfo, Real> computeVolumeWeightedCellInput(
    const std::map<SubdomainID, std::pair<unsigned int, std::string>> & var_num,
    const coupling::CouplingFields * phase);

  /**
   * Send temperature from MOOSE to the OpenMC cells by computing a volume average
   * and applying a single temperature per OpenMC cell
   */
  void sendTemperatureToOpenMC();

  /**
   * Send density from MOOSE to the fluid OpenMC cells by computing a volume average
   * and applying a single density per OpenMC cell.
   */
  void sendDensityToOpenMC();

  /**
   * Multiplier on the normalized tally results; for fixed source runs,
   * we multiply the tally (which has units of eV/source)
   * by the source strength and the eV to joule conversion, while for k-eigenvalue runs, we
   * multiply the normalized tally (which is unitless and has an integral
   * value of 1.0) by the power.
   * @param[in] score tally score
   */
  Real tallyMultiplier(const unsigned int & score) const;

  /**
   * Factor by which to normalize a tally
   * @param[in] score tally score
   * @return value to divide tally sum by for normalization
   */
  Real tallyNormalization(const unsigned int & score) const;

  /**
   * Normalize the local tally by either the global tally, or the sum
   * of the local tally. For fixed source simulations, do nothing because the
   * tally result is not re-normalized to any integral quantity.
   * @param[in] tally_result value of tally result
   * @param[in] score tally score
   * @return normalized tally
   */
  template <typename T>
  T normalizeLocalTally(const T & tally_result, const unsigned int & score) const;

  /**
   * Add local tally
   * @param[in] score score type
   * @param[in] filters tally filters
   */
  void addLocalTally(const std::vector<std::string> & score, std::vector<openmc::Filter *> & filters);

  /**
   * Check the sum of the fluid and solid tallies (if present) against the global
   * tally.
   * @param[in] score tally score
   */
  void checkTallySum(const unsigned int & score) const;

  /**
   * Fill the mesh translations to be applied to each unstructured mesh; if no
   * translations are explicitly given, a translation of (0.0, 0.0, 0.0) is assumed.
   */
  void fillMeshTranslations();

  /**
   * Find the OpenMC cell at a given point in space in terms of the _particle members
   * @param[in] point point
   * @return whether OpenMC reported an error
   */
  bool findCell(const Point & point);

  /**
   * Checks that the contained material cells exactly match between a reference obtained
   * by calling openmc::Cell::get_contained_cells for each tally cell and a shortcut
   * approach that assumes all tally cells (which aren't simply just material fills)
   * has exactly the same contained material cells.
   * @param[in] reference reference map to compare against
   * @param[in] compare shortcut map to compare
   */
  void compareContainedCells(std::map<cellInfo, containedCells> & reference,
                             std::map<cellInfo, containedCells> & compare);

  std::unique_ptr<NumericVector<Number>> _serialized_solution;

  /**
   * Type of tally to apply to extract score from OpenMC;
   * if you want to tally in cells, use 'cell'. Otherwise, to tally on an
   * unstructured mesh, use 'mesh'. Currently, this implementation is limited
   * to a single mesh in the OpenMC geometry.
   * TODO: allow the same mesh to be repeated several times throughout the
   * OpenMC geometry
   */
  const tally::TallyTypeEnum _tally_type;

  /**
   * Where to get the initial OpenMC temperatures and densities from;
   * can be either hdf5 (from a properties.h5 file), xml (whatever is already
   * set in the XML files), or moose (meaning whatever ICs are set on the
   * 'temp' and 'density' variables).
   */
  const coupling::OpenMCInitialCondition _initial_condition;

  /// Type of relaxation to apply to the OpenMC tallies
  const relaxation::RelaxationEnum _relaxation;

  /**
   * Type of trigger to apply to OpenMC tallies to indicate when
   * the simulation is complete. These can be used to on-the-fly adjust the number
   * of active batches in order to reach some desired criteria (which is specified
   * by this parameter).
   */
  const tally::TallyTriggerTypeEnum _tally_trigger;

  /**
   * Type of trigger to apply to k eigenvalue to indicate when
   * the simulation is complete. These can be used to on-the-fly adjust the number
   * of active batches in order to reach some desired criteria (which is specified
   * by this parameter).
   */
  const tally::TallyTriggerTypeEnum _k_trigger;

  /**
   * Whether to check if any of the tallies evaluate to zero; if set to true,
   * and a tally is zero, an error is thrown. This can be helpful in identifying
   * cases where you added tallies, but there isn't any fissile material, or
   * if you have an error in the geometry or tally setup (such as using a CellFilter
   * when you should be using a CellInstanceFilter).
   */
  const bool & _check_zero_tallies;

  /**
   * Coordinate level in the OpenMC domain that fluid cells are located on,
   * for the purpose of setting up a cell filter for the fluid phase.
   */
  unsigned int _fluid_cell_level;

  /**
   * Coordinate level in the OpenMC domain that solid cells are located on,
   * for the purpose of setting up a cell filter for the solid phase.
   */
  unsigned int _solid_cell_level;

  /**
   * Whether the cell level should be taken as the lowest local level in the geometry
   * in the case that the lowest local level is *higher* than the _solid_cell_level.
   * In other words, if 'lowest_solid_cell' is specified, then in regions of the OpenMC
   * domain where the lowest level in the geometry is \f$N\f$ for \f$N<3\f$, but 'lowest_solid_cell'
   * is set to 3, then the actual level used in mapping is the locally lowest cell level.
   */
  bool _using_lowest_solid_level;

  /**
   * Whether the cell level should be taken as the lowest local level in the geometry
   * in the case that the lowest local level is *higher* than the _fluid_cell_level.
   * In other words, if 'lowest_fluid_cell' is specified, then in regions of the OpenMC
   * domain where the lowest level in the geometry is \f$N\f$ for \f$N<3\f$, but 'lowest_fluid_cell'
   * is set to 3, then the actual level used in mapping is the locally lowest cell level.
   */
  bool _using_lowest_fluid_level;

  /**
   * Whether OpenMC properties (temperature and density) should be exported
   * after being updated in syncSolutions.
   */
  const bool & _export_properties;

  /**
   * How to normalize the OpenMC tally into units of W/volume. If 'true',
   * normalization is performed by dividing each local tally against a problem-global
   * tally. The advantage of this approach is that some power-producing parts of the
   * OpenMC domain can be excluded from multiphysics feedback (without us having to guess
   * what the power of the *included* part of the domain is). This can let us do
   * "zooming" type calculations, where perhaps we only want to send T/H feedback to
   * one bundle in a full core.
   *
   * If 'false', normalization is performed by dividing each local tally by the sum
   * of the local tally itself. The advantage of this approach becomes evident when
   * using mesh tallies. If a mesh tally does not perfectly align with an OpenMC cell -
   * for instance, a first-order sphere mesh will not perfectly match the volume of a
   * TRISO pebble - then not all of the power actually produced in the pebble is
   * tallies on the mesh approximation to that pebble. Therefore, if you set a core
   * power of 1 MW and you normalized based on a global tally, you'd always
   * miss some of that power when sending to MOOSE. So, in this case, it is better to
   * normalize against the local tally itself so that the correct power is preserved.
   */
  const bool _normalize_by_global;

  /**
   * If 'fixed_mesh' is false, this indicates that the [Mesh] is changing during
   * the simulation (either from adaptive refinement or from deformation).
   * When the mesh changes during the simulation, the mapping from OpenMC cells to
   * the [Mesh] must be re-established after each OpenMC run.
   */
  const bool _need_to_reinit_coupling;

  /**
   * Whether to check the tallies against the global tally;
   * if set to true, and the tallies added for the 'tally_blocks' do not
   * sum to the global tally, an error is thrown. If you are
   * only performing multiphysics feedback for, say, a single assembly in a
   * full-core OpenMC model, you must set this check to false, because there
   * are known fission sources outside the domain of interest.
   *
   * If not specified, then this is set to 'true' if normalizing by a global
   * tally, and to 'false' if normalizing by the local tally (because when we choose
   * to normalize by the local tally, we're probably using mesh tallies). But you can
   * of course still set a value for this parameter to override the default.
   */
  const bool _check_tally_sum;

  /**
   * Whether to check that the [Mesh] volume each cell tally maps to is identical.
   * This is a useful helper function for OpenMC models where each cell tally has the
   * same volume (often the case for many reactor geometries). If the OpenMC model
   * cell tallies all are of the same spatial size, it's still possible that they
   * can map to different volumes in the MOOSE mesh if the MOOSE elements don't line
   * up with the edges of the OpenMC cells. Different volumes then can distort the
   * volume normalization that we do to convert the fission power to a volumetric
   * power (in a perfect world, we would actually divide OpenMC's tallies by the
   * results of a stochastic volume calculation in OpenMC, but that is too expensive).
   */
  const bool & _check_equal_mapped_tally_volumes;

  /// Absolute tolerance for checking equal tally mapped volumes
  const Real & _equal_tally_volume_abs_tol;

  /// Constant relaxation factor
  const Real & _relaxation_factor;

  /**
   * If known a priori by the user, whether the tally cells (which are not simply material
   * fills) have EXACTLY the same contained material cells. This is a big optimization for
   * TRISO problems in setting up homogenized temperature/density feedback to OpenMC.
   *
   * The concept can best be explained with a pebble bed reactor.
   * If every pebble is filled with an identical TRISO universe, then the material fills
   * in each pebble are identical to one another except for a constant offset. This idea
   * can be used to then skip all but the first two openmc::Cell::get_contained_cells
   * calls (which are required in order to figure out the pattern by which pebble N is
   * incremented relative to pebble 1).
   *
   * When using this parameter, we HIGHLY recommend setting 'check_identical_tally_cell_fills =
   * true' the first time you run your model. This will figure out the material cell fills using a
   * method that calls openmc::Cell::get_contained_cells for every tally cell, i.e. without assuming
   * anything about repeated structure in your OpenMC model. Setting 'identical_tally_cell_fills =
   * true' without also setting 'check_identical_tally_cell_fills = true' may result in SILENT
   * errors!!! So it is essential to be sure you've removed any error sources before you turn the
   * error check off to actually leverage the speedup.
   *
   * Note: for any tally cells that are just filled with a material, we use the approach
   * where openmc::Cell::get_contained_cells is called in full.
   *
   * This optimization will not work (and 'check_identical_tally_cells = true' *will*
   * catch these) for:
   * - any situation where tallied, non-material-fill pebbles have different fills
   *   (such as if you have different TRISO lattices in each pebble)
   * - any situation where there is a "gap" in the incrementing of the material fill
   *   instances (such as if pebble 89 does not map to 'tally_blocks', then the instance
   *   shift for pebble 90 relative to pebble 1 is 89, when it should have been 90).
   */
  const bool & _identical_tally_cell_fills;

  /**
   * Whether we should rigorously check that each tally cell has identical fills;
   * this is SLOW for large TRISO problems, but is essential to ensure the accuracy of
   * 'identical_tally_cell_fills = true'. Please set this parameter to 'true' at least
   * once before running production cases to be sure the optimization can be applied.
   */
  const bool & _check_identical_tally_cell_fills;

  /**
   * Whether it can be assumed that all of the tallies (both those set by the user
   * in the XML file, as well as those created automatically by Cardinal) are
   * spatially separate. This means that once a particle scores to one tally bin, it wouldn't
   * score to ANY other tally bins. This can dramatically increase tracking rates
   * for problems with many tallies.
   */
  const bool & _assume_separate_tallies;

  /**
   * Whether to map density according to each individual OpenMC cell (in which case an
   * error is thrown if you don't have a unique material in each cell) or by material.
   */
  bool _map_density_by_cell;

  /**
   * Whether the problem has fluid blocks specified; note that this is NOT necessarily
   * indicative that the mapping was successful in finding any cells corresponding to those blocks
   */
  const bool _has_fluid_blocks;

  /**
   * Whether the problem has solid blocks specified; note that this is NOT necessarily
   * indicative that the mapping was successful in finding any cells corresponding to those blocks
   */
  const bool _has_solid_blocks;

  /**
   * Whether a global tally is required for the sake of normalization and/or checking
   * the tally sum
   */
  const bool _needs_global_tally;

  /// Whether tallies should be added to the fluid phase
  bool _add_tallies_to_fluid;

  /// Whether tallies should be added to the solid phase
  bool _add_tallies_to_solid;

  /// Tally estimator to use for the OpenMC tallies created for multiphysics
  openmc::TallyEstimator _tally_estimator;

  /// OpenMC tally score(s) to write into the 'tally_name' auxiliary variable(s)
  std::vector<std::string> _tally_score;

  /// Auxiliary variable name(s) for the OpenMC tally(s)
  std::vector<std::string> _tally_name;

  /// Blocks in MOOSE mesh that correspond to the fluid phase
  std::unordered_set<SubdomainID> _fluid_blocks;

  /// Blocks in MOOSE mesh that correspond to the solid phase
  std::unordered_set<SubdomainID> _solid_blocks;

  /// Blocks (mapped to OpenMC cells) for which to add tallies
  std::unordered_set<SubdomainID> _tally_blocks;

  /// Blocks in MOOSE mesh that correspond to the fluid phase
  std::vector<SubdomainName> _fluid_block_names;

  /// Blocks in MOOSE mesh that correspond to the solid phase
  std::vector<SubdomainName> _solid_block_names;

  /// Blocks in MOOSE mesh that correspond to the solid phase
  std::vector<SubdomainName> _tally_block_names;

  /// Mapping of MOOSE elements to the OpenMC cell they map to (if any)
  std::vector<cellInfo> _elem_to_cell{};

  /// Phase of each cell
  std::map<cellInfo, coupling::CouplingFields> _cell_phase;

  /// Number of solid elements in the MOOSE mesh
  int _n_moose_solid_elems;

  /// Number of fluid elements in the MOOSE mesh
  int _n_moose_fluid_elems;

  /// Number of no-coupling elements in the MOOSE mesh
  int _n_moose_none_elems;

  /// Number of solid elements mapped to OpenMC cells
  int _n_mapped_solid_elems;

  /// Number of fluid elements mapped to OpenMC cells
  int _n_mapped_fluid_elems;

  /// Number of no-coupling elements mapped to OpenMC cells
  int _n_mapped_none_elems;

  /// Total volume of uncoupled MOOSE mesh elements
  Real _uncoupled_volume;

  /// Whether non-material cells are mapped
  bool _material_cells_only{true};

  /// Mapping of OpenMC cell indices to a vector of MOOSE element IDs
  std::map<cellInfo, std::vector<unsigned int>> _cell_to_elem;

  /// Mapping of OpenMC cell indices to a vector of MOOSE element IDs, on each local rank
  std::map<cellInfo, std::vector<unsigned int>> _local_cell_to_elem;

  /// Mapping of OpenMC cell indices to the subdomain IDs each maps to
  std::map<cellInfo, std::unordered_set<SubdomainID>> _cell_to_elem_subdomain;

  /// Mapping of elem subdomains to materials
  std::map<SubdomainID, std::set<int32_t>> _subdomain_to_material;

  /**
   * A point inside the cell, taken simply as the centroid of the first global
   * element inside the cell. This is stored to accelerate the particle search.
   */
  std::map<cellInfo, Point> _cell_to_point;

  /// Whether a cell index, instance pair should be added to the tally filter
  std::map<cellInfo, bool> _cell_has_tally;

  /**
   * Volume associated with the mapped element space for each OpenMC cell; the unit
   * for this volume is whatever is used in the [Mesh] block
   */
  std::map<cellInfo, Real> _cell_to_elem_volume;

  /**
   * Volume associated with the actual OpenMC cell, computed by an optional
   * OpenMCVolumeCalculation user object
   */
  std::map<cellInfo, Real> _cell_volume;

  /**
   * Material filling each cell to receive density & temperature feedback. We enforce
   * that these "fluid" cells are filled with a material (cannot be filled with a lattice
   * or universe).
   */
  std::map<cellInfo, int32_t> _cell_to_material;

  /// Material-type cells contained within a cell
  std::map<cellInfo, containedCells> _cell_to_contained_material_cells;

  /// Number of material-type cells contained within a cell
  std::map<cellInfo, int32_t> _cell_to_n_contained;

  /// OpenMC cells to which a tally is to be added
  std::vector<cellInfo> _tally_cells;

  /// Global tally
  openmc::Tally * _global_tally{nullptr};

  /**
   * Local tallies; multiple tallies will only exist when
   * translating multiple unstructured meshes throughout the geometry
   */
  std::vector<openmc::Tally *> _local_tally;

  /// OpenMC unstructured mesh instance for use of mesh tallies
  const openmc::LibMesh * _mesh_template;

  /// Tally variable(s)
  std::vector<unsigned int> _tally_var;

  /// Sum value of the global tally(s), across all bins
  std::vector<Real> _global_sum_tally;

  /// Sum value of the local tally(s), across all bins
  std::vector<Real> _local_sum_tally;

  /// Mean value of the local tally(s), across all bins; only used for fixed source mode
  std::vector<Real> _local_mean_tally;

  /**
   * Mesh template file to use for creating mesh tallies in OpenMC; currently, this mesh
   * must be identical to the mesh used in the [Mesh] block because a simple copy transfer
   * is used to extract the tallies and put on the application's mesh in preparation for
   * a transfer to another MOOSE app. If not set, this indicates that tallying will be
   * performed directly on the [Mesh].
   * TODO: allow the mesh to not be identical, both in terms of using different units
   * and more general differences like not having a particular phase present
   */
  const std::string * _mesh_template_filename = nullptr;

  /**
   * Whether the present transfer is the first transfer; because ExternalProblem::solve
   * always does the transfer TO the multiapp first, this is also synonymous with the
   * first incoming transfer, with respect to whether that transfer is skipped by setting
   * the 'skip_first_incoming_transfer' parameter
   */
  static bool _first_transfer;

  /// Dummy particle to reduce number of allocations of particles for cell lookup routines
  openmc::Particle _particle;

  /**
   * Translations to apply to the mesh template, in the event that the mesh should be
   * repeated throughout the geometry. For instance, in pincell type geometries, you can
   * use this feature to repeat the same cylinder mesh multiple times throughout the domain.
   */
  std::vector<Point> _mesh_translations;

  /// OpenMC mesh filters for unstructured mesh tallies
  std::vector<openmc::MeshFilter *> _mesh_filters;

  /// OpenMC solution fields to output to the mesh mirror
  const MultiMooseEnum * _outputs = nullptr;

  /// Suffixes to apply to 'tally_name' in order to name the fields in the 'output'
  std::vector<std::string> _output_name;

  /// Numeric identifiers for the external variables (for each score)
  std::vector<std::vector<unsigned int>> _external_vars;

  /// Spatial dimension of the Monte Carlo problem
  static constexpr int DIMENSION{3};

  /// Number of particles simulated in the first iteration
  unsigned int _n_particles_1;

  /// Threshold to use for accepting tallies when using triggers
  Real _tally_trigger_threshold;

  /**
   * Current fixed point iteration tally result; for instance, when using constant
   * relaxation, the tally is updated as:
   * q(n+1) = (1-a) * q(n) + a * PHI(q(n), s)
   * where q(n+1) is _current_tally, a is the relaxation factor, q(n)
   * is _previous_tally, and PHI is the most-recently-computed tally result
   * (the _current_raw_tally).
   */
  std::vector<std::vector<xt::xtensor<double, 1>>> _current_tally;

  /// Previous fixed point iteration tally result (after relaxation)
  std::vector<std::vector<xt::xtensor<double, 1>>> _previous_tally;

  /// Current "raw" tally output from Monte Carlo solution
  std::vector<std::vector<xt::xtensor<double, 1>>> _current_raw_tally;

  /// Current "raw" tally standard deviation
  std::vector<std::vector<xt::xtensor<double, 1>>> _current_raw_tally_std_dev;

  /// Mapping from temperature variable name to the subdomains on which to read it from
  std::map<std::string, std::vector<SubdomainName>> _temp_vars_to_blocks;

  /// Optional volume calculation for cells which map to MOOSE
  OpenMCVolumeCalculation * _volume_calc;

  /// Userobject that maps from a partial-symmetry OpenMC model to a whole-domain [Mesh]
  const SymmetryPointGenerator * _symmetry;

  /// Number of solid elements in each mapped OpenMC cell (global)
  std::map<cellInfo, int> _n_solid;

  /// Number of fluid elements in each mapped OpenMC cell (global)
  std::map<cellInfo, int> _n_fluid;

  /// Number of none elements in each mapped OpenMC cell (global)
  std::map<cellInfo, int> _n_none;

  /// Index in OpenMC tallies corresponding to the global tally added by Cardinal
  unsigned int _global_tally_index;

  /// Index in OpenMC tallies corresponding to the first local tally added by Cardinal
  unsigned int _local_tally_index;

  /// Index in OpenMC tally filters corresponding to the first filter added by Cardinal
  unsigned int _filter_index;

  /// Index in OpenMC meshes corresponding to the mesh tally (if used)
  unsigned int _mesh_index;

  /// Index in tally_score pointing to the score used for normalizing flux tallies in eigenvalue mode
  unsigned int _source_rate_index;

#ifdef ENABLE_DAGMC
  /// Optional skinner to re-generate the OpenMC geometry on-the-fly for DAGMC models
  MoabSkinner * _skinner = nullptr;

  /// Pointer to DAGMC
  std::shared_ptr<moab::DagMC> _dagmc = nullptr;
#endif

  /// Total number of unique OpenMC cell IDs + instances combinations
  long unsigned int _n_openmc_cells;

  /// Index in the OpenMC universes corresponding to the DAGMC universe
  int32_t _dagmc_universe_index;

  /// Conversion rate from eV to Joule
  static constexpr Real EV_TO_JOULE = 1.6022e-19;

  /// Tolerance for setting zero tally
  static constexpr Real ZERO_TALLY_THRESHOLD = 1e-12;

private:
  /**
   * Update the number of particles according to the Dufek-Gudowski relaxation scheme
   */
  void dufekGudowskiParticleUpdate();

  /// Flattened cell IDs collected after parallel communication
  std::vector<int32_t> _flattened_ids;

  /// Flattened cell instancess collected after parallel communication
  std::vector<int32_t> _flattened_instances;

  /// Mapping from subdomain IDs to which aux variable to read temperature (K) from
  std::map<SubdomainID, std::pair<unsigned int, std::string>> _subdomain_to_temp_vars;

  /// Mapping from subdomain IDs to which aux variable to read density (kg/m3) from
  std::map<SubdomainID, std::pair<unsigned int, std::string>> _subdomain_to_density_vars;
};
