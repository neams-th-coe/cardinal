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

#include "NekSpatialBinUserObject.h"
#include "NekInterface.h"
#include "CardinalUtils.h"

InputParameters
NekSpatialBinUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += NekBase::validParams();
  params += NekFieldInterface::validParams();
  params.addRequiredParam<std::vector<UserObjectName>>(
      "bins", "Userobjects providing a spatial bin given a point");
  params.addParam<unsigned int>(
      "interval",
      1,
      "Frequency (in number of time steps) with which to execute this user object; user objects "
      "can be expensive and not necessary to evaluate on every single time step. NOTE: you "
      "probably want to match with 'time_step_interval' in the Output");
  params.addParam<bool>(
      "map_space_by_qp",
      false,
      "Whether to map the NekRS spatial domain to a bin according to the element centroids (true) "
      "or quadrature point locations (false).");
  params.addParam<bool>(
      "check_zero_contributions",
      true,
      "Whether to throw an error if no GLL points/element centroids in the NekRS mesh map to a "
      "spatial bin; this "
      "can be used to ensure that the bins are sufficiently big to get at least one contributing "
      "point from the NekRS mesh.");
  return params;
}

NekSpatialBinUserObject::NekSpatialBinUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    NekBase(this, parameters),
    NekFieldInterface(this, parameters, true /* allow normal */),
    _interval(getParam<unsigned int>("interval")),
    _bin_names(getParam<std::vector<UserObjectName>>("bins")),
    _map_space_by_qp(getParam<bool>("map_space_by_qp")),
    _check_zero_contributions(getParam<bool>("check_zero_contributions")),
    _bin_values(nullptr),
    _bin_values_x(nullptr),
    _bin_values_y(nullptr),
    _bin_values_z(nullptr),
    _bin_volumes(nullptr),
    _bin_counts(nullptr),
    _bin_partial_values(nullptr),
    _bin_partial_counts(nullptr)
{
  _fixed_mesh = !nekrs::hasMovingMesh();

  if (_bin_names.size() == 0)
    paramError("bins", "Length of vector must be greater than zero!");

  for (auto & b : _bin_names)
  {
    // first check that the user object exists
    if (!hasUserObjectByName<UserObject>(b))
      mooseError("Bin user object with name '" + b +
                 "' not found in problem. The user objects "
                 "in 'bins' must be listed before the '" +
                 name() + "' user object.");

    // then check that it's the right type
    if (!hasUserObjectByName<SpatialBinUserObject>(b))
      mooseError("Bin user object with name '" + b +
                 "' must inherit from SpatialBinUserObject.\n\n"
                 "Volume options: HexagonalSubchannelBin, LayeredBin, RadialBin\n"
                 "Side options: HexagonalSubchannelGapBin, LayeredGapBin");

    _bins.push_back(&getUserObjectByName<SpatialBinUserObject>(b));
  }

  _n_bins = num_bins();

  _bin_values = (double *)calloc(_n_bins, sizeof(double));
  _bin_volumes = (double *)calloc(_n_bins, sizeof(double));
  _bin_partial_values = (double *)calloc(_n_bins, sizeof(double));
  _bin_counts = (int *)calloc(_n_bins, sizeof(int));
  _bin_partial_counts = (int *)calloc(_n_bins, sizeof(int));

  if (_field == field::velocity_component)
  {
    _bin_values_x = (double *)calloc(_n_bins, sizeof(double));
    _bin_values_y = (double *)calloc(_n_bins, sizeof(double));
    _bin_values_z = (double *)calloc(_n_bins, sizeof(double));
  }

  _has_direction = {false, false, false};
  _bin_providing_direction.resize(3);
  for (unsigned int b = 0; b < _bins.size(); ++b)
  {
    const auto & bin = _bins[b];

    // directions provided by this bin
    auto bin_directions = bin->directions();

    for (const auto & d : bin_directions)
    {
      if (_has_direction[d])
      {
        const auto & bin_providing_d = _bins[_bin_providing_direction[d]];
        mooseError("Cannot combine multiple distributions in the same coordinate direction!\n"
                   "Bin '" +
                   bin->name() + "' conflicts with bin '" + bin_providing_d->name() + "'.");
      }
      else
      {
        _has_direction[d] = true;
        _bin_providing_direction[d] = b;
      }
    }
  }

  // initialize all points to (0, 0, 0)
  for (unsigned int i = 0; i < _n_bins; ++i)
    _points.push_back(Point(0.0, 0.0, 0.0));

  // we will at most have 3 separate distributions
  if (_bins.size() == 1)
    computePoints1D();
  else if (_bins.size() == 2)
    computePoints2D();
  else
    computePoints3D();

  // with a user-specified direction, the direction for each bin is the same
  if (_field == field::velocity_component && _velocity_component == component::user)
    for (unsigned int i = 0; i < _n_bins; ++i)
      _velocity_bin_directions.push_back(_velocity_direction);
}

NekSpatialBinUserObject::~NekSpatialBinUserObject()
{
  freePointer(_bin_values);
  freePointer(_bin_volumes);
  freePointer(_bin_counts);
  freePointer(_bin_partial_values);
  freePointer(_bin_partial_counts);

  freePointer(_bin_values_x);
  freePointer(_bin_values_y);
  freePointer(_bin_values_z);
}

void
NekSpatialBinUserObject::execute()
{
  if (_fe_problem.timeStep() % _interval == 0)
    executeUserObject();
}

Point
NekSpatialBinUserObject::nekPoint(const int & local_elem_id, const int & local_node_id) const
{
  if (_map_space_by_qp)
    return nekrs::gllPoint(local_elem_id, local_node_id);
  else
    return nekrs::centroid(local_elem_id);
}

void
NekSpatialBinUserObject::resetPartialStorage()
{
  for (unsigned int i = 0; i < _n_bins; ++i)
  {
    _bin_partial_values[i] = 0.0;
    _bin_partial_counts[i] = 0;
  }
}

void
NekSpatialBinUserObject::computeBinVolumes()
{
  getBinVolumes();

  if (_check_zero_contributions)
  {
    for (unsigned int i = 0; i < _n_bins; ++i)
    {
      if (_bin_counts[i] == 0)
      {
        std::string map = _map_space_by_qp ? "GLL points" : "element centroids";
        mooseError(
            "Failed to map any " + map + " to bin " + Moose::stringify(i) +
            "!\n\n"
            "This can happen if the bins are much finer than the NekRS mesh or if the bins are "
            "defined in a way that results in bins entirely outside the NekRS domain. You can turn "
            "this error off by setting 'check_zero_contributions = false' (at your own risk!).");
      }
    }
  }
}

Real
NekSpatialBinUserObject::spatialValue(const Point & p) const
{
  return _bin_values[bin(p)];
}

const std::vector<unsigned int>
NekSpatialBinUserObject::unrolledBin(const unsigned int & total_bin_index) const
{
  std::vector<unsigned int> local_bins;
  local_bins.resize(_bins.size());

  int running_index = total_bin_index;
  for (int i = _bins.size() - 1; i >= 0; --i)
  {
    local_bins[i] = running_index % _bins[i]->num_bins();
    running_index -= local_bins[i];
    running_index /= _bins[i]->num_bins();
  }

  return local_bins;
}

const unsigned int
NekSpatialBinUserObject::bin(const Point & p) const
{
  // get the indices into each of the individual bin objects
  std::vector<unsigned int> indices;
  for (const auto & b : _bins)
    indices.push_back(b->bin(p));

  // convert to a total index into the multidimensional bin union
  unsigned int index = indices[0];
  for (unsigned int i = 1; i < _bins.size(); ++i)
    index = index * _bins[i]->num_bins() + indices[i];

  return index;
}

const unsigned int
NekSpatialBinUserObject::num_bins() const
{
  unsigned int num_bins = 1;
  for (const auto & b : _bins)
    num_bins *= b->num_bins();

  return num_bins;
}

void
NekSpatialBinUserObject::computePoints1D()
{
  for (unsigned int i = 0; i < _bins[0]->num_bins(); ++i)
  {
    std::vector<unsigned int> indices = {i};
    fillCoordinates(indices, _points[i]);
  }
}

void
NekSpatialBinUserObject::computePoints2D()
{
  int p = 0;
  for (unsigned int i = 0; i < _bins[0]->num_bins(); ++i)
  {
    for (unsigned int j = 0; j < _bins[1]->num_bins(); ++j, ++p)
    {
      std::vector<unsigned int> indices = {i, j};
      fillCoordinates(indices, _points[p]);
    }
  }
}

void
NekSpatialBinUserObject::computePoints3D()
{
  int p = 0;
  for (unsigned int i = 0; i < _bins[0]->num_bins(); ++i)
  {
    for (unsigned int j = 0; j < _bins[1]->num_bins(); ++j)
    {
      for (unsigned int k = 0; k < _bins[2]->num_bins(); ++k, ++p)
      {
        std::vector<unsigned int> indices = {i, j, k};
        fillCoordinates(indices, _points[p]);
      }
    }
  }
}

void
NekSpatialBinUserObject::fillCoordinates(const std::vector<unsigned int> & indices, Point & p) const
{
  for (unsigned int b = 0; b < _bins.size(); ++b)
  {
    const auto & bin = _bins[b];
    const auto & centers = bin->getBinCenters();

    for (const auto & d : bin->directions())
      p(d) = centers[indices[b]](d);
  }
}

#endif
