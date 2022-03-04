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

#include "ExternalProblem.h"

/**
 * Base class for all MOOSE wrappings of OpenMC
 */
class OpenMCProblemBase : public ExternalProblem
{
public:
  OpenMCProblemBase(const InputParameters & params);

  static InputParameters validParams();

  /// Run a k-eigenvalue OpenMC simulation
  void externalSolve() override;

  /**
   * Type definition for storing the relevant aspects of the OpenMC geometry; the first
   * value is the cell index, while the second is the cell instance.
   */
  typedef std::pair<int32_t, int32_t> cellInfo;

  /**
   * Get the number of particles used in the current Monte Carlo calculation
   * @return number of particles
   */
  const int64_t & nParticles() const;

  /**
   * Get the cell ID from the cell index
   * @param[in] index cell index
   * @return cell ID
   */
  int32_t cellID(const int32_t index) const;

  /**
   * Get the material ID from the material index
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
   * Get the path output
   * @return path output
   */
  std::string pathOutput() const { return _path_output; }

protected:
  /**
   * Set an auxiliary elemental variable to a specified value
   * @param[in] var_num variable number
   * @param[in] elem_ids element IDs to set
   * @param[in] value value to set
   */
  void fillElementalAuxVariable(const unsigned int & var_num,
    const std::vector<unsigned int> & elem_ids, const Real & value);

  /**
   * Get name of source bank file to write
   * @param[out] file name
   */
  std::string sourceBankFileName() const { return
    _path_output + "initial_source_" + std::to_string(_fixed_point_iteration) + ".h5"; }

  /// Power by which to normalize the OpenMC results
  const Real & _power;

  /// Whether to print diagnostic information about model setup and the transfers
  const bool & _verbose;

  /**
   * Whether to take the starting fission source from iteration \f$n\f$ as the
   * fission source converged from iteration \f$n-1\f$. Setting this to true will
   * in most cases lead to improved convergence of the initial source as iterations
   * progress because you don't "start from scratch" each iteration and do the same
   * identical (within a random number seed) converging of the fission source.
   */
  const bool & _reuse_source;

  /**
   * Whether the OpenMC model consists of a single coordinate level; this can
   * in some cases be used for more verbose error messages
   */
  const bool _single_coord_level;

  /// Total number of unique OpenMC cell IDs + instances combinations
  long unsigned int _n_openmc_cells;

  /**
   * Fixed point iteration index used in relaxation; because we sometimes run OpenMC
   * in a pseudo-transient coupling with NekRS, we simply increment this by 1 each
   * time we call openmc::run(). This uses a zero indexing, so after the first iteration,
   * we have finished iteration 0, and so on.
   */
  int _fixed_point_iteration;

  /// Directory where OpenMC output files are written
  std::string _path_output;
};
