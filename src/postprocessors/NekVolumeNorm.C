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

#include "NekVolumeNorm.h"
#include "NekInterface.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>
#include <string>

registerMooseObject("CardinalApp", NekVolumeNorm);

InputParameters
NekVolumeNorm::validParams()
{
  InputParameters params = NekFieldPostprocessor::validParams();
  params.addParam<std::string>("N",
                               "2",
                               "Order of the volume norm. Specify a finite real value N >= 1, "
                               "or use 'infinity' for the L-infinity norm.");

  params.addClassDescription("Computes a finite L^N norm or an L-infinity norm of the difference "
                             "between a NekRS solution field and an optional analytical function.");

  return params;
}

NekVolumeNorm::NekVolumeNorm(const InputParameters & parameters) : NekFieldPostprocessor(parameters)
{
  if (_nek_problem->nondimensional())
    mooseError("The NekVolumeNorm object does not yet support non-dimensional runs! "
               "Please contact the development team to accelerate this feature "
               "addition to support your use case.");

  auto value = getParam<std::string>("N");

  std::transform(
      value.begin(), value.end(), value.begin(), [](unsigned char c) { return std::tolower(c); });

  if (value == "infinity")
    _N = std::numeric_limits<Real>::infinity();

  std::size_t parsed_characters = 0;

  try
  {
    _N = std::stod(value, &parsed_characters);
  }
  catch (const std::exception &)
  {
    paramError("N", "Specify a finite real value N >= 1, or use 'infinity'.");
  }

  if (parsed_characters != value.size())
    paramError("N", "Specify a finite real value N >= 1, or use 'infinity'.");

  if (_N < 1.0)
    paramError("N", "Norm orders must satisfy N >= 1.");
}

Real
NekVolumeNorm::getValue() const
{
  return nekrs::volumeNorm(_field, _pp_mesh, _function, _t, _N);
}

#endif
