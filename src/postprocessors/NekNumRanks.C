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

#ifdef ENABLE_NEK_COUPLING

#include "NekNumRanks.h"

registerMooseObject("CardinalApp", NekNumRanks);

InputParameters
NekNumRanks::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params += NekBase::validParams();
  params.addClassDescription("Number of MPI ranks used to run NekRS");
  return params;
}

NekNumRanks::NekNumRanks(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), NekBase(this, parameters)
{
}

Real
NekNumRanks::getValue() const
{
  return nekrs::commSize();
}

#endif
