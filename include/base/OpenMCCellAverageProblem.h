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

/// Tally includes.
#include "TallyBase.h"

#include "openmc/tallies/filter_mesh.h"

#ifdef ENABLE_DAGMC
#include "MoabSkinner.h"
#include "DagMC.hpp"
#endif

/**
 * Mapping of OpenMC to a collection of MOOSE elements, with temperature and/or
 * density feedback. The mappind is established authomatically by looping over
 * all the MOOSE elements and finding the OpenMC cell at each element's centroid.
 *
 * All feedback into OpenMC is performed via element averages. The
 * 'temperature_blocks' parameter is used to indicate which MOOSE blocks should
 * provide temperature feedback, while the 'density_blocks' parameter is used to
 * indicate which MOOSE blocks should provide density feedback. Tallies are
 * automatically added to OpenMC using either cell or mesh tallies.
 *
 * Each OpenMC cell shall not have ambiguous data transfers. That is, a cell
 * should map to a set of elements that are ALL/NONE providing temperature
 * feedback, ALL/NONE providing density feedback, ALL/NONE providing cell
 * tallies, and ALL/NONE being uncoupled altogether.
 *
 *  TODO: If this is too restrictive in the future, we could implement some type
 *        of weighted averaging process. Also, if a cell maps to a phase and an
 *        unmapped region, perhaps we want to allow that.
 *
 * Other considerations you should be aware of:
 *  - The density being transferred into OpenMC from MOOSE is in units of kg/m3; this
 *    is the unit employed by the MOOSE fluid properties module.
 *  - The temperature being transferred into OpenMC from MOOSE is in units of K; this
 *    is the unit employed by the MOOSE fluid and solid properties modules.
 *  - If your geometry is highly fine-scale (such as TRISOs), you might be able to get a
 *    huge speedup in your runtime if you (i) build your OpenMC model by repeating the same
 *    TRISO universe in each of your repeatable-units (e.g. pebbles, compacts, plates)
 *    AND (ii) leverage the 'identical_cell_fills' option.
 */
class OpenMCCellAverageProblem : public OpenMCProblemBase
{
public:
  OpenMCCellAverageProblem(const InputParameters & params);
  static InputParameters validParams();

  virtual void initialSetup() override;
  virtual void externalSolve() override;
  virtual void syncSolutions(ExternalProblem::Direction direction) override;
  virtual bool converged(unsigned int) override { return true; }

  /**
   * Read a 2d vector of subdomain names, and check that there are no duplications
   * and that all provided values exist on the mesh.
   * @param[in] name string name for the 2d vector parameter
   * @param[out] names subdomain names
   * @param[out] flattened_ids flattened 1d vector of subdomain IDs
   */
  void read2DBlockParameters(const std::string name,
                             std::vector<std::vector<SubdomainName>> & names,
                             std::vector<SubdomainID> & flattened_ids);

  /**
   * Check that the specified blocks are in the mesh
   * @param[in] name name for throwing an error
   * @param[in] ids block IDs to check
   * @param[in] names block subdomain names for throwing an error
   */
  void checkBlocksInMesh(const std::string name,
                         const std::vector<SubdomainID> & ids,
                         const std::vector<SubdomainName> & names) const;

  /// Initialize the mapping of OpenMC to the MooseMesh and perform additional setup actions
  void setupProblem();

  /**
   * Add the tally variable(s) (to receive OpenMC tally values), temperature variable(s)
   * (to write into OpenMC cells), and density variable(s) (to write into OpenMC materials)
   */
  virtual void addExternalVariables() override;

  /**
   * Get the cell volume from a stochastic calculation
   * @param[in] cell_info cell index, instance pair
   * @return stochastically-computed OpenMC cell volume
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
  virtual const std::map<cellInfo, std::vector<unsigned int>> & cellToElem() const
  {
    return _cell_to_elem;
  }

  /**
   * Get the MOOSE subdomains associated with an OpenMC cell
   * @param info the cell info
   * @return MOOSE subdomains associated with an OpenMC cell
   */
  virtual std::unordered_set<SubdomainID> getCellToElementSub(const cellInfo & info)
  {
    return _cell_to_elem_subdomain.at(info);
  }

  /**
   * Whether transformations are applied to the [Mesh] points when mapping to OpenMC
   * @return whether transformations are applied
   */
  virtual bool hasPointTransformations() const { return _symmetry != nullptr; }

  /**
   * Get all the scores added to the tally
   * @return scores
   */
  virtual const std::vector<std::string> & getTallyScores() const { return _all_tally_scores; }

  /**
   * Apply transformations to point
   * @param[in] pt point
   * @return transformed point
   */
  virtual Point transformPoint(const Point & pt) const
  {
    return this->hasPointTransformations() ? _symmetry->transformPoint(pt) : pt;
  }

  /**
   * This class uses elem->volume() in order to normalize the tally values. However,
   * elem->volume() is expensive, so whenever MOOSE does integration, they set
   *  _current_elem_volume to the volume as set by the sum of the quadrature weights.
   * The quadrature rule that MOOSE provides when you only have CONSTANT MONOMIALS is
   * insufficient for exactly integrating the element Jacobian mapping type (which
   * is FIRST LAGRANGE for a first order element), so you get an error relative to
   * the libmesh volume computation.
   *
   * So, we need to make sure that a minimum order quadrature rule is used
   * so that the total tally as computed by an
   * ElementIntegralVariablePostprocessor actually matches the specified total
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
   * Get the fields coupled for each cell; because we require that each cell maps to a consistent
   * set, we simply look up the coupled fields of the first element that this cell maps to. Note
   * that this function requires a valid instance, index pair for cellInfo - you cannot pass in an
   * unmapped cell, i.e. (UNMAPPED, UNMAPPED)
   * @param[in] cell_info cell index, instance pair
   * @return coupling fields
   */
  coupling::CouplingFields cellFeedback(const cellInfo & cell_info) const;

  /**
   * Whether a cell has density feedback
   * @param[in] cell_info cell index,instance pair
   * @return if cell has density feedback
   */
  bool hasDensityFeedback(const cellInfo & cell_info) const
  {
    std::vector<coupling::CouplingFields> phase = {coupling::density,
                                                   coupling::density_and_temperature};
    return std::find(phase.begin(), phase.end(), cellFeedback(cell_info)) != phase.end();
  }

  /**
   * Whether a cell has temperature feedback
   * @param[in] cell_info cell index,instance pair
   * @return if cell has temperature feedback
   */
  bool hasTemperatureFeedback(const cellInfo & cell_info) const
  {
    std::vector<coupling::CouplingFields> phase = {coupling::temperature,
                                                   coupling::density_and_temperature};
    return std::find(phase.begin(), phase.end(), cellFeedback(cell_info)) != phase.end();
  }

  /**
   * Get the local tally
   * @return local tally
   */
  const std::vector<std::shared_ptr<TallyBase>> & getLocalTally() const { return _local_tallies; }

  /**
   * Get the temperature of a cell; for cells not filled with materials, this will return
   * the temperature of the first material-type cell
   * @param[in] cell_info cell index, instance pair
   */
  double cellTemperature(const cellInfo & cell_info) const;

  /**
   * Get the volume that each OpenMC cell mapped to
   * @param[in] cell_info cell index, instance pair
   */
  double cellMappedVolume(const cellInfo & cell_info) const;

  /// Reconstruct the DAGMC geometry after skinning
  void reloadDAGMC();

  /**
   * Add a Tally object using the new tally system. TODO: rename to addTally once
   * OpenMCCellAverageProblem and OpenMCProblemBase are refactored.
   * @param[in] type the new tally type
   * @param[in] name the name of the new tally
   * @param[in] moose_object_pars the input parameters of the new tally
   */
  void addTallyObject(const std::string & type,
                      const std::string & name,
                      InputParameters & moose_object_pars);

  /**
   * Multiplier on the normalized tally results; for fixed source runs,
   * we multiply the tally (which has units of eV/source)
   * by the source strength and the eV to joule conversion, while for k-eigenvalue runs, we
   * multiply the normalized tally (which is unitless and has an integral
   * value of 1.0) by the power.
   * @param[in] global_score tally score
   */
  Real tallyMultiplier(unsigned int global_score) const;

  int fixedPointIteration() const { return _fixed_point_iteration; }

  /// Constant flag to indicate that a cell/element was unmapped
  static constexpr int32_t UNMAPPED{-1};

  /// Spatial dimension of the Monte Carlo problem
  static constexpr int DIMENSION{3};

protected:
  /**
   * Get the cell level in OpenMC to use for coupling
   * @param[in] c point
   * @return cell level
   */
  unsigned int getCellLevel(const Point & c) const;

  /**
   * Read the names of the MOOSE variables used for sending feedback into OpenMC
   * @param[in] param feedback term to read
   * @param[in] default_name default name to use for MOOSE variables holding this field
   * @param[out] vars_to_specified_blocks map from MOOSE variable names to the blocks on which they
   * are defined
   * @param[out] specified_blocks user-specified blocks for feedback
   */
  void
  readBlockVariables(const std::string & param,
                     const std::string & default_name,
                     std::map<std::string, std::vector<SubdomainName>> & vars_to_specified_blocks,
                     std::vector<SubdomainID> & specified_blocks);

  /**
   * Whether this cell has an identical fill
   * @param[in] cell_info cell index, instance pair
   * @return whether this cell has an identical fill
   */
  bool cellHasIdenticalFill(const cellInfo & cell_info) const;

  /**
   * When using the 'identical_cell_fills' feature, this is used to determine the
   * contained material cells in each parent cell by applying a uniform shift
   * @param[in] cell_info cell index, instance pair
   * @return material cells contained within the given cell
   */
  containedCells shiftCellInstances(const cellInfo & cell_info) const;

  /**
   * Whether this cell overlaps with ANY value in the given subdomain set
   * @param[in] cell_info cell index, instance pair
   * @param[in] id subdomain IDs
   * @return whether the cell overlaps with the subdomain
   */
  bool cellMapsToSubdomain(const cellInfo & cell_info,
                           const std::unordered_set<SubdomainID> & id) const;

  /**
   * Get the first material cell contained in the given cell
   * @param[in] cell_info cell index, instance pair
   * @return material cell index, instance pair
   */
  cellInfo firstContainedMaterialCell(const cellInfo & cell_info) const;

  /**
   * Get all of the material cells contained within this cell
   * @param[in] cell_info cell index, instance pair
   * @return all material cells contained in the given cell
   */
  containedCells containedMaterialCells(const cellInfo & cell_info) const;

  /**
   * Re-generate the OpenMC materials in-place, needed for skinning operation where
   * we create new OpenMC materials on-the-fly in order to receive density feedback.
   */
  virtual void updateMaterials();

  /**
   * Get a list of each material in the problem, sorted by subdomain. This function also checks
   * that there is just one OpenMC material in each subdomain, necessary for the DAGMC skinning.
   * @return material in each subdomain
   */
  std::vector<std::string> getMaterialInEachSubdomain() const;

  /**
   * Apply transformations and scale point from MOOSE into the OpenMC domain
   * @param[in] pt point
   * @return transformed point
   */
  Point transformPointToOpenMC(const Point & pt) const;

  /**
   * Check that the tally normalization gives a total tally sum of 1.0 (when normalized
   * against the total tally value).
   * @param[in] sum sum of the tally
   * @param[in] score tally score
   */
  void checkNormalization(const Real & sum, unsigned int global_score) const;

  /**
   * For geometries with fine-scale details (e.g. TRISO), Cardinal's default settings can
   * take a very long time to initialize the problem (but we can't change those defaults
   * because they are not 100% applicable all the time). So, we print out a message to
   * the user to point them in the right direction if their initialization is taking a
   * long time.
   * @param[in] start time to use for evaluating whether we've exceeded our limit for printing the
   * message
   */
  void
  printTrisoHelp(const std::chrono::time_point<std::chrono::high_resolution_clock> & start) const;

  /**
   * Print to the console the names of the auxvariables used for I/O with OpenMC.
   * We only print these tables once, upon initialization, because this data does
   * not change if re-initializing the spatial mapping for moving-mesh problems,
   * adaptive refinement, skinning, etc.
   */
  void printAuxVariableIO();

  /**
   * Get all the material indices within the set of cells
   * @param[in] contained_cells set of cells
   * @return contained materials
   */
  std::vector<int32_t> materialsInCells(const containedCells & contained_cells) const;

  /// Loop over the mapped cells, and build a map between subdomains to OpenMC materials
  void subdomainsToMaterials();

  /**
   * Get a set of all subdomains that have at least 1 element coupled to an OpenMC cell
   * @return subdomains with at least 1 element coupled to OpenMC
   */
  std::set<SubdomainID> coupledSubdomains() const;

  /**
   * Gather a vector of values to be summed for each cell
   * @param[in] local local values to be summed for the cells
   * @param[out] global global mapping of the summed values to the cells
   */
  template <typename T>
  void gatherCellSum(std::vector<T> & local, std::map<cellInfo, T> & global) const;

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
   * Read the block parameters based on user settings
   * @param[in] name name of input parameter representing a vector of subdomain names
   * @param[in] blocks list of block ids to write
   */
  void readBlockParameters(const std::string name, std::unordered_set<SubdomainID> & blocks);

  /**
   * Cache the material cells contained within each coupling cell;
   * depending on user settings, this may attempt to take shortcuts
   * by assuming each cell has the same fills
   */
  void cacheContainedCells();

  /**
   * Fill the cached contained cells data structure for a given cell
   * @param[in] cell_info cell index, instance pair
   * @param[in] hint location hint used to accelerate the search
   * @param[out] map contained cell map
   */
  void setContainedCells(const cellInfo & cell_info,
                         const Point & hint,
                         std::map<cellInfo, containedCells> & map);

  /**
   * Check that the structure of the contained material cells for two cell matches;
   * i.e. this checks that the keys are the same and that the *number* of instances
   * of each filling material cell match.
   * @param[in] cell_info cell index, instance pair
   * @param[in] reference map we want to check against
   * @param[in] compare map we want to check
   */
  void checkContainedCellsStructure(const cellInfo & cell_info,
                                    containedCells & reference,
                                    containedCells & compare) const;

  /**
   * Set a minimum order for a volume quadrature rule
   * @param[in] volume_order order of the volume quadrature rule
   * @param[in] type string type of quadrature rule for printing a console message
   */
  void setMinimumVolumeQRules(Order & volume_order, const std::string & type);

  /// For keeping the output neat when using verbose
  std::string printNewline() const
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

  /// Loop over the elements in the MOOSE mesh and store the type of feedback applied by each.
  void storeElementPhase();

  /**
   * Relax the tally and normalize it according to some "global" tally. If you set
   * 'normalize_by_global_tally = true', you will be normalizing by a tally over the ENTIRE
   * OpenMC geometry. Otherwise, you normalize only by the sum of the tally bins themselves.
   *
   * NOTE: This function relaxes the tally _distribution_, and not the actual magnitude of the sum.
   * That is, we relax the shape distribution and then multiply it by the power
   * (for k-eigenvalue) or source strength (for fixed source) of the current step before
   * applying it to MOOSE. If the magnitude of the power or source strength is constant in time,
   * there is zero error in this. But if the magnitude of the tally varies in time, we are basically
   * relaxing the distribution of the tally, but then multiplying it by the _current_ mean tally
   * magnitude.
   *
   * There will be very small errors in these approximations unless the power/source strength
   * change dramatically with iteration. But because relaxation is itself a numerical approximation,
   * this is still inconsequential at the end of the day as long as your problem has converged
   * the relaxed tally to the raw (unrelaxed) tally.
   * @param[in] global_score the global index of the tally score
   * @param[in] local_score the local index of the tally score
   * @param[in] local_tally the tally to relax and normalize
   */
  void relaxAndNormalizeTally(unsigned int global_score,
                              unsigned int local_score,
                              std::shared_ptr<TallyBase> local_tally);

  /**
   * Loop over all the OpenMC cells and count the number of MOOSE elements to which the cell
   * is mapped based on phase.
   */
  void getCellMappedPhase();

  /// This function is used to ensure that each OpenMC cell only maps to a single phase
  void checkCellMappedPhase();

  /// Loop over all the OpenMC cells and get the element subdomain IDs that map to each cell
  void getCellMappedSubdomains();

  /**
   * Loop over all the OpenMC cells and compute the volume of the MOOSE elements that each
   * cell maps to
   */
  void computeCellMappedVolumes();

  /// Set up the mapping from MOOSE elements to OpenMC cells
  void initializeElementToCellMapping();

  /// Populate maps of MOOSE elements to OpenMC cells
  void mapElemsToCells();

  /**
   * A function which validates local tallies. This is done to ensure that at least one of the
   * tallies contains a heating score when running in eigenvalue mode. This must be done outside
   * of the constructor as tallies are added from an external system.
   */
  void validateLocalTallies();

  /// Add OpenMC tallies to facilitate the coupling
  void initializeTallies();

  /**
   * Reset any tallies previously added by Cardinal, by deleting them from OpenMC.
   * Also delete any mesh filters and meshes added to OpenMC for mesh filters.
   */
  void resetTallies();

  /// Find the material filling each cell which receives density feedback
  void getMaterialFills();

  /**
   * Get one point inside each cell, for accelerating the particle search routine.
   * This function will get the centroid of the first global element in the lowest
   * rank in the cell.
   */
  void getPointInCell();

  /**
   * Compute the product of volume with a field across ranks and sum into a global map
   * @param[in] var_num variable to weight with volume, mapped by subdomain ID
   * @param[in] phase phases to compute the operation for
   * @return volume-weighted field for each cell, in a global sense
   */
  std::map<cellInfo, Real> computeVolumeWeightedCellInput(
      const std::map<SubdomainID, std::pair<unsigned int, std::string>> & var_num,
      const std::vector<coupling::CouplingFields> * phase) const;

  /**
   * Send temperature from MOOSE to OpenMC by computing a volume average
   * and applying a single temperature per OpenMC cell
   */
  void sendTemperatureToOpenMC() const;

  /**
   * Send density from MOOSE to OpenMC by computing a volume average
   * and applying a single density per OpenMC cell.
   */
  void sendDensityToOpenMC() const;

  /**
   * Factor by which to normalize a tally
   * @param[in] global_score global index for the tally score
   * @return value to divide tally sum by for normalization
   */
  Real tallyNormalization(unsigned int global_score) const;

  /**
   * Check the sum of the tallies against the global tally
   * @param[in] score tally score
   */
  void checkTallySum(const unsigned int & score) const;

  /**
   * Check if a mapped location is in the outer universe of a lattice
   * @param[in] level lattice level
   * @return whether the location is in the outer universe
   */
  void latticeOuterCheck(const Point & c, int level) const;

  /**
   * Report an error for a mapped location in an outer universe of a lattice
   * @param[in] c Mapped location
   * @param[in] level level of the mapped cell
   */
  void latticeOuterError(const Point & c, int level) const;

  /**
   * Find the OpenMC cell at a given point in space
   * @param[in] point point
   * @return whether OpenMC reported an error
   */
  bool findCell(const Point & point);

  /**
   * Checks that the contained material cells exactly match between a reference obtained
   * by calling openmc::Cell::get_contained_cells for each cell and a shortcut
   * approach that assumes all identical cells (which aren't simply just material fills)
   * has exactly the same contained material cells.
   * @param[in] reference reference map to compare against
   * @param[in] compare shortcut map to compare
   */
  void compareContainedCells(std::map<cellInfo, containedCells> & reference,
                             std::map<cellInfo, containedCells> & compare) const;

  std::unique_ptr<NumericVector<Number>> _serialized_solution;

  /**
   * Whether to automatically compute the mapping of OpenMC cell IDs and
   * instances to the [Mesh].
   */
  const bool & _output_cell_mapping;

  /**
   * Where to get the initial OpenMC temperatures and densities from;
   * can be either hdf5 (from a properties.h5 file), xml (whatever is already
   * set in the XML files), or moose (meaning whatever ICs are set on the 'temperature_variables'
   * and 'density_variables'
   */
  const coupling::OpenMCInitialCondition _initial_condition;

  /// Type of relaxation to apply to the OpenMC tallies
  const relaxation::RelaxationEnum _relaxation;

  /**
   * Type of trigger to apply to k eigenvalue to indicate when
   * the simulation is complete. These can be used to on-the-fly adjust the number
   * of active batches in order to reach some desired criteria (which is specified
   * by this parameter).
   */
  const trigger::TallyTriggerTypeEnum _k_trigger;

  /**
   * Coordinate level in the OpenMC domain to use for mapping cells to mesh.
   * When using 'lowest_cell_level', this parameter indicates that the lowest
   * cell level is used, up until _cell_level.
   */
  unsigned int _cell_level;

  /**
   * Whether OpenMC properties (temperature and density) should be exported
   * after being updated in syncSolutions.
   */
  const bool & _export_properties;

  /**
   * How to normalize the OpenMC tally into units of W/volume. If 'true',
   * normalization is performed by dividing each local tally against a problem-global
   * tally. The advantage of this approach is that some non-zero tally regions of the
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
   * When using this parameter, we HIGHLY recommend setting 'check_identical_cell_fills =
   * true' the first time you run your model. This will figure out the material cell fills using a
   * method that calls openmc::Cell::get_contained_cells for every tally cell, i.e. without assuming
   * anything about repeated structure in your OpenMC model. Setting 'identical_cell_fills'
   * without also setting 'check_identical_cell_fills = true' may result in SILENT
   * errors!!! So it is essential to be sure you've removed any error sources before you turn the
   * error check off to actually leverage the speedup.
   *
   * Note: for any tally cells that are just filled with a material, we use the approach
   * where openmc::Cell::get_contained_cells is called in full.
   *
   * This optimization will not work (and 'check_identical_cell_fills = true' *will*
   * catch these) for:
   * - any situation where tallied, non-material-fill pebbles have different fills
   *   (such as if you have different TRISO lattices in each pebble)
   * - any situation where there is a "gap" in the incrementing of the material fill
   *   instances (such as if pebble 89 does not map to 'tally_blocks', then the instance
   *   shift for pebble 90 relative to pebble 1 is 89, when it should have been 90).
   */
  const bool _has_identical_cell_fills;

  /**
   * Whether we should rigorously check that each tally cell has identical fills;
   * this is SLOW for large TRISO problems, but is essential to ensure the accuracy of
   * 'identical_cell_fills'. Please set 'check_identical_cell_fills' to 'true' at least
   * once before running production cases to be sure the optimization can be applied.
   */
  const bool & _check_identical_cell_fills;

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
   * Whether the problem has density feedback blocks specified; note that this is NOT necessarily
   * indicative that the mapping was successful in finding any cells corresponding to those blocks
   */
  const bool _specified_density_feedback;

  /**
   * Whether the problem has temperature feedback blocks specified; note that this is NOT
   * necessarily indicative that the mapping was successful in finding any cells corresponding to
   * those blocks
   */
  const bool _specified_temperature_feedback;

  /// Whether any cell tallies exist.
  bool _has_cell_tallies = false;

  /// Whether any spatial mapping from OpenMC's cells to the mesh is needed
  bool _needs_to_map_cells;

  /**
   * Whether a global tally is required for the sake of normalization and/or checking
   * the tally sum
   */
  const bool _needs_global_tally;

  /// A vector of the tally objects created by the [Tallies] block.
  std::vector<std::shared_ptr<TallyBase>> _local_tallies;

  /// A list of all of the scores contained by the local tallies added in the [Tallies] block.
  std::vector<std::string> _all_tally_scores;

  /**
   * The [Tallies] block allows tallies with different scores, and so we can't assume they have the
   * same indices in each tally's arrays. This variable map between the name of each score and it's
   * index in each local tally.
   */
  std::vector<std::map<std::string, int>> _local_tally_score_map;

  /// A vector of auxvariable ids added by the [Tallies] block.
  std::vector<std::vector<unsigned int>> _tally_var_ids;

  /// A vector of external (output-based) auxvariable ids added by the [Tallies] block.
  std::vector<std::vector<std::vector<unsigned int>>> _tally_ext_var_ids;

  /// Whether the problem contains a cell tally or not.
  bool _contains_cell_tally = false;

  /// Blocks in MOOSE mesh that provide density feedback
  std::vector<SubdomainID> _density_blocks;

  /// Blocks in MOOSE mesh that provide temperature feedback
  std::vector<SubdomainID> _temp_blocks;

  /// Blocks for which the cell fills are identical
  std::unordered_set<SubdomainID> _identical_cell_fill_blocks;

  /// Mapping of MOOSE elements to the OpenMC cell they map to (if any)
  std::vector<cellInfo> _elem_to_cell{};

  /// Phase of each cell
  std::map<cellInfo, coupling::CouplingFields> _cell_phase;

  /// Number of elements in the MOOSE mesh that exclusively provide density feedback
  int _n_moose_density_elems;

  /// Number of elements in the MOOSE mesh that exclusively provide temperature feedback
  int _n_moose_temp_elems;

  /// Number of elements in the MOOSE mesh which provide temperature+density feedback
  int _n_moose_temp_density_elems;

  /// Number of no-coupling elements in the MOOSE mesh
  int _n_moose_none_elems;

  /**
   * Number of MOOSE elements that exclusively provide temperature feedback,
   * and which successfully mapped to OpenMC cells
   */
  int _n_mapped_temp_elems;

  /**
   * Number of MOOSE elements that exclusively provide density feedback,
   * and which successfully mapped to OpenMC cells
   */
  int _n_mapped_density_elems;

  /**
   * Number of MOOSE elements that provide temperature+density feedback,
   * and which successfully mapped to OpenMC cells
   */
  int _n_mapped_temp_density_elems;

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
   * Material filling each cell to receive density feedback. We enforce that these
   * cells are filled with a material (cannot be filled with a lattice or universe).
   */
  std::map<cellInfo, int32_t> _cell_to_material;

  /**
   * Material-type cells contained within a cell; this is only populated if a cell
   * is NOT indicated as having an identical fill
   */
  std::map<cellInfo, containedCells> _cell_to_contained_material_cells;

  /// Number of material-type cells contained within a cell
  std::map<cellInfo, int32_t> _cell_to_n_contained;

  /**
   * Global tallies. We add one per tally added in the [Tallies] block to
   * enable global noramlization.
   */
  std::vector<openmc::Tally *> _global_tallies;

  /// Global tally scores corresponding to '_global_tallies'.
  std::vector<std::vector<std::string>> _global_tally_scores;

  /// Global tally estimators corresponding to '_global_tallies'.
  std::vector<openmc::TallyEstimator> _global_tally_estimators;

  /// Sum value of the global tally(s), across all bins
  std::vector<Real> _global_sum_tally;

  /// Sum value of the local tally(s), across all bins
  std::vector<Real> _local_sum_tally;

  /// Mean value of the local tally(s), across all bins; only used for fixed source mode
  std::vector<Real> _local_mean_tally;

  /// Whether the present transfer is the first transfer
  static bool _first_transfer;

  /// Whether the diagnostic tables on initialization have already been printed
  static bool _printed_initial;

  /// Whether a warning has already been printed about very long setup times (for TRISOs)
  static bool _printed_triso_warning;

  /// Dummy particle to reduce number of allocations of particles for cell lookup routines
  openmc::Particle _particle;

  /// Number of particles simulated in the first iteration
  unsigned int _n_particles_1;

  /// Mapping from temperature variable name to the subdomains on which to read it from
  std::map<std::string, std::vector<SubdomainName>> _temp_vars_to_blocks;

  /// Mapping from density variable name to the subdomains on which to read it from
  std::map<std::string, std::vector<SubdomainName>> _density_vars_to_blocks;

  /// Optional volume calculation for cells which map to MOOSE
  OpenMCVolumeCalculation * _volume_calc;

  /// Userobject that maps from a partial-symmetry OpenMC model to a whole-domain [Mesh]
  const SymmetryPointGenerator * _symmetry;

  /// Number of temperature-only feedback elements in each mapped OpenMC cell (global)
  std::map<cellInfo, int> _n_temp;

  /// Number of density-only feedback elements in each mapped OpenMC cell (global)
  std::map<cellInfo, int> _n_rho;

  /// Number of temperature+density feedback elements in each mapped OpenMC cell (global)
  std::map<cellInfo, int> _n_temp_rho;

  /// Number of none elements in each mapped OpenMC cell (global)
  std::map<cellInfo, int> _n_none;

  /// Index in OpenMC tallies corresponding to the first global tally added by Cardinal
  unsigned int _global_tally_index = 0;

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

  /// Offsets for each cell instance in an identically-repeated universe
  containedCells _instance_offsets;

  /// Offset for each cell relative to the first identical-fill cell
  std::map<cellInfo, int32_t> _n_offset;

  /// First identical-fill cell
  cellInfo _first_identical_cell;

  /// Materials in the first identical-fill cell
  std::vector<int32_t> _first_identical_cell_materials;

  /// Mapping from subdomain IDs to which aux variable to read temperature (K) from
  std::map<SubdomainID, std::pair<unsigned int, std::string>> _subdomain_to_temp_vars;

  /// Mapping from subdomain IDs to which aux variable to read density (kg/m3) from
  std::map<SubdomainID, std::pair<unsigned int, std::string>> _subdomain_to_density_vars;
};
