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
#include "openmc/tallies/filter_cell.h"
#include "openmc/tallies/filter_cell_instance.h"
#include "openmc/tallies/filter_mesh.h"
#include "openmc/mesh.h"
#include "openmc/tallies/tally.h"
#include "CardinalEnums.h"
#include "SymmetryPointGenerator.h"

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
 * control where multiphysics feedback is sent to OpenMC, and where heat source
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

  virtual ~OpenMCCellAverageProblem() override;

  /**
   * Add 'heat_source', 'temp', and, if any fluid blocks are specified, a
   * 'density' variable. These are used to communicate OpenMC's solution with MOOSE,
   * and for MOOSE to communicate its solution with OpenMC.
   */
  virtual void addExternalVariables() override;

  virtual void externalSolve() override;

  virtual void syncSolutions(ExternalProblem::Direction direction) override;

  virtual bool converged() override { return true; }

  /**
   * Whether transformations are applied to the [Mesh] points when mapping to OpenMC
   * @return whether transformations are applied
   */
  virtual bool hasPointTransformations() const { return _symmetry != nullptr; }

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
   * class so that the total heat source as computed by an
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
  const coupling::CouplingFields cellCouplingFields(const cellInfo & cell_info);

  /**
   * Get a descriptive, formatted, string describing a cell
   * @param[in] cell_info cell index, instance pair
   * @return descriptive string describing cell
   */
  std::string printCell(const cellInfo & cell_info) const;

  /**
   * Get the density conversion factor (multiplicative factor)
   * @return density conversion factor from kg/m3 to g/cm3
   */
  const Real & densityConversionFactor() const { return _density_conversion_factor; }

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

  /**
   * Compute relative error
   * @param[in] sum sum of scores
   * @param[in] sum_sq sum of scores squared
   * @param[in] n_realizations number of realizations
   */
  Real relativeError(const Real & sum, const Real & sum_sq, const int & n_realizations) const;

  /// Constant flag to indicate that a cell/element was unmapped
  static constexpr int32_t UNMAPPED{-1};

protected:
  /**
   * Gather the _cell_to_elem structure distributed across ranks so that all information
   * on the cell mapping is available to all ranks.
   */
  void gatherCellToElem();

  /**
   * Get the element coupling phase
   * @param[in] elem
   * @return coupling phase
   */
  const coupling::CouplingFields elemPhase(const Elem * elem) const;

  /**
   * Read the parameters needed for triggers
   * @param[in] params input parameters
   */
  void getTallyTriggerParameters(const InputParameters & params);

  /**
   * Read the block parameters and tally information based on user settings
   * @param[in] name phase that these blocks will be mapped to
   * @param[in] blocks list of block ids to write
   */
  void readBlockParameters(const std::string name, std::unordered_set<SubdomainID> & blocks);

  /// Read the parameters for 'fluid_blocks'
  void readFluidBlocks() { readBlockParameters("fluid", _fluid_blocks); }

  /// Read the parameters for 'solid_blocks'
  void readSolidBlocks() { readBlockParameters("solid", _solid_blocks); }

  /// Read the parameters for 'tally_blocks'
  void readTallyBlocks() { readBlockParameters("tally", _tally_blocks); }

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
   * is used to get the heat source from a mesh tally onto the [Mesh], we require that the
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
   * Compute the number of digits required to display an integer
   * @param[in] number number to display
   */
  int digits(const int & number) const;

  /**
   * Compute the mean value of a tally
   * @param[in] tally OpenMC tallies (multiple if repeated mesh tallies)
   * @return mean value
   */
  double tallySum(std::vector<openmc::Tally *> tally) const;

  void relaxAndNormalizeHeatSource(const int & t);

  /**
   * Loop over all the OpenMC cells and count the number of MOOSE elements to which the cell
   * is mapped based on phase. This function is used to ensure that each OpenMC cell only maps
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

  /// Add tallies for the fluid and/or solid cells
  void initializeTallies();

  /// Find the material filling each fluid cell
  void getMaterialFills();

  /**
   * Get one point inside each cell, for accelerating the particle search routine.
   * This function will get the centroid of the first global element in the cell.
   */
  void getPointInCell();

  /**
   * Check whether the power in a particular tally bin is zero, which will throw
   * an error if 'check_zero_tallies = true'.
   * @param[in] power_fraction fractional power of the bin
   * @param[in] descriptor string to use in formatting the error message content
   */
  void checkZeroTally(const Real & power_fraction, const std::string & descriptor) const;

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
   * Extract the heat source from OpenMC and normalize by a global kappa fission tally,
   * then apply as a uniform field to the corresponding MOOSE elements.
   */
  void getHeatSourceFromOpenMC();

  /**
   * Get the fission tally standard deviation as a function of space and store into variable
   * @param[in] var_num variable number to store the standard deviation in
   */
  void getFissionTallyStandardDeviationFromOpenMC(const unsigned int & var_num);

  /**
   * Get the fission tally (i.e. raw, unrelaxed output from OpenMC)
   *  as a function of space and store into variable
   * @param[in] var_num variable number to store the tally in
   */
  void getFissionTallyFromOpenMC(const unsigned int & var_num);

  /**
   * Normalize the local tally by either the global kappa fission tally, or the sum
   * of the local kappa fission tally
   * @param[in] tally_result value of tally result
   * @return normalized tally
   */
  Real normalizeLocalTally(const Real & tally_result) const;

  /**
   * Normalize the local tally by either the global kappa fission tally, or the sum
   * of the local kappa fission tally
   * @param[in] raw_tally value of tally result
   * @return normalized tally
   */
  xt::xtensor<double, 1> normalizeLocalTally(const xt::xtensor<double, 1> & raw_tally) const;

  /**
   * Add the local kappa-fission tally
   * @param[in] filters tally filters
   * @param[in] estimator estimator type
   */
  void addLocalTally(std::vector<openmc::Filter *> & filters,
                     const openmc::TallyEstimator estimator);

  /**
   * Check the sum of the fluid and solid tallies (if present) against the global
   * kappa fission tally.
   */
  void checkTallySum() const;

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
   * Get the fill of an OpenMC cell
   * @param[in] cell_info cell ID, instance
   * @param[out] fill_type fill type of the cell, one of MATERIAL, UNIVERSE, or LATTICE
   * @return indices of material fills
   */
  std::vector<int32_t> cellFill(const cellInfo & cell_info, int & fill_type) const;

  /**
   * Whether a cell contains any fissile materials; for now, this simply returns true for
   * cells filled by universes or lattices because we have yet to implement something more
   * sophisticated that recurses down into all the fills
   * @param[in] cell_info cell ID, instance
   * @return whether cell contains fissile material
   */
  bool cellHasFissileMaterials(const cellInfo & cell_info) const;

  /// Extract user-specified additional output fields from OpenMC
  void extractOutputs();

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
   * Type of tally to apply to extract kappa fission score from OpenMC;
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

  /**
   * Type of relaxation to apply to the OpenMC solution; relaxation is
   * applied to the heat source tally.
   */
  const relaxation::RelaxationEnum _relaxation;

  /**
   * Type of trigger to apply to OpenMC kappa-fission tallies to indicate when
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

  /// Whether a mesh scaling was specified by the user
  const bool _specified_scaling;

  /**
   * Multiplicative factor to apply to the mesh in the [Mesh] block in order to convert
   * whatever units that mesh is in into OpenMC's length scale of centimeters. For instance,
   * it is commonplace to develop fuel performance models with a length scale of meters.
   * Rather than onerously convert all MOOSE inputs to which OpenMC will be coupled to units
   * of centimeters (which might not even be possible depending on how material properties are
   * calculated for those other applications, like if thermal conductivity is computed by a
   * module in units of W/m/K), this parameter allows us to scale the mesh over which we loop
   * to identify a coupling to OpenMC. Note that this does not actually scale the mesh itself,
   * but simply multiplies the mesh coordinates by this parameter when identifying the mapping
   * between elements and cells.
   *
   * By default, this parameter is set to 1.0, meaning that OpenMC is coupled to another
   * MOOSE application with an input written in terms of centimeters. Set this parameter to 100
   * if the coupled MOOSE application is in units of meters, for instance.
   *
   * To summarize by example - if the MOOSE application uses units of meters, with a mesh
   * named mesh.exo, then the OpenMC-wrapped input file should also use that mesh (with
   * units of meters) in its [Mesh] block (or perhaps a coarser version of that mesh if
   * the resolution of coupling does not need to match - the units just have to be the same).
   * Then, you should set 'scaling = 100.0' so that the mapping is performed correctly.
   */
  const Real & _scaling;

  /**
   * How to normalize the OpenMC kappa-fission tally into units of W/volume. If 'true',
   * normalization is performed by dividing each local tally against a problem-global
   * kappa-fission tally. The advantage of this approach is that some power-producing parts of the
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
   * power of 1 MW and you normalized based on a global kappa fission tally, you'd always
   * miss some of that power when sending to MOOSE. So, in this case, it is better to
   * normalize against the local tally itself so that the correct power is preserved.
   */
  const bool & _normalize_by_global;

  /**
   * Whether to check the tallies against the global kappa fission tally;
   * if set to true, and the tallies added for the 'tally_blocks' do not
   * sum to the global kappa fission tally, an error is thrown. If you are
   * only performing multiphysics feedback for, say, a single assembly in a
   * full-core OpenMC model, you must set this check to false, because there
   * are known fission sources outside the domain of interest.
   *
   * If not specified, then this is set to 'true' if normalizing by a global
   * tally, and to 'false' if normalizing by the local tally (because when we choose
   * to normalize by the local tally, we're probably using mesh tallies). But you can
   * of course still set a value for this parameter to override the default.
   */
  const bool & _check_tally_sum;

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

  /**
   * Whether tallies should be added to the fluid phase; this should be true if you have
   * a fissile fluid phase and wish to couple that heat source to MOOSE.
   */
  bool _add_tallies_to_fluid;

  /**
   * Whether tallies should be added to the solid phase; this should be true if you have
   * a fissile solid phase and wish to couple that heat source to MOOSE.
   */
  bool _add_tallies_to_solid;

  /// Blocks in MOOSE mesh that correspond to the fluid phase
  std::unordered_set<SubdomainID> _fluid_blocks;

  /// Blocks in MOOSE mesh that correspond to the solid phase
  std::unordered_set<SubdomainID> _solid_blocks;

  /// Blocks (mapped to OpenMC cells) for which to add tallies
  std::unordered_set<SubdomainID> _tally_blocks;

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

  /// Material filling each cell
  std::map<cellInfo, int32_t> _cell_to_material;

  /// Material-type cells contained within a cell
  std::map<cellInfo, containedCells> _cell_to_contained_material_cells;

  /// Number of material-type cells contained within a cell
  std::map<cellInfo, int32_t> _cell_to_n_contained;

  /// OpenMC cells to which a kappa fission tally is to be added
  std::vector<cellInfo> _tally_cells;

  /// Global kappa fission tally
  openmc::Tally * _global_tally{nullptr};

  /**
   * Local kappa fission tallies; multiple tallies will only exist when
   * translating multiple unstructured meshes throughout the geometry
   */
  std::vector<openmc::Tally *> _local_tally;

  /// OpenMC unstructured mesh instance for use of mesh tallies
  const openmc::LibMesh * _mesh_template;

  /// Heat source variable
  unsigned int _heat_source_var;

  /// Temperature variable
  unsigned int _temp_var;

  /// Density variable, which must be in units of kg/m3 based on internal conversions
  unsigned int _density_var;

  /// Mean value of the global kappa fission tally
  Real _global_kappa_fission;

  /// Mean value of the local kappa fission tally
  Real _local_kappa_fission;

  /// Conversion unit to transfer between kg/m3 and g/cm3
  static constexpr Real _density_conversion_factor{0.001};

  /**
   * Number of digits to use to display the cell ID for diagnostic messages; this is
   * estimated conservatively based on the total number of cells, even though there
   * may be distributed cells such that the maximum cell ID is far smaller than the
   * total number of cells.
   */
  const int _n_cell_digits;

  /**
   * For OpenMC geometries with a single coordinate level, we define default behavior for
   * tally_blocks to be all of the subdomains in the MOOSE mesh.
   */
  const bool _using_default_tally_blocks;

  /**
   * Mesh template file to use for creating mesh tallies in OpenMC; currently, this mesh
   * must be identical to the mesh used in the [Mesh] block because a simple copy transfer
   * is used to extract the tallies and put on the application's mesh in preparation for
   * a transfer to another MOOSE app.
   * TODO: allow the mesh to not be identical, both in terms of using different units
   * and more general differences like not having a particular phase present
   */
  std::string _mesh_template_filename;

  /**
   * Whether the present transfer is the first transfer; because ExternalProblem::solve
   * always does the transfer TO the multiapp first, this is also synonymous with the
   * first incoming transfer, with respect to whether that transfer is skipped by setting
   * the 'skip_first_incoming_transfer' parameter
   */
  static bool _first_transfer;

  /// ID used by OpenMC to indicate that a material fill is VOID
  static constexpr int MATERIAL_VOID{-1};

  /// Dummy particle to reduce number of allocations of particles for cell lookup routines
  openmc::Particle _particle;

  /**
   * Translations to apply to the mesh template, in the event that the mesh should be
   * repeated throughout the geometry. For instance, in pincell type geometries, you can
   * use this feature to repeat the same cylinder mesh multiple times throughout the domain.
   */
  std::vector<Point> _mesh_translations;

  /// OpenMC mesh filters for unstructured mesh tallies
  std::vector<const openmc::MeshFilter *> _mesh_filters;

  /// OpenMC solution fields to output to the mesh mirror
  const MultiMooseEnum * _outputs = nullptr;

  /// Numeric identifiers for the external variables
  std::vector<unsigned int> _external_vars;

  /// Spatial dimension of the Monte Carlo problem
  static constexpr int DIMENSION{3};

  /// Total number of particles simulated
  unsigned int _total_n_particles;

  /// Number of particles simulated in the first iteration
  unsigned int _n_particles_1;

  /// Threshold to use for accepting tallies when using triggers
  Real _tally_trigger_threshold;

  /**
   * Current fixed point iteration tally result; for instance, when using constant
   * relaxation, the heat source is updated as:
   * q(n+1) = (1-a) * q(n) + a * PHI(q(n), s)
   * where q(n+1) is _current_mean_tally, a is the relaxation factor, q(n)
   * is _previous_mean_tally, and PHI is the most-recently-computed tally result
   * (available locally in the heat source update function).
   */
  std::vector<xt::xtensor<double, 1>> _current_mean_tally;

  /// Previous fixed point iteration tally result (after relaxation)
  std::vector<xt::xtensor<double, 1>> _previous_mean_tally;

  /**
   * Variables to "collate" together (presumably from separate MOOSE apps)
   * together into the 'temp' variable that OpenMC reads from
   */
  const std::vector<std::string> * _temperature_vars;

  /**
   * Blocks of temperature to "collate" together (presumably from separate MOOSE apps)
   * together into the 'temp' variable that OpenMC reads from
   */
  const std::vector<SubdomainName> * _temperature_blocks;

  /// Helper utility to rotate [Mesh] points according to symmetry in OpenMC model
  std::unique_ptr<SymmetryPointGenerator> _symmetry;

private:
  /**
   * Update the number of particles according to the Dufek-Gudowski relaxation scheme
   */
  void dufekGudowskiParticleUpdate();
};
