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

#include "NekRSStandaloneProblem.h"
#include "NekInterface.h"
#include "UserErrorChecking.h"

registerMooseObject("CardinalApp", NekRSStandaloneProblem);

InputParameters
NekRSStandaloneProblem::validParams()
{
  InputParameters params = NekRSProblemBase::validParams();

  params.addParam<bool>("calculate_ros_tensor", false, "Whether we are calculating the "
                        "rate of strain tensor.");

  params.addParam<bool>("traction_on_boundary", false, "Whether we are calculating the "
                        "traction components on the specified sideset/boundary.");

  return params;
}

NekRSStandaloneProblem::NekRSStandaloneProblem(const InputParameters & params)
  : NekRSProblemBase(params),
    _calculate_ros_tensor(getParam<bool>("calculate_ros_tensor")),
    _traction_on_boundary(getParam<bool>("traction_on_boundary"))
{
  // It doesn't make sense to specify both a boundary and volume for a
  // standalone case, because we are only using the boundary/volume for
  // data extraction (and if you set 'volume = true', you automatically get
  // the solution on the boundaries as well).
  if (_nek_mesh->boundary() && _nek_mesh->volume())
    mooseWarning("When 'volume = true' for '" + type() +
                 "', it is redundant to also set 'boundary'.\n"
                 "Boundary IDs will be ignored.");

  if (nekrs::hasMovingMesh())
    mooseWarning("This class does not transfer mesh displacements from NekRS to Cardinal.\n"
                 "The [Mesh] in Cardinal won't reflect NekRS's internal mesh changes.");

  _minimum_scratch_size_for_coupling = 0;
  if (!params.isParamSetByUser("n_usrwrk_slots"))
    _n_usrwrk_slots = 0;
}

// initialize and calc traction
void
NekRSStandaloneProblem::addROSTensorVariables()
{
  nekrs::initializeROSTensor();
  _var_names.push_back("ros_s11");
  _var_names.push_back("ros_s22");
  _var_names.push_back("ros_s33");
  _var_names.push_back("ros_s12");
  _var_names.push_back("ros_s23");
  _var_names.push_back("ros_s13");
}

void
NekRSStandaloneProblem::addTractionVariables()
{
  nekrs::initializeTraction();
  _var_names.push_back("tr_x");
  _var_names.push_back("tr_y");
  _var_names.push_back("tr_z");
}

void
NekRSStandaloneProblem::addExternalVariables()
{
  if(_calculate_ros_tensor)
    addROSTensorVariables();

  if(_traction_on_boundary)
    addTractionVariables();
}
// transfer traction as output

#endif
