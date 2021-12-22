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
#include "openmc/tallies/filter_cell.h"
#include "openmc/tallies/filter_cell_instance.h"
#include "openmc/tallies/filter_mesh.h"
#include "openmc/mesh.h"
#include "openmc/tallies/tally.h"

/**
 * Base class for wrapping OpenMC as a MOOSE application
 */
class OpenMCProblemBase : public ExternalProblem
{
public:
  OpenMCProblemBase(const InputParameters & params);
  static InputParameters validParams();

  virtual ~OpenMCProblemBase() override;

  virtual bool converged() override { return true; }

  /**
   * Type definition for storing the relevant aspects of the OpenMC geometry; the first
   * value is the cell index, while the second is the cell instance.
   */
  typedef std::pair<int32_t, int32_t> cellInfo;

  /**
   * Get the number of elements (in a distributed-mesh-friendly implementation)
   * that exist on a specified set of blocks
   * @param[in] blocks blocks to count elements for
   * @return number of elements
   */
  int elemsInBlock(const std::unordered_set<SubdomainID> & blocks) const;

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
   * Compute the number of digits required to display an integer
   * @param[in] number number to display
   */
  int digits(const int & number) const;

  /**
   * Print point coordinates with a neater formatting than the default libMesh
   * point printing
   * @return formatted point
   */
  std::string printPoint(const Point & p) const;

  /**
   * Get a descriptive, formatted, string describing a material
   * @param[in] index material index
   * @return descriptive string
   */
  std::string printMaterial(const int32_t & index) const;

  /**
   * Compute the mean value of a tally
   * @param[in] tally OpenMC tallies (multiple if repeated mesh tallies)
   * @return mean value
   */
  double tallySum(std::vector<openmc::Tally *> tally) const;

  /**
   * Compute relative error
   * @param[in] sum sum of scores
   * @param[in] sum_sq sum of scores squared
   * @param[in] n_realizations number of realizations
   */
  Real relativeError(const Real & sum, const Real & sum_sq, const int & n_realizations) const;

protected:
  /**
   * Get the cell ID, instance combination at the _particle's position and level
   * @param[in] level coordinate level
   * @return cell ID, instance combination
   */
  cellInfo particleCell(const int & level) const;

  /**
   * Set an auxiliary elemental variable to a specified value
   * @param[in] var_num variable number
   * @param[in] elem_ids element IDs to set
   * @param[in] value value to set
   */
  void fillElementalAuxVariable(const unsigned int & var_num, const std::vector<unsigned int> & elem_ids, const Real & value);

  /**
   * Set the number of particles to run for a Monte Carlo calculation
   * @param[in] n number of particles
   */
  void setParticles(const int64_t & n) const;

  /**
   * Get the number of particles used in the current Monte Carlo calculation
   * @return number of particles
   */
  const int64_t & nParticles() const;

  std::unique_ptr<NumericVector<Number>> _serialized_solution;

  /// Moose mesh
  MeshBase & _mesh_base;

  /// Power to normalize the OpenMC tallies by
  const Real & _power;

  /// Whether to print diagnostic information about model setup and the transfers
  const bool & _verbose;

  /**
   * Whether the OpenMC model consists of a single coordinate level; we track this so
   * that we can provide some helpful error messages for this case.
   */
  const bool _single_coord_level;

  /**
   * Number of digits to use to display the cell ID for diagnostic messages; this is
   * estimated conservatively based on the total number of cells, even though there
   * may be distributed cells such that the maximum cell ID is far smaller than the
   * total number of cells.
   */
  const int _n_cell_digits;

  /// Total number of OpenMC cells, across all coordinate levels
  long unsigned int _n_openmc_cells;

  /// Dummy particle to reduce number of allocations of particles for cell lookup routines
  openmc::Particle _particle;
};
