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

#include "OpenMCBase.h"

/**
 * A class which acts as an interface for tallies, allowing derived classes to
 * error-check the fetching of tally field variables.
 */
class TallyInterface : public OpenMCBase
{
public:
  static InputParameters validParams();

  TallyInterface(const ParallelParamObject * object, const InputParameters & parameters);

protected:
  /**
   * Get the score given a parameter name which points to a MooseEnum parameter. This checks
   * to ensure that the score is valid (at least one tally is accumulating it).
   * @param[in] score_param the name of the input parameter holding the score
   * @return the valid score
   */
  std::string getScore(const std::string & score_param);

  /**
   * Fetch a tally name by the score and the tally param in the input parameters. This
   * checks the tally to ensure it is valid for the given score. If the parameter
   * 'tally_param' is not valid and only one tally object accumulates 'score', this
   * function returns the name of that tally.
   * @param[in] score the score which the tally should be accumulating
   * @param[in] tally_param the parameter name in the input parameters for '_object' which contains
   * the name of a tally.
   * @return the name of the tally which can be used to fetch score variables
   */
  std::string tallyByScore(const std::string & score, const std::string & tally_param);

private:
  /// The object deriving from TallyInterface
  const ParallelParamObject * _object;
};
