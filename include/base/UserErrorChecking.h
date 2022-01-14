#pragma once

#include "InputParameters.h"

/**
 * Checks whether the user has specified an input parameter that will go un-used, printing
 * an error message to help in the debugging process.
 * @param p input parameters object
 * @param name name of the input parameter
 * @param explanation short explanation of the reason why parameter is unused
 */
void
checkUnusedParam(const InputParameters & p, const std::string & name, const std::string & explanation);

/**
 * Checks whether the user has specified an input parameter is set
 * @param p input parameters object
 * @param name name of the input parameter
 * @param explanation short explanation of the reason why parameter is required
 */
void
checkRequiredParam(const InputParameters & p, const std::string & name, const std::string & explanation);
