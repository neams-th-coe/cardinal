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

#define LIBMESH

#include "CardinalProblem.h"
#include "PostprocessorInterface.h"
#include "CardinalEnums.h"
#include "OpenMCNuclideDensities.h"
#include "OpenMCTallyNuclides.h"

#include "mpi.h"
#include "openmc/bank.h"
#include "openmc/capi.h"
#include "openmc/cell.h"
#include "openmc/geometry.h"
#include "openmc/geometry_aux.h"
#include "openmc/hdf5_interface.h"
#include "openmc/material.h"
#include "openmc/mesh.h"
#include "openmc/settings.h"
#include "openmc/simulation.h"
#include "openmc/source.h"
#include "openmc/state_point.h"
#include "openmc/tallies/tally.h"
#include "openmc/tallies/filter_cell_instance.h"
#include "xtensor/xview.hpp"

/**
 * Base class for all MOOSE wrappings of OpenMC
 */
class OpenMCProblemBase : public CardinalProblem, public PostprocessorInterface
{
public:
  OpenMCProblemBase(const InputParameters & params);

  static InputParameters validParams();

  virtual ~OpenMCProblemBase() override;

  /**
   * Get the subdomain name for a given ID. If not named, we return the ID
   * @param[in] id subdomain ID
   * @return name
   */
  std::string subdomainName(const SubdomainID & id) const;

  /**
   * Print a full error message when catching errors from OpenMC
   * @param[in] err OpenMC error code
   * @param[in] descriptor descriptive message for error
   */
  void catchOpenMCError(const int & err, const std::string descriptor) const;

  /**
   * Whether the score is a heating-type score
   * @return whether tally from OpenMC has units of eV/src
   */
  bool isHeatingScore(const std::string & score) const;

  /**
   * Add a constant monomial auxiliary variable
   * @param[in] name name of the variable
   * @param[in] block optional subdomain names on which to restrict the variable
   * @return numeric index for the variable in the auxiliary system
   */
  unsigned int addExternalVariable(const std::string & name, const std::vector<SubdomainName> * block = nullptr);

  /**
   * Get the scaling value applied to the [Mesh] to convert to OpenMC's centimeters units
   * @return scaling value
   */
  const Real & scaling() const { return _scaling; }

  /**
   * Whether the problem has user defined scaling or not.
   * @return whether the user has set the problem scaling or not
   */
  bool hasScaling() const { return _specified_scaling; }

  /**
   * Convert from a MOOSE-type enum into a valid OpenMC tally score string
   * @param[in] score MOOSE-type enum string
   * @return OpenMC tally score string
   */
  std::string enumToTallyScore(const std::string & score) const;

  /**
   * Convert into a MOOSE-type enum from a valid OpenMC tally score string
   * @param[in] score OpenMC tally score string
   * @return MOOSE-type enum string
   */
  std::string tallyScoreToEnum(const std::string & score) const;

  /**
   * Find the geometry type in the OpenMC model
   * @param[out] has_csg_universe whether there is at least 1 CSG universe
   * @param[out] has_dag_universe whether there is at least 1 DagMC universe
   */
  virtual void geometryType(bool & has_csg_universe, bool & has_dag_universe) const;

  /// Whether this is the first time OpenMC is running
  bool firstSolve() const;

  /**
   * Convert from a MooseEnum for a trigger metric to an OpenMC enum
   * @param[in] trigger trigger metric
   * @return OpenMC enum
   */
  openmc::TriggerMetric triggerMetric(trigger::TallyTriggerTypeEnum trigger) const;
  openmc::TriggerMetric triggerMetric(std::string trigger) const;

  /**
   * Convert from a MooseEnum for tally estimator to an OpenMC enum
   * @param[in] estimator MOOSE estimator enum
   * @return OpenMC enum
   */
  openmc::TallyEstimator tallyEstimator(tally::TallyEstimatorEnum estimator) const;

  /**
   * Convert a tally estimator to a string (for output purposes).
   * @param[in] estimator OpenMC tally estimator enum
   * @return a string form of the OpenMC tally estimator enum
   */
  std::string estimatorToString(openmc::TallyEstimator estimator) const;

  /// Run a k-eigenvalue OpenMC simulation
  void externalSolve() override;

  /// Import temperature and density from a properties.h5 file
  void importProperties() const;

  /**
   * \brief Compute the sum of a tally within each bin
   *
   * For example, suppose we have a cell tally with 4 bins, one for each of 4
   * different cells. This function will return the sum of the tally in each of
   * those bins, so the return xtensor will have a length of 4, with each value
   * representing the sum for that bin.
   *
   * @param[in] tally OpenMC tally
   * @param[in] score tally score
   * @return tally sum within each bin
   */
  xt::xtensor<double, 1> tallySum(openmc::Tally * tally, const unsigned int & score) const;

  /**
   * Compute the sum of a tally across all of its bins
   * @param[in] tally OpenMC tallies (multiple if repeated mesh tallies)
   * @param[in] score tally score
   * @return tally sum
   */
  double tallySumAcrossBins(std::vector<openmc::Tally *> tally, const unsigned int & score) const;

  /**
   * Compute the mean of a tally across all of its bins
   * @param[in] tally OpenMC tallies (multiple if repeated mesh tallies)
   * @param[in] score tally score
   * @return tally mean
   */
  double tallyMeanAcrossBins(std::vector<openmc::Tally *> tally, const unsigned int & score) const;

  /**
   * Type definition for storing the relevant aspects of the OpenMC geometry; the first
   * value is the cell index, while the second is the cell instance.
   */
  typedef std::pair<int32_t, int32_t> cellInfo;

  /**
   * Whether a cell is filled with VOID (vacuum)
   * @param[in] cell_info cell ID, instance
   * @return whether cell is void
   */
  bool cellIsVoid(const cellInfo & cell_info) const;

  /**
   * Whether this cell has zero instances
   * @param[in] cell_info cell info
   * @return whether this cell has zero instances
   */
  bool cellHasZeroInstances(const cellInfo & cell_info) const;

  /**
   * Get the material name given its index. If the material does not have a name,
   * return the ID.
   * @param[in] index
   * @return material name
   */
  std::string materialName(const int32_t index) const;

  /**
   * Compute relative error
   * @param[in] sum sum of scores
   * @param[in] sum_sq sum of scores squared
   * @param[in] n_realizations number of realizations
   */
  xt::xtensor<double, 1> relativeError(const xt::xtensor<double, 1> & sum,
    const xt::xtensor<double, 1> & sum_sq, const int & n_realizations) const;

  /**
   * Get the density conversion factor (multiplicative factor)
   * @return density conversion factor from kg/m3 to g/cm3
   */
  const Real & densityConversionFactor() const { return _density_conversion_factor; }

  /**
   * Get the number of particles used in the current Monte Carlo calculation
   * @return number of particles
   */
  int nParticles() const;

  /**
   * Total number of particles run (not multiplied by batches)
   * @return total number of particles
   */
  int nTotalParticles() const { return _total_n_particles; }

  /**
   * Get the cell ID from the cell index
   * @param[in] index cell index
   * @return cell ID
   */
  int32_t cellID(const int32_t index) const;

  /**
   * Get the material ID from the material index; for VOID cells, this returns -1
   * @param[in] index material index
   * @return cell material ID
   */
  int32_t materialID(const int32_t index) const;

  /**
   * Print point coordinates with a neater formatting than the default MOOSE printing
   * @return formatted point string
   */
  std::string printPoint(const Point & p) const;

  /**
   * Get a descriptive, formatted, string describing a material
   * @param[in] index material index
   * @return descriptive string
   */
  std::string printMaterial(const int32_t & index) const;

  /**
   * Write the source bank to HDF5 for postprocessing or for use in subsequent solves
   * @param[in] filename file name
   */
  void writeSourceBank(const std::string & filename);

  /**
   * Get the total (i.e. summed across all ranks, if distributed)
   * number of elements in a given block
   * @param[in] id subdomainID
   * return number of elements in block
   */
  unsigned int numElemsInSubdomain(const SubdomainID & id) const;

  /**
   * Whether the element is owned by this rank
   * @return whether element is owned by this rank
   */
  bool isLocalElem(const Elem * elem) const;

  /**
   * Get the global element ID from the local element ID
   * @param[in] id local element ID
   * @return global element ID
   */
  unsigned int globalElemID(const unsigned int & id) const { return _local_to_global_elem[id]; }

  /**
   * Set the cell temperature, and print helpful error message if a failure occurs; this sets
   * the temperature for the id + instance, which could be one of N cells filling the 'cell_info'
   * parent cell (which is what we actually use for error printing)
   * @param[in] id cell ID
   * @param[in] instance cell instance
   * @param[in] T temperature
   * @param[in] cell_info cell info for which we are setting interior temperature, for error printing
   */
  virtual void setCellTemperature(const int32_t & id, const int32_t & instance, const Real & T,
    const cellInfo & cell_info) const;

  /**
   * Set the cell density, and print helpful error message if a failure occurs
   * @param[in] density density
   * @param[in] cell_info cell info for which we are setting the density
   */
  virtual void setCellDensity(const Real & density, const cellInfo & cell_info) const;

  /**
   * Get a descriptive, formatted, string describing a cell
   * @param[in] cell_info cell index, instance pair
   * @param[in] brief whether to print a shorter string
   * @return descriptive string describing cell
   */
  virtual std::string printCell(const cellInfo & cell_info, const bool brief = false) const;

  /**
   * Get the fill of an OpenMC cell
   * @param[in] cell_info cell ID, instance
   * @param[out] fill_type fill type of the cell, one of MATERIAL, UNIVERSE, or LATTICE
   * @return indices of what is filling the cell
   */
  virtual std::vector<int32_t> cellFill(const cellInfo & cell_info, int & fill_type) const;

  /**
   * Whether the cell has a material fill (if so, then get the material index). Void counts
   * as a material, with a material index of -1.
   * @param[in] cell_info cell ID, instance
   * @param[out] material_index material index in the cell
   * @return whether the cell is filled by a material
   */
  bool materialFill(const cellInfo & cell_info, int32_t & material_index) const;

  /**
   * Calculate the number of unique OpenMC cells (each with a unique ID & instance)
   * @return number of unique OpenMC Cells in entire model
   */
  long unsigned int numCells() const;

protected:
  /// Find all userobjects which are changing OpenMC data structures
  void getOpenMCUserObjects();

  /// Set the nuclide densities for any materials being modified via MOOSE
  void sendNuclideDensitiesToOpenMC();

  /// Set the tally nuclides for any tallies being modified via MOOSE
  void sendTallyNuclidesToOpenMC();

  /**
   * Set an auxiliary elemental variable to a specified value
   * @param[in] var_num variable number
   * @param[in] elem_ids element IDs to set
   * @param[in] value value to set
   */
  void fillElementalAuxVariable(const unsigned int & var_num,
                                const std::vector<unsigned int> & elem_ids,
                                const Real & value);

  /**
   * Get name of source bank file to write
   * @param[out] file name
   */
  std::string sourceBankFileName() const
  {
    return openmc::settings::path_output + "initial_source_" +
           std::to_string(_fixed_point_iteration) + ".h5";
  }

  /// Whether to print diagnostic information about model setup and the transfers
  const bool & _verbose;

  /// Power by which to normalize the OpenMC results, for k-eigenvalue mode
  const Real * _power;

  /// Source strength by which to normalize the OpenMC results, for fixed source mode
  const Real * _source_strength;

  /**
   * Whether to take the starting fission source from iteration \f$n\f$ as the
   * fission source converged from iteration \f$n-1\f$. Setting this to true will
   * in most cases lead to improved convergence of the initial source as iterations
   * progress because you don't "start from scratch" each iteration and do the same
   * identical (within a random number seed) converging of the fission source.
   */
  bool _reuse_source;

  /// Whether a mesh scaling was specified by the user
  const bool _specified_scaling;

  /**
   * Multiplicative factor to apply to the mesh in the [Mesh] block in order to convert
   * whatever units that mesh is in into OpenMC's length scale of centimeters. For instance,
   * it is commonplace to develop fuel performance models with a length scale of meters.
   * Rather than onerously convert all MOOSE inputs to which OpenMC will be coupled to units
   * of centimeters, this parameter allows us to scale the mesh we couple to OpenMC.
   * Note that this does not actually scale the mesh itself, but simply multiplies the mesh
   * coordinates by this parameter when identifying the mapping between elements and cells.
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

  /// Whether to skip writing statepoints from OpenMC
  const bool & _skip_statepoint;

  /**
   * Fixed point iteration index used in relaxation; because we sometimes run OpenMC
   * in a pseudo-transient coupling with NekRS, we simply increment this by 1 each
   * time we call openmc::run(). This uses a zero indexing, so after the first iteration,
   * we have finished iteration 0, and so on.
   */
  int _fixed_point_iteration;

  /// Total number of particles simulated
  unsigned int _total_n_particles;

  /// Total number of unique OpenMC cell IDs + instances combinations
  long unsigned int _n_openmc_cells;

  /**
   * Number of digits to use to display the cell ID for diagnostic messages; this is
   * estimated conservatively based on the total number of cells, even though there
   * may be distributed cells such that the maximum cell ID is far smaller than the
   * total number of cells.
   */
  int _n_cell_digits;

  /// OpenMC run mode
  openmc::RunMode _run_mode;

  /// Userobjects for changing OpenMC material compositions
  std::vector<OpenMCNuclideDensities *> _nuclide_densities_uos;

  /// Userobjects for changing OpenMC tally nuclides
  std::vector<OpenMCTallyNuclides *> _tally_nuclides_uos;

  /// Mapping from local element indices to global element indices for this rank
  std::vector<unsigned int> _local_to_global_elem;

  /// Conversion unit to transfer between kg/m3 and g/cm3
  static constexpr Real _density_conversion_factor{0.001};

  /// ID used by OpenMC to indicate that a material fill is VOID
  static constexpr int MATERIAL_VOID{-1};
};
