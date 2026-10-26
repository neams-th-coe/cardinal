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
  InputParameters params = NekFieldPostprocessor::validParams();
  params.addRequiredParam<Point>("point", "The physical point where the field will be evaluated");
  params.addClassDescription("Uses NekRS's pointInterpolation to query the NekRS solution at a "
                             "point (does not need to be a grid point).");
  return params;
}

NekPointValue::NekPointValue(const InputParameters & parameters)
  : NekFieldPostprocessor(parameters), _point(getParam<Point>("point")), _value(0)
{
}

void
NekPointValue::execute()
{
  // convert to non-dimensional before we use the points to get the solution from NekRS
  std::vector<dfloat> x = {_point(0) / nekrs::referenceLength()};
  std::vector<dfloat> y = {_point(1) / nekrs::referenceLength()};
  std::vector<dfloat> z = {_point(2) / nekrs::referenceLength()};

  int n = x.size();

  auto nrs = nekrs::nrsPtr();

  // set the points to be interpolated; we include this every time we call the
  // interpolator, in case the mesh is moving. TODO: auto-detect for efficiency
  auto interp = pointInterpolation_t(nrs->meshV, platform->comm.mpiComm());
  interp.setPoints(x, y, z);
  const auto verbosity = pointInterpolation_t::VerbosityLevel::Basic;
  interp.find(verbosity);

  // if this slot is not used for coupling, we are responsible for copying it to
  // device before calling the interp function. Otherwise, Cardinal handles copying
  // to device automatically.
  if (_field == field::usrwrk00 && !_nek_problem->isUsrWrkSlotReservedForCoupling(0))
    _nek_problem->copyIndividualScratchSlot(0);
  if (_field == field::usrwrk01 && !_nek_problem->isUsrWrkSlotReservedForCoupling(1))
    _nek_problem->copyIndividualScratchSlot(1);
  if (_field == field::usrwrk02 && !_nek_problem->isUsrWrkSlotReservedForCoupling(2))
    _nek_problem->copyIndividualScratchSlot(2);

  // interpolate the field onto those points
  auto interpolated = nekrs::interpolateToPoints(interp, _field, n, _velocity_direction);

  // only one point is being queried
  _value = interpolated[0];

  // function value to subtract off; the function is dimensional, so evaluate at the
  // dimensional position and dimensional time. This time will be the time of the timestep if
  // called with execute_on = timestep_begin; otherwise, it will be the time corresponding to
  // the next time step if called with execute_on = timestep_end (the default)
  auto function_value = _function ? _function->value(_t, _point) : 0.0;
  _value -= function_value;
}

Real
NekPointValue::getValue() const
{
  return _value;
}

#endif
