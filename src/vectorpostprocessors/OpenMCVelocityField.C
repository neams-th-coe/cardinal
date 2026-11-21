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

#include "OpenMCVelocityField.h"
#include "openmc/simulation.h"

registerMooseObject("CardinalApp", OpenMCVelocityField);

InputParameters
OpenMCVelocityField::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription(
      "Returns a vector of velocity values corresponding to each mesh cell of "
      "the OpenMC velocity field.");
  return params;
}

OpenMCVelocityField::OpenMCVelocityField(const InputParameters & params)
  : GeneralVectorPostprocessor(params),
    _cell_id(declareVector("cell_id")),
    _vx(declareVector("vx")),
    _vy(declareVector("vy")),
    _vz(declareVector("vz"))
{
}

void
OpenMCVelocityField::initialize()
{
  _cell_id.clear();
  _vx.clear();
  _vy.clear();
  _vz.clear();
}

void
OpenMCVelocityField::execute()
{
  size_t n = openmc_velocity_field_size();

  _cell_id.resize(n);
  _vx.resize(n);
  _vy.resize(n);
  _vz.resize(n);

  for (size_t i = 0; i < n; i++)
  {
    double temp[3];
    int err = openmc_velocity_field_get_value(i, temp);
    if (err != 0)
      mooseError("Failed to get velocity from OpenMC at index ", i);

    _cell_id[i] = i;
    _vx[i] = temp[0];
    _vy[i] = temp[1];
    _vz[i] = temp[2];
  }
}
