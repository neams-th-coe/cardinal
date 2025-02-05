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

#include "NekPointValue.h"
#include "pointInterpolation.hpp"

registerMooseObject("CardinalApp", NekPointValue);

InputParameters
NekPointValue::validParams()
{
  InputParameters params = NekFieldInterface::validParams();
  params += NekPostprocessor::validParams();
  params.addRequiredParam<Point>("point", "The physical point where the field will be evaluated");
  params.addClassDescription("Uses NekRS's pointInterpolation to query the NekRS solution at a "
                             "point (does not need to be a grid point).");
  return params;
}

NekPointValue::NekPointValue(const InputParameters & parameters)
  : NekPostprocessor(parameters),
    NekFieldInterface(this, parameters),
    _point(getParam<Point>("point")),
    _value(0)
{
}

void
NekPointValue::execute()
{
  // the input functions are dimensional quantities; first, need to transform
  // them into non-dimensional form before NekRS evaluates them
  auto t = _t / nekrs::referenceTime();
  auto p = _point / nekrs::referenceLength();

  // if there is a shifting function, evaluate that function
  auto shift = _shift ? _shift->value(t, p) : 0.0;

  // if the field is just a function, we can evaluate directly without using
  // the interpolation in NekRS. We don't do any dimensionalization back from
  // NekRS's non-dimensional form, because we don't know exactly what this
  // function represents, physically (i.e., its units)
  if (_field == field::function)
  {
    _value = _function->value(t, p) - shift;
    return;
  }

  std::vector<dfloat> x = {p(0)};
  std::vector<dfloat> y = {p(1)};
  std::vector<dfloat> z = {p(2)};
  int n = x.size();

  nrs_t * nrs = (nrs_t *)nekrs::nrsPtr();

  // set the points to be interpolated; we include this every time we call the
  // interpolator, in case the mesh is moving. TODO: auto-detect for efficiency
  auto interp = pointInterpolation_t(nrs);
  interp.setPoints(n, x.data(), y.data(), z.data());
  const auto verbosity = pointInterpolation_t::VerbosityLevel::Basic;
  interp.find(verbosity);

  // interpolate the field onto those points
  occa::memory o_interpolated;
  int n_values = n;
  switch (_field)
  {
    case field::velocity_x:
    case field::velocity_y:
    case field::velocity_z:
    case field::velocity:
    case field::velocity_x_squared:
    case field::velocity_y_squared:
    case field::velocity_z_squared:
    case field::velocity_component:
      n_values = n * nrs->NVfields;
      o_interpolated = platform->device.malloc<dfloat>(n_values);
      interp.eval(n_values, nrs->fieldOffset, nrs->cds->o_U, n, o_interpolated);
      break;
    case field::pressure:
      o_interpolated = platform->device.malloc<dfloat>(n);
      interp.eval(1, nrs->fieldOffset, nrs->o_P, n, o_interpolated);
      break;
    case field::temperature:
    case field::scalar01:
    case field::scalar02:
    case field::scalar03:
      n_values = n * nrs->Nscalar;
      o_interpolated = platform->device.malloc<dfloat>(n_values);
      interp.eval(n_values, nrs->fieldOffset, nrs->cds->o_S, n, o_interpolated);
      break;
    case field::unity:
      _value = 1;
      break;
    default:
      mooseError("Unhandled NekFieldEnum in NekPointValue!");
  }

  // the interpolation happens on device, so we need to copy it back to the host
  std::vector<dfloat> interpolated(n_values);
  o_interpolated.copyTo(interpolated.data(), n_values);

  // because of NekRS's way of storing solution, we need extra steps to actually
  // return what the user wants
  switch (_field)
  {
    case field::velocity_x:
      _value = interpolated[0];
      break;
    case field::velocity_y:
      _value = interpolated[1];
      break;
    case field::velocity_z:
      _value = interpolated[2];
      break;
    case field::velocity:
      _value = std::sqrt(interpolated[0] * interpolated[0] + interpolated[1] * interpolated[1] +
                         interpolated[2] * interpolated[2]);
      break;
    case field::velocity_x_squared:
      _value = interpolated[0] * interpolated[0];
      break;
    case field::velocity_y_squared:
      _value = interpolated[1] * interpolated[1];
      break;
    case field::velocity_z_squared:
      _value = interpolated[2] * interpolated[2];
      break;
    case field::velocity_component:
      _value = interpolated[0] * _velocity_direction(0) + interpolated[1] * _velocity_direction(1) + interpolated[2] * _velocity_direction(2);
      break;
    case field::pressure:
      _value = interpolated[0];
      break;
    case field::temperature:
      _value = interpolated[0];
      break;
    case field::scalar01:
      _value = interpolated[1];
      break;
    case field::scalar02:
      _value = interpolated[2];
      break;
    case field::scalar03:
      _value = interpolated[3];
      break;
    case field::unity:
      break;
    default:
      mooseError("Unhandled NekFieldEnum in NekPointValue!");
  }

  _value -= shift;
  nekrs::dimensionalize(_field, _value);

  // need to add the temperature shift, if field is temperature
  if (_field == field::temperature)
    _value += nekrs::referenceTemperature();
}

Real
NekPointValue::getValue() const
{
  return _value;
}

#endif
