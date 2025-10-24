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

#include "NekMeshDeformation.h"
#include "DisplacedProblem.h"

registerMooseObject("CardinalApp", NekMeshDeformation);

extern nekrs::usrwrkIndices indices;

InputParameters
NekMeshDeformation::validParams()
{
  auto params = FieldTransferBase::validParams();
  params.addClassDescription("Reads/writes mesh deformation between NekRS and MOOSE.");
  return params;
}

NekMeshDeformation::NekMeshDeformation(const InputParameters & parameters)
  : FieldTransferBase(parameters)
{
  if (_direction == "to_nek" /* && nekrs::hasBlendingSolver() */)
  {
    if (_usrwrk_slot.size() != 3)
      paramError("usrwrk_slot", "For mesh deformation, 'usrwrk_slot' must be of length 3");

    // all the mesh velocities scale the same way
    auto d = nekrs::nondimensionalDivisor(field::mesh_velocity_x);
    auto a = nekrs::nondimensionalAdditive(field::mesh_velocity_x);
    addExternalVariable(_usrwrk_slot[0], _variable + "_x", a, d);
    addExternalVariable(_usrwrk_slot[1], _variable + "_y", a, d);
    addExternalVariable(_usrwrk_slot[2], _variable + "_z", a, d);

    indices.mesh_velocity_x = _usrwrk_slot[0] * nekrs::fieldOffset();
    indices.mesh_velocity_y = _usrwrk_slot[1] * nekrs::fieldOffset();
    indices.mesh_velocity_z = _usrwrk_slot[2] * nekrs::fieldOffset();
  }
  else
  {
    // technically, we do not need usrwrk_slot when not using the blending mesh solver,
    // since we directly apply the displacements to the entire mesh volume. However,
    // the base class is set up to require usrwrk_slot whenever we have displacements
    // being applied. Because the mesh deformation is not a widely used feature yet,
    // we ignore this annoyance. If someone wants to fix in future with a redesign
    // they are welcome to.
    // checkUnusedParam(parameters, "usrwrk_slot", "not using the blending mesh solver");
  }

  if (_direction == "from_nek")
    paramError("direction",
               "The NekMeshDeformation currently only supports transfers 'to_nek'; contact the "
               "Cardinal developer team if you require reading of NekRS mesh coordinates.");

  if (_app.actionWarehouse().displacedMesh() && !nekrs::hasMovingMesh())
    mooseWarning(
        "Your NekRSMesh has 'displacements', but '" + _nek_problem.casename() +
        ".par' does not have a\n"
        "solver in the [MESH] block! The displacements transferred to NekRS will be unused.");

  if (nekrs::hasMovingMesh() && _nek_mesh->exactMirror())
    mooseError("An exact mesh mirror is not yet implemented for the boundary mesh solver.");

  int n_per_surf = _nek_mesh->exactMirror() ? std::pow(_nek_mesh->nekNumQuadraturePoints1D(), 2.0)
                                            : _nek_mesh->numVerticesPerSurface();
  int n_per_vol = _nek_mesh->exactMirror() ? std::pow(_nek_mesh->nekNumQuadraturePoints1D(), 3.0)
                                           : _nek_mesh->numVerticesPerVolume();

  if (nekrs::hasMovingMesh())
  {
    int n_entries = _nek_mesh->volume() ? n_per_vol : n_per_surf;
    _displacement_x = (double *)calloc(n_entries, sizeof(double));
    _displacement_y = (double *)calloc(n_entries, sizeof(double));
    _displacement_z = (double *)calloc(n_entries, sizeof(double));

    if (nekrs::hasBlendingSolver())
      _mesh_velocity_elem = (double *)calloc(n_entries, sizeof(double));
  }

  auto boundary = _nek_mesh->boundary();
  if (!boundary && nekrs::hasBlendingSolver())
    mooseError("'" + _nek_problem.casename() +
               ".par' has a solver in the [MESH] block. This solver uses\n"
               "boundary displacement values from MOOSE to move the NekRS mesh. Please indicate\n"
               "the 'boundary' for which mesh motion is coupled from MOOSE to NekRS.");

  if (nekrs::hasBlendingSolver())
  {
    bool has_one_mv_bc = false;
    for (const auto & b : *boundary)
    {
      if (nekrs::isMovingMeshBoundary(b))
      {
        has_one_mv_bc = true;
        break;
      }
    }

    if (!has_one_mv_bc)
      mooseError("For boundary-coupled moving mesh problems, you need at least one "
                 "boundary in '" +
                 _nek_problem.casename() +
                 ".par'\nto be of the type 'codedFixedValue'"
                 " in the [MESH] block.");
  }

  if (!_nek_mesh->volume() && nekrs::hasUserMeshSolver())
    mooseError(
        "'" + _nek_problem.casename() +
        ".par' has 'solver = user' in the [MESH] block. With this solver,\n"
        "displacement values are sent to every GLL point in NekRS's volume. If you only are "
        "building\n"
        "a boundary mesh mirror, it's possible that some displacement values could result\n"
        "in negative Jacobians if a sideset moves beyond the bounds of an undeformed element.\n"
        "To eliminate this possibility, please enable 'volume = true' for NekRSMesh and send a\n"
        "whole-domain displacement to NekRS.");

  if (nekrs::hasBlendingSolver())
    _nek_mesh->initializePreviousDisplacements();

  if (nekrs::hasUserMeshSolver())
    _nek_mesh->saveInitialVolMesh();
}

NekMeshDeformation::~NekMeshDeformation()
{
  freePointer(_displacement_x);
  freePointer(_displacement_y);
  freePointer(_displacement_z);
  freePointer(_mesh_velocity_elem);
}

void
NekMeshDeformation::sendDataToNek()
{
  if (nekrs::hasUserMeshSolver())
    sendVolumeDeformationToNek();
  else if (nekrs::hasBlendingSolver())
    sendBoundaryDeformationToNek();
  else
    mooseError("Unhandled mesh solver case in NekMeshDeformation!");

  _nek_problem.getDisplacedProblem()->updateMesh();
}

void
NekMeshDeformation::sendVolumeDeformationToNek()
{
  _console << "Sending volume deformation to NekRS" << std::endl;

  auto d = nekrs::nondimensionalDivisor(field::x_displacement);
  auto a = nekrs::nondimensionalAdditive(field::x_displacement);
  for (unsigned int e = 0; e < _nek_mesh->numVolumeElems(); e++)
  {
    // We can only write into the nekRS scratch space if that face is "owned" by the current process
    if (nekrs::commRank() != _nek_mesh->volumeCoupling().processor_id(e))
      continue;

    _nek_problem.mapVolumeDataToNekVolume(
        e, _variable_number[_variable + "_x"], d, a, &_displacement_x);
    _nek_problem.writeVolumeDisplacement(
        e, _displacement_x, field::x_displacement, &(_nek_mesh->nek_initial_x()));

    _nek_problem.mapVolumeDataToNekVolume(
        e, _variable_number[_variable + "_y"], d, a, &_displacement_y);
    _nek_problem.writeVolumeDisplacement(
        e, _displacement_y, field::y_displacement, &(_nek_mesh->nek_initial_y()));

    _nek_problem.mapVolumeDataToNekVolume(
        e, _variable_number[_variable + "_z"], d, a, &_displacement_z);
    _nek_problem.writeVolumeDisplacement(
        e, _displacement_z, field::z_displacement, &(_nek_mesh->nek_initial_z()));
  }
}

void
NekMeshDeformation::sendBoundaryDeformationToNek()
{
  _console << "Sending boundary deformation to NekRS..." << std::endl;

  auto d = nekrs::nondimensionalDivisor(field::x_displacement);
  auto a = nekrs::nondimensionalAdditive(field::x_displacement);
  if (!_nek_mesh->volume())
  {
    for (unsigned int e = 0; e < _nek_mesh->numSurfaceElems(); e++)
    {
      // We can only write into the nekRS scratch space if that face is "owned" by the current
      // process
      if (nekrs::commRank() != _nek_mesh->boundaryCoupling().processor_id(e))
        continue;

      _nek_problem.mapFaceDataToNekFace(
          e, _variable_number[_variable + "_x"], d, a, &_displacement_x);
      calculateMeshVelocity(e, field::mesh_velocity_x);
      _nek_problem.writeBoundarySolution(
          _usrwrk_slot[0] * nekrs::fieldOffset(), e, _mesh_velocity_elem);

      _nek_problem.mapFaceDataToNekFace(
          e, _variable_number[_variable + "_y"], d, a, &_displacement_y);
      calculateMeshVelocity(e, field::mesh_velocity_y);
      _nek_problem.writeBoundarySolution(
          _usrwrk_slot[1] * nekrs::fieldOffset(), e, _mesh_velocity_elem);

      _nek_problem.mapFaceDataToNekFace(
          e, _variable_number[_variable + "_z"], d, a, &_displacement_z);
      calculateMeshVelocity(e, field::mesh_velocity_z);
      _nek_problem.writeBoundarySolution(
          _usrwrk_slot[2] * nekrs::fieldOffset(), e, _mesh_velocity_elem);
    }
  }
  else
  {
    for (unsigned int e = 0; e < _nek_mesh->numVolumeElems(); ++e)
    {
      // We can only write into the nekRS scratch space if that face is "owned" by the current
      // process
      if (nekrs::commRank() != _nek_mesh->volumeCoupling().processor_id(e))
        continue;

      _nek_problem.mapFaceDataToNekVolume(
          e, _variable_number[_variable + "_x"], d, a, &_displacement_x);
      calculateMeshVelocity(e, field::mesh_velocity_x);
      _nek_problem.writeVolumeSolution(
          _usrwrk_slot[0] * nekrs::fieldOffset(), e, _mesh_velocity_elem);

      _nek_problem.mapFaceDataToNekVolume(
          e, _variable_number[_variable + "_y"], d, a, &_displacement_y);
      calculateMeshVelocity(e, field::mesh_velocity_y);
      _nek_problem.writeVolumeSolution(
          _usrwrk_slot[1] * nekrs::fieldOffset(), e, _mesh_velocity_elem);

      _nek_problem.mapFaceDataToNekVolume(
          e, _variable_number[_variable + "_z"], d, a, &_displacement_z);
      calculateMeshVelocity(e, field::mesh_velocity_z);
      _nek_problem.writeVolumeSolution(
          _usrwrk_slot[2] * nekrs::fieldOffset(), e, _mesh_velocity_elem);
    }
  }
}

void
NekMeshDeformation::calculateMeshVelocity(int e, const field::NekWriteEnum & field)
{
  int len =
      _nek_mesh->volume() ? _nek_mesh->numVerticesPerVolume() : _nek_mesh->numVerticesPerSurface();

  double dt = _nek_problem.transientExecutioner()->getTimeStepper()->getCurrentDT();

  double *displacement = nullptr, *prev_disp = nullptr;
  field::NekWriteEnum disp_field;

  switch (field)
  {
    case field::mesh_velocity_x:
      displacement = _displacement_x;
      prev_disp = _nek_mesh->prev_disp_x().data();
      disp_field = field::x_displacement;
      break;
    case field::mesh_velocity_y:
      displacement = _displacement_y;
      prev_disp = _nek_mesh->prev_disp_y().data();
      disp_field = field::y_displacement;
      break;
    case field::mesh_velocity_z:
      displacement = _displacement_z;
      prev_disp = _nek_mesh->prev_disp_z().data();
      disp_field = field::z_displacement;
      break;
    default:
      mooseError("Unhandled NekWriteEnum in NekRSProblem::calculateMeshVelocity!\n");
  }

  auto reference_v = nekrs::nondimensionalDivisor(field::velocity);
  for (int i = 0; i < len; i++)
    _mesh_velocity_elem[i] = (displacement[i] - prev_disp[(e * len) + i]) / dt / reference_v;

  _nek_mesh->updateDisplacement(e, displacement, disp_field);
}

#endif
