#include "NekSpatialBinUserObject.h"

InputParameters
NekSpatialBinUserObject::validParams()
{
  InputParameters params = NekUserObject::validParams();
  params.addRequiredParam<std::vector<UserObjectName>>("bins", "Userobjects providing a spatial bin given a point");
  params.addRequiredParam<MooseEnum>("field", getNekFieldEnum(), "Field to postprocess; "
    "options: velocity_x, velocity_y, velocity_z, "
    "velocity, temperature, pressure, unity");
  params.addParam<bool>("map_space_by_qp", false,
    "Whether to map the NekRS spatial domain to a bin according to the element centroids (true) "
    "or quadrature point locations (false).");
  return params;
}

NekSpatialBinUserObject::NekSpatialBinUserObject(const InputParameters & parameters)
  : NekUserObject(parameters),
    _bin_names(getParam<std::vector<UserObjectName>>("bins")),
    _field(getParam<MooseEnum>("field").getEnum<field::NekFieldEnum>()),
    _map_space_by_qp(getParam<bool>("map_space_by_qp"))
{
  if (_nek_problem->nondimensional() && _field == field::temperature)
    mooseError("Spatial bin user objects are not yet available for non-dimensional solutions with temperature! "
      "Please change your MOOSE-wrapped input file to be in the same non-dimensional units as your NekRS case.");

  if (_bin_names.size() == 0)
    paramError("bins", "Length of vector must be greater than zero!");

  for (auto & b : _bin_names)
  {
    // first check that the user object exists
    if (!hasUserObjectByName<UserObject>(b))
      mooseError("Bin user object with name '" + b + "' not found in problem.\nThe user objects "
        "in 'bins' must be listed before the '" + name() + "' user object.");

    // then check that it's the right type
    if (!hasUserObjectByName<SpatialBinUserObject>(b))
      mooseError("Bin user object with name '" + b + "' must inherit from SpatialBinUserObject.\n\n"
        "Options: HexagonalSubchannelBin, LayeredBin, RadialBin");

    _bins.push_back(&getUserObjectByName<SpatialBinUserObject>(b));
  }

  _bin_values = (double *) calloc(num_bins(), sizeof(double));

  checkValidField(_field);

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
          "Bin '" + bin->name() + "' conflicts with bin '" + bin_providing_d->name() + "'.");
       }
      else
      {
        _has_direction[d] = true;
        _bin_providing_direction[d] = b;
      }
    }
  }

  // initialize all points to (0, 0, 0)
  int n_bins = num_bins();
  for (unsigned int i = 0; i < n_bins; ++i)
    _points.push_back(Point(0.0, 0.0, 0.0));

  // we will at most have 3 separate distributions
  if (_bins.size() == 1)
    computePoints1D();
  else if (_bins.size() == 2)
    computePoints2D();
  else
    computePoints3D();
}

NekSpatialBinUserObject::~NekSpatialBinUserObject()
{
  free(_bin_values);
}

Real
NekSpatialBinUserObject::spatialValue(const Point & p) const
{
  return _bin_values[bin(p)];
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
