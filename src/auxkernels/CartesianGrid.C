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

#include "CartesianGrid.h"
#include "libmesh/mesh_tools.h"

registerMooseObject("CardinalApp", CartesianGrid);

InputParameters
CartesianGrid::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredRangeCheckedParam<unsigned int>("nx", "nx > 0", "Number of bins in x direction");
  params.addRequiredRangeCheckedParam<unsigned int>("ny", "ny > 0", "Number of bins in y direction");
  params.addRequiredRangeCheckedParam<unsigned int>("nz", "nz > 0", "Number of bins in z direction");
  params.addClassDescription("Divide space into 3-D Cartesian bins");
  return params;
}

CartesianGrid::CartesianGrid(const InputParameters & parameters) :
  AuxKernel(parameters),
  _nx(getParam<unsigned int>("nx")),
  _ny(getParam<unsigned int>("ny")),
  _nz(getParam<unsigned int>("nz"))
{
  BoundingBox bounding_box = MeshTools::create_bounding_box(_mesh);
  _min = bounding_box.min();
  auto max = bounding_box.max();
  _dx = (max(0) - _min(0)) / _nx;
  _dy = (max(1) - _min(1)) / _ny;
  _dz = (max(2) - _min(2)) / _nz;
}

Real
CartesianGrid::computeValue()
{
  Point pt = _current_elem->vertex_average();
  int x = (pt(0) - _min(0))/ _dx;
  int y = (pt(1) - _min(1))/ _dy;
  int z = (pt(2) - _min(2))/ _dz;
  return x + y * _nx + z * _nx * _ny;
}
