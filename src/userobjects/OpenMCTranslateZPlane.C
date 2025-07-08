#ifdef ENABLE_OPENMC_COUPLING

#include "OpenMCTranslateZPlane.h"
#include "openmc/surface.h"

registerMooseObject("CardinalApp", OpenMCTranslateZPlane);

InputParameters
OpenMCTranslateZPlane::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += OpenMCBase::validParams();
  params.addRequiredParam<int32_t>("surface_id", "ID of surface to change position");
  params.addRequiredParam<double>("displacement", "Change in location of surface (cm)");
  params.declareControllable("displacement");
  params.addClassDescription("Displaces a surface in an OpenMC geometry.");
  return params;
}

OpenMCTranslateZPlane::OpenMCTranslateZPlane(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    OpenMCBase(this, parameters),
    _surface_id(getParam<int32_t>("surface_id")),
    _displacement(getParam<double>("displacement"))
{
  // probably not correct
  _openmc_problem->catchOpenMCError(_surface_index = openmc::model::surface_map[_surface_id], 
                                    "get the surface index for surface with ID " +
                                    std::to_string(_surface_id));
}

void
OpenMCTranslateZPlane::setValue()
{
  try
  {
    openmc::model::surfaces[_surface_index]->z0_ += _displacement;
  }
  catch (const std::exception & e)
  {
    mooseError("In attempting to set change ZPlane position in the '" + name() +
               "' UserObject, OpenMC reported:\n\n" + e.what());
  }
}

#endif
