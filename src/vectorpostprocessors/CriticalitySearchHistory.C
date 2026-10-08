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

#ifdef ENABLE_OPENMC_COUPLING

#include "CriticalitySearchHistory.h"
#include "CriticalitySearchBase.h"
#include "OpenMCCellAverageProblem.h"

registerMooseObject("CardinalApp", CriticalitySearchHistory);

InputParameters
CriticalitySearchHistory::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params += OpenMCBase::validParams();
  params.addClassDescription("Values tried in the most recent criticality search and the k "
                             "eigenvalue and reactivity computed by OpenMC for each");
  return params;
}

CriticalitySearchHistory::CriticalitySearchHistory(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    OpenMCBase(this, parameters),
    _input(declareVector("input")),
    _k(declareVector("k")),
    _k_std_dev(declareVector("k_std_dev")),
    _reactivity(declareVector("reactivity")),
    _reactivity_std_dev(declareVector("reactivity_std_dev"))
{
}

void
CriticalitySearchHistory::execute()
{
  const auto * search = _openmc_problem->criticalitySearch();
  if (!search)
    mooseError("A CriticalitySearchHistory requires a [CriticalitySearch] in the [Problem] block!");

  _input = search->inputs();
  _k = search->kValues();
  _k_std_dev = search->kStdDevValues();

  constexpr Real to_pcm = 1e5;
  _reactivity.resize(_k.size());
  _reactivity_std_dev.resize(_k.size());
  for (const auto i : index_range(_k))
  {
    _reactivity[i] = (_k[i] - 1.0) / _k[i] * to_pcm;

    // first-order propagation of the uncertainty in k through rho = 1 - 1 / k
    _reactivity_std_dev[i] = _k_std_dev[i] / (_k[i] * _k[i]) * to_pcm;
  }
}

#endif
