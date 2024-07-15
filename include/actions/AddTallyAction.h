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

#include "MooseObjectAction.h"

/// This action adds tallies for use in simulations that couple OpenMC.
class AddTallyAction : public MooseObjectAction
{
public:
  static InputParameters validParams();

  AddTallyAction(const InputParameters & parameters);

  virtual void act() override;

protected:
  /**
   * This function adds a a tally which may be an offset into an array
   * of translations with index instance. Used for translated mesh tallies
   * that all use the same base mesh for the spatial filter.
   * @param instance the index of the tally into the translation array
   * @param translation the translation vector at instance
   */
  void addMeshTally(unsigned int instance = 0, const Point & translation = Point(0.0, 0.0, 0.0));

  /**
   * This function adds a a tally which may be an offset into an array
   * of translations with index instance. Used for translated mesh tallies
   * that all use the same base mesh for the spatial filter.
   */
  void addTally();

  /**
   * Fill the mesh translations to be applied to an unstructured mesh tally; if no
   * translations are explicitly given, a translation of (0.0, 0.0, 0.0) is assumed.
   */
  void fillMeshTranslations();

  /**
   * Read the mesh translations from file data
   * @param[in] data data from file
   */
  void readMeshTranslations(const std::vector<std::vector<double>> & data);

  /// The number of mesh translations.
  std::vector<Point> _mesh_translations;
};
