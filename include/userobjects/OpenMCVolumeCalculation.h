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

#include "GeneralUserObject.h"

#include "OpenMCBase.h"
#include "openmc/volume_calc.h"

/**
 * Class that computes a volume calculation over the OpenMC model
 * based on information from the OpenMC wrapping (OpenMCCellAverageProblem).
 */
class OpenMCVolumeCalculation : public GeneralUserObject, public OpenMCBase
{
public:
  static InputParameters validParams();

  OpenMCVolumeCalculation(const InputParameters & parameters);

  virtual void initialize() {}
  virtual void finalize() {}
  virtual void execute() {}

  /// Initialize the volume calculation (not in initialize() because we want to control this from the Problem)
  virtual void initializeVolumeCalculation();

  /// Compute the cell volumes
  virtual void computeVolumes();

  /// Erase previously-added volume calculation
  virtual void resetVolumeCalculation();

  /**
   * Convert from a Point to a Position
   * @param[in] point point
   * @return position
   */
  openmc::Position position(const Point & pt) const;

  /**
   * Get the OpenMC cell volume
   * @param[in] index cell index
   * @param[out] vol stochastically-computed volume
   * @param[out] std_dev standard deviation
   */
  void cellVolume(const unsigned int & index, Real & vol, Real & std_dev) const;

protected:
  /// Number of stochastic samples for calculation
  const unsigned int & _n_samples;

  /// Trigger for deciding when to terminate the stochastic volume calculation
  const MooseEnum _trigger;

  /// Threshold for terminating the trigger
  Real _trigger_threshold;

  /// Length multiplier applied to [Mesh] to get into OpenMC centimeters
  Real _scaling;

  /// Lower left of the box within which to compute OpenMC volumes
  Point _lower_left;

  /// Upper right of the box within which to compute OpenMC volumes
  Point _upper_right;

  /// Volume calculation object
  std::unique_ptr<openmc::VolumeCalculation> _volume_calc;

  /// Results of the volume calculation
  std::vector<openmc::VolumeCalculation::Result> _results;

  /// Map from cell index to its volume calculation result
  std::map<int, int> _index_to_calc_index;

  /// Index of volume calculation we add in OpenMC
  unsigned int _calc_index;
};
