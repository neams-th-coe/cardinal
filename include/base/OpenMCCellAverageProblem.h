#pragma once

#define LIBMESH

#include "ExternalProblem.h"
#include "openmc/tallies/filter_cell.h"
#include "openmc/tallies/filter_cell_instance.h"
#include "openmc/tallies/filter_mesh.h"
#include "openmc/mesh.h"
#include "openmc/tallies/tally.h"
#include "CardinalEnums.h"

class OpenMCCellAverageProblem;

template<>
InputParameters validParams<OpenMCCellAverageProblem>();

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
class OpenMCCellAverageProblem : public ExternalProblem
{
public:
  OpenMCCellAverageProblem(const InputParameters & params);
  virtual ~OpenMCCellAverageProblem() override;

  /**
   * Add 'heat_source', 'temp', and, if any fluid blocks are specified, a
   * 'density' variable. These are used to communicate OpenMC's solution with MOOSE,
   * and for MOOSE to communicate its solution with OpenMC.
   */
  virtual void addExternalVariables() override;

  /// Run a k-eigenvalue OpenMC simulation
  virtual void externalSolve() override;

  virtual void syncSolutions(ExternalProblem::Direction direction) override;

  virtual bool converged() override { return true; }

  /**
   * Type definition for storing the relevant aspects of the OpenMC geometry; the first
   * value is the cell index, while the second is the cell instance.
   */
  typedef std::pair<int32_t, int32_t> cellInfo;

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
   * Get the fields coupled for each cell; because we require that each cell map to a single phase,
   * we simply look up the coupled fields of the first element that this cell maps to. Note that
   * this function requires a valid instance, index pair for cellInfo - you cannot pass in an
   * unmapped cell, i.e. (UNMAPPED, UNMAPPED)
   * @param[in] cell_info cell index, instance pair
   * @return coupling fields
   */
  const coupling::CouplingFields cellCouplingFields(const cellInfo & cell_info);

  /**
   * Get the cell ID
   * @param[in] index cell index
   * @return cell ID
   */
  int32_t cellID(const int32_t index) const;

  /**
   * Get the material ID
   * @param[in] index material index
   * @return cell material ID
   */
  int32_t materialID(const int32_t index) const;

  /**
   * Get a descriptive, formatted, string describing a cell
   * @param[in] cell_info cell index, instance pair
   * @return descriptive string
   */
  std::string printCell(const cellInfo & cell_info) const;

  /**
   * Get a descriptive, formatted, string describing a material
   * @param[in] index material index
   * @return descriptive string
   */
  std::string printMaterial(const int32_t & index) const;

  /**
   * Get the density conversion factor (multiplicative factor)
   * @return density conversion factor from kg/m3 to g/cm3
   */
  const Real & densityConversionFactor() const { return _density_conversion_factor; }

  const std::vector<openmc::Tally *> & getLocalTally() const { return _local_tally; }

  /// Constant flag to indicate that a cell/element was unmapped
  static constexpr int32_t UNMAPPED {-1};

protected:
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

  /// For keeping the output neat when using verbose
  std::string printNewline() { if (_verbose) return "\n"; else return ""; }

  /**
   * Check whether a vector extracted with getParam is empty
   * @param[in] vector vector
   * @param[in] name name to use for printing error if empty
   */
  template <typename T> void checkEmptyVector(const std::vector<T> & vector,
    const std::string & name) const;

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
  void getCellLevel(const std::string name, int & cell_level);

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
   * Check that the same MOOSE block ID doesn't apepar in both the 'fluid_blocks' and 'solid_blocks',
   * or else it's not clear whether that block should exchange temperature and density with MOOSE
   * or just temperature alone.
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

  /// Add tallies for the fluid and/or solid cells
  void initializeTallies();

  /// Find the material filling each fluid cell
  void getMaterialFills();

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
   * Normalize the local tally by either the global kappa fission tally, or the sum
   * of the local kappa fission tally
   * @param[in] tally_result value of tally result
   * @return normalized tally
   */
  Real normalizeLocalTally(const Real & tally_result) const;

  /**
   * Add the local kappa-fission tally
   * @param[in] filters tally filters
   * @param[in] estimator estimator type
   */
  void addLocalTally(std::vector<openmc::Filter *> & filters, const openmc::TallyEstimator estimator);

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

  std::unique_ptr<NumericVector<Number>> _serialized_solution;

  /**
   * Type of filter to apply to extract kappa fission score from OpenMC;
   * if you want to tally in distributed cells, use 'cell_filter'; however,
   * this filter requires that those cells be filled with materials. Therefore,
   * we keep as an option the 'cell' filter, which does not discriminate based
   * on cell instance, but does allow non-material fills (you will just need to
   * be careful about how you set up the problem if you want to use lattices).
   */
  const filter::CellFilterEnum _tally_filter;

  /**
   * Type of tally to apply to extract kappa fission score from OpenMC;
   * if you want to tally in cells, use 'cell'. Otherwise, to tally on an
   * unstructured mesh, use 'mesh'. Currently, this implementation is limited
   * to a single mesh in the OpenMC geometry.
   * TODO: allow the same mesh to be repeated several times throughout the
   * OpenMC geometry
   */
  const tally::TallyTypeEnum _tally_type;

  /// Constant power for the entire OpenMC domain
  const Real & _power;

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
  int _fluid_cell_level;

  /**
   * Coordinate level in the OpenMC domain that solid cells are located on,
   * for the purpose of setting up a cell filter for the solid phase.
   */
  int _solid_cell_level;

  /// Whether to print diagnostic information about model setup and the transfers
  const bool & _verbose;

  /**
   * Whether to skip the first density and temperature transfer into OpenMC; this
   * can be used to apply OpenMC's initial values for density and temperature in its
   * XML files rather than whatever is transferred into OpenMC from MOOSE.
   */
  const bool & _skip_first_incoming_transfer;

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
  std::vector<cellInfo> _elem_to_cell {};

  /**
   * Phase of each element in the MOOSE mesh according to settings in the 'fluid_blocks'
   * and 'solid_blocks' parameters.
   */
  std::vector<coupling::CouplingFields> _elem_phase {};

  /// Number of solid elements in the MOOSE mesh
  int _n_moose_solid_elems;

  /// Number of fluid elements in the MOOSE mesh
  int _n_moose_fluid_elems;

  /// Number of no-coupling elements in the MOOSE mesh
  int _n_moose_none_elems;

  /// Mapping of OpenMC cell indices to a vector of MOOSE element IDs
  std::map<cellInfo, std::vector<unsigned int>> _cell_to_elem;

  /// Whether a cell index, instance pair should be added to the tally filter
  std::map<cellInfo, bool> _cell_has_tally;

  /**
   * Volume associated with the mapped element space for each OpenMC cell; the unit
   * for this volume is whatever is used in the [Mesh] block
   */
  std::map<cellInfo, Real> _cell_to_elem_volume;

  /// Material filling each cell
  std::map<cellInfo, int32_t> _cell_to_material;

  /// OpenMC cells to which a kappa fission tally is to be added
  std::vector<cellInfo> _tally_cells;

  /// Global kappa fission tally
  openmc::Tally * _global_tally {nullptr};

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
  static constexpr Real _density_conversion_factor {0.001};

  /**
   * Whether the OpenMC model consists of a single coordinate level; we track this so
   * that we can provide some helpful error messages for this case. If there is more
   * than one coordinate level, however, the error checking becomes too difficult,
   * because cells can be filled with universes, lattices, etc.
   */
  const bool _single_coord_level;

  /// Total number of OpenMC cells, across all coordinate levels
  const int _n_openmc_cells;

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
  static constexpr int MATERIAL_VOID {-1};

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

  /// Spatial dimension of the Monte Carlo problem
  static constexpr int DIMENSION {3};
};
