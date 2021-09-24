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
