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

#include "InputParameters.h"

/**
 * Checks whether the user has specified an input parameter that will go un-used, printing
 * an error message to help in the debugging process.
 * @param p input parameters object
 * @param name name of the input parameter
 * @param explanation short explanation of the reason why parameter is unused
 */
void checkUnusedParam(const InputParameters & p,
                      const std::string & name,
                      const std::string & explanation);

/**
 * Checks whether the user has specified an input parameter is set
 * @param p input parameters object
 * @param name name of the input parameter
 * @param explanation short explanation of the reason why parameter is required
 */
void checkRequiredParam(const InputParameters & p,
                        const std::string & name,
                        const std::string & explanation);

/**
 * Checks whether a set of parameters are all jointly provided or jointly not provided
 * @param p input parameters object
 * @param name name of the input parameter
 * @param explanation short explanation of the reason why parameter is required
 */
void checkJointParams(const InputParameters & p,
                      const std::vector<std::string> & name,
                      const std::string & explanation);
