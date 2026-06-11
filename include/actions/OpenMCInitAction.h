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

#include "Action.h"

/**
 * Initialize OpenMC.
 */
class OpenMCInitAction : public Action
{
public:
  OpenMCInitAction(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void act() override;

protected:
  /**
   * Check whether an OpenMCCellAverageProblem have been requested in the input file and returns the
   * associated path to the XML directory if an OpenMCCellAverageProblem is present in the input
   * file.
   * @param[out] xml_directory directory in which OpenMC settings xml files are located
   * @return whether an OpenMCCellAverageProblem have been requested in the input file
   */
  bool isOpenMCCellAverageProblemRequested(std::string & xml_directory) const;

  /// Check if OpenMCMeshGenerator is present in the Mesh block and return xml_directory if it is the case
  bool get_openmc_mesh_generator_xml_directory(std::string & xml_directory) const;

  /**
   * Call the OpenMC initialization handle
   * @param[in] xml_directory directory in which OpenMC settings xml files are located
   */
  void initOpenMC(const std::string & xml_directory);
};
