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

#include "NekRSProblem.h"
#include "FieldTransferBase.h"
#include "UserErrorChecking.h"

std::map<unsigned int, std::string> FieldTransferBase::_field_usrwrk_map;

InputParameters
FieldTransferBase::validParams()
{
  auto params = NekTransferBase::validParams();
  params.addParam<std::vector<unsigned int>>(
      "usrwrk_slot",
      "When 'direction = to_nek', the slot(s) in the usrwrk array to write the incoming data; "
      "provide one entry for each quantity being passed");
  params.addClassDescription("Base class for transferring field data between NekRS and MOOSE.");
  params.registerBase("FieldTransfer");
  params.registerSystemAttributeName("FieldTransfer");
  return params;
}

FieldTransferBase::FieldTransferBase(const InputParameters & parameters)
  : NekTransferBase(parameters), _variable(name())
{
  if (_direction == "to_nek")
  {
    checkRequiredParam(parameters, "usrwrk_slot", "writing data 'to_nek'");
    _usrwrk_slot = getParam<std::vector<unsigned int>>("usrwrk_slot");

    // there should be no duplicates within a single transfer
    std::set<unsigned int> slots(_usrwrk_slot.begin(), _usrwrk_slot.end());
    if (slots.size() != _usrwrk_slot.size())
    {
      std::string slots;
      for (const auto & u : _usrwrk_slot)
        slots += Moose::stringify(u) + " ";
      paramError(
          "usrwrk_slot",
          "There are duplicate entries in 'usrwrk_slot': " + slots +
              "; duplicate entries are not allowed because the field transfer will overwrite "
              "itself. Cardinal has allocated " +
              Moose::stringify(_nek_problem.nUsrWrkSlots()) +
              " usrwrk slots; if you need more, set 'n_usrwrk_slots' in the [Problem] block.");
    }

    // each slot should not be greater than the amount allocated
    for (const auto & u : _usrwrk_slot)
      checkAllocatedUsrwrkSlot(u);
  }

  _n_per_surf = _nek_mesh->exactMirror() ? std::pow(_nek_mesh->nekNumQuadraturePoints1D(), 2.0)
                                         : _nek_mesh->numVerticesPerSurface();
  _n_per_vol = _nek_mesh->exactMirror() ? std::pow(_nek_mesh->nekNumQuadraturePoints1D(), 3.0)
                                        : _nek_mesh->numVerticesPerVolume();
  _v_face = (double *)calloc(_n_per_surf, sizeof(double));
  _v_elem = (double *)calloc(_n_per_vol, sizeof(double));
  _external_data = (double *)calloc(_nek_problem.nPoints(), sizeof(double));
}

FieldTransferBase::~FieldTransferBase()
{
  freePointer(_v_face);
  freePointer(_v_elem);
  freePointer(_external_data);
}

void
FieldTransferBase::addExternalVariable(const std::string name)
{
  InputParameters var_params = _factory.getValidParams("MooseVariable");
  var_params.set<MooseEnum>("family") = "LAGRANGE";

  switch (_nek_mesh->order())
  {
    case order::first:
      var_params.set<MooseEnum>("order") = "FIRST";
      break;
    case order::second:
      var_params.set<MooseEnum>("order") = "SECOND";
      break;
    default:
      mooseError("Unhandled 'NekOrderEnum' in 'FieldTransferBase'!");
  }

  _nek_problem.checkDuplicateVariableName(name);
  _nek_problem.addAuxVariable("MooseVariable", name, var_params);
  _variable_number.insert(std::pair<std::string, unsigned int>(
      name, _nek_problem.getAuxiliarySystem().getFieldVariable<Real>(0, name).number()));
}

void
FieldTransferBase::addExternalVariable(const unsigned int slot, const std::string name)
{
  addExternalVariable(name);

  // check that no other field transfer is trying to write into the same slot;
  // we don't need to check the scalar transfers, because they will always execute
  // after the field transfers
  bool duplicate_field = _field_usrwrk_map.find(slot) != _field_usrwrk_map.end();
  if (duplicate_field)
    paramError("usrwrk_slot",
               "A duplicate slot, " + Moose::stringify(slot) +
                   ", is being used by another FieldTransfer. Duplicate slots are not allowed for "
                   "field transfers because these transfers will overwrite all data in that slot. "
                   "If you need more slots, increase 'n_usrwrk_slots' in the [Problem] block.");

  _field_usrwrk_map.insert({slot, name});
}

void
FieldTransferBase::fillAuxVariable(const unsigned int var_number, const double * value)
{
  auto & solution = _nek_problem.getAuxiliarySystem().solution();
  auto sys_number = _nek_problem.getAuxiliarySystem().number();
  auto pid = _communicator.rank();

  for (unsigned int e = 0; e < _nek_mesh->numElems(); e++)
  {
    for (int build = 0; build < _nek_mesh->nMoosePerNek(); ++build)
    {
      auto elem_ptr = _nek_mesh->queryElemPtr(e * _nek_mesh->nMoosePerNek() + build);

      // Only work on elements we can find on our local chunk of a
      // distributed mesh
      if (!elem_ptr)
      {
        libmesh_assert(!_nek_mesh->getMesh().is_serial());
        continue;
      }

      for (unsigned int n = 0; n < _nek_mesh->numVerticesPerElem(); n++)
      {
        auto node_ptr = elem_ptr->node_ptr(n);

        // For each face vertex, we can only write into the MOOSE auxiliary fields if that
        // vertex is "owned" by the present MOOSE process.
        if (node_ptr->processor_id() == pid)
        {
          int node_index = _nek_mesh->nodeIndex(n);
          auto node_offset =
              (e * _nek_mesh->nMoosePerNek() + build) * _nek_mesh->numVerticesPerElem() +
              node_index;

          // get the DOF for the auxiliary variable, then use it to set the value in the auxiliary
          // system
          auto dof_idx = node_ptr->dof_number(sys_number, var_number, 0);
          solution.set(dof_idx, value[node_offset]);
        }
      }
    }
  }

  solution.close();
}
#endif
