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

#include "SAMNekRSProblem.h"
#include "Moose.h"
#include "AuxiliarySystem.h"
#include "TimeStepper.h"
#include "NekInterface.h"
#include "TimedPrint.h"
#include "MooseUtils.h"
#include "CardinalUtils.h"

#include "nekrs.hpp"
#include "nekInterface/nekInterfaceAdapter.hpp"

registerMooseObject("CardinalApp", SAMNekRSProblem);

bool SAMNekRSProblem::_first = true;

InputParameters
SAMNekRSProblem::validParams()
{
  InputParameters params = NekRSProblemBase::validParams();
//  params.addParam<PostprocessorName>("max_T", "If provided, postprocessor used to limit the maximum "
//    "temperature (in dimensional form) in the nekRS problem");

  params.addParam<bool>("moving_mesh", false, "Moving mesh");

  return params;
}

SAMNekRSProblem::SAMNekRSProblem(const InputParameters &params) : NekRSProblemBase(params),
    _moving_mesh(getParam<bool>("moving_mesh"))
{

  // Depending on the type of coupling, initialize various problem parameters
//  if (_boundary && !_volume) // only boundary coupling
//    _flux_face = (double *) calloc(_n_vertices_per_surface, sizeof(double));
}

SAMNekRSProblem::~SAMNekRSProblem()
{
  nekrs::freeScratch();

//  freePointer(_flux_face);
}

void
SAMNekRSProblem::initialSetup()
{
  if (nekrs::buildOnly())
    return;

  NekRSProblemBase::initialSetup();

//  if (_boundary)
//    _flux_integral = &getPostprocessorValueByName("flux_integral");

}

//void
//SAMNekRSProblem::sendBoundaryHeatFluxToNek()
//{
//  auto & solution = _aux->solution();
//  auto sys_number = _aux->number();
//
//  if (_first)
//  {
//    _serialized_solution->init(_aux->sys().n_dofs(), false, SERIAL);
//    _first = false;
//  }
//
//  solution.localize(*_serialized_solution);
//
//  auto & mesh = _nek_mesh->getMesh();
//
//  {
//    _console << "Sending heat flux to NekRS boundary " << Moose::stringify(*_boundary) << std::endl;
//
//    if (!_volume)
//    {
//      for (unsigned int e = 0; e < _n_surface_elems; e++)
//      {
//        auto elem_ptr = mesh.query_elem_ptr(e);
//
//        // Only work on elements we can find on our local chunk of a
//        // distributed mesh
//        if (!elem_ptr)
//        {
//          libmesh_assert(!mesh.is_serial());
//          continue;
//        }
//
//        for (unsigned int n = 0; n < _n_vertices_per_surface; n++)
//        {
//          auto node_ptr = elem_ptr->node_ptr(n);
//
//          // For each face, get the flux at the libMesh nodes. This will be passed into
//          // nekRS, which will interpolate onto its GLL points. Because we are looping over
//          // nodes from libMesh, we need to get the GLL index known by nekRS and use it to
//          // determine the offset in the nekRS arrays.
//          int node_index = _nek_mesh->boundaryNodeIndex(n);
//          auto node_offset = e * _n_vertices_per_surface + node_index;
//          auto dof_idx = node_ptr->dof_number(sys_number, _avg_flux_var, 0);
//          _flux_face[node_index] = (*_serialized_solution)(dof_idx) / nekrs::solution::referenceFlux();
//        }
//
//        // Now that we have the flux at the nodes of the NekRSMesh, we can interpolate them
//        // onto the nekRS GLL points
//        nekrs::flux(e, _nek_mesh->order(), _flux_face);
//      }
//    }
//    else if (_volume)
//    {
//      // For the case of a boundary-only coupling, we could just loop over the elements on
//      // the boundary of interest and write (carefully) into the volume nrs-usrwrk array. Now,
//      // our flux variable is defined over the entire volume (maybe the MOOSE transfer only sent
//      // meaningful values to the coupling boundaries), so we need to do a volume interpolation
//      // of the flux into nrs->usrwrk, rather than a face interpolation. This could definitely be
//      // optimized in the future to truly only just write the boundary values into the nekRS
//      // scratch space rather than the volume values, but it looks right now that our biggest
//      // expense occurs in the MOOSE transfer system, not these transfers internally to nekRS.
//      for (unsigned int e = 0; e < _n_volume_elems; ++e)
//      {
//        int n_faces_on_boundary = nekrs::mesh::facesOnBoundary(e);
//
//        auto elem_ptr = mesh.query_elem_ptr(e);
//
//        // Only work on elements we can find on our local chunk of a
//        // distributed mesh
//        if (!elem_ptr)
//          {
//            libmesh_assert(!mesh.is_serial());
//            continue;
//          }
//
//        // though the flux is a volume field, the only meaningful values are on the coupling
//        // boundaries, so we can just skip this interpolation if this volume element isn't on
//        // a coupling boundary, because that flux data isn't used anyways
//        if (n_faces_on_boundary > 0)
//        {
//          auto elem_ptr = mesh.elem_ptr(e);
//
//          for (unsigned int n = 0; n < _n_vertices_per_volume; ++n)
//          {
//            auto node_ptr = elem_ptr->node_ptr(n);
//
//            // For each element, get the flux at the libMesh nodes. This will be passed into
//            // nekRS, which will interpolate onto its GLL points. Because we are looping over
//            // nodes from libMesh, we need to get the GLL index known by nekRS and use it to
//            // determine the offset in the nekRS arrays.
//            int node_index = _nek_mesh->volumeNodeIndex(n);
//            auto node_offset = e * _n_vertices_per_volume + node_index;
//            auto dof_idx = node_ptr->dof_number(sys_number, _avg_flux_var, 0);
//            _flux_elem[node_index] = (*_serialized_solution)(dof_idx) / nekrs::solution::referenceFlux();
//          }
//
//          // Now that we have the flux at the nodes of the NekRSMesh, we can interpolate them
//          // onto the nekRS GLL points
//          nekrs::writeVolumeSolution(e, _nek_mesh->order(), field::flux, _flux_elem);
//        }
//      }
//    }
//  }
//
//  // Because the NekRSMesh may be quite different from that used in the app solving for
//  // the heat flux, we will need to normalize the total flux on the nekRS side by the
//  // total flux computed by the coupled MOOSE app. For this and the next check of the
//  // flux integral, we need to scale the integral back up again to the dimensional form
//  // for the sake of comparison.
//  const Real scale_squared = _nek_mesh->scaling() * _nek_mesh->scaling();
//  const double nek_flux = nekrs::fluxIntegral();
//  const double moose_flux = *_flux_integral;
//
//  // For the sake of printing diagnostics to the screen regarding the flux normalization,
//  // we first scale the nek flux by any unit changes and then by the reference flux.
//  const double nek_flux_print_mult = scale_squared * nekrs::solution::referenceFlux();
//  double normalized_nek_flux = 0.0;
//  bool successful_normalization;
//
//  _console << "Normalizing total NekRS flux of " << Moose::stringify(nek_flux * nek_flux_print_mult) <<
//    " to the conserved MOOSE value of " << Moose::stringify(moose_flux) << std::endl;
//
//  successful_normalization = nekrs::normalizeFlux(moose_flux, nek_flux, normalized_nek_flux);
//
//  // If before normalization, there is a large difference between the nekRS imposed flux
//  // and the MOOSE flux, this could mean that there is a poor match between the domains,
//  // even if neither value is zero. For instance, if you forgot that the nekRS mesh is in
//  // units of centimeters, but you're coupling to an app based in meters, the fluxes will
//  // be very different from one another.
//  if (moose_flux && (std::abs(nek_flux * nek_flux_print_mult - moose_flux) / moose_flux) > 0.25)
//    mooseDoOnce(mooseWarning("nekRS flux differs from MOOSE flux by more than 25\%! "
//      "This could indicate that your geometries do not line up properly."));
//
//  if (!successful_normalization)
//    mooseError("Flux normalization process failed! nekRS integrated flux: ", normalized_nek_flux,
//      " MOOSE integrated flux: ", moose_flux, ".\n\nThis may happen if the nekRS mesh "
//      "is very different from that used in the App sending heat flux to nekRS and the "
//      "nearest node transfer is only picking up zero values in the coupled App.");
//}

//void
//SAMNekRSProblem::getBoundaryTemperatureFromNek()
//{
//  _console << "Extracting NekRS temperature from boundary " << Moose::stringify(*_boundary) << std::endl;
//
//  // Get the temperature solution from nekRS. Note that nekRS performs a global communication
//  // here such that each nekRS process has all the boundary temperature information. That is,
//  // every process knows the full boundary temperature solution
//  nekrs::boundarySolution(_nek_mesh->order(), _needs_interpolation, field::temperature, _T);
//}

//void
//SAMNekRSProblem::addExternalVariables()
//{
//  NekRSProblemBase::addExternalVariables();
//  auto var_params = getExternalVariableParameters();
//
//  addAuxVariable("MooseVariable", "temp", var_params);
//  _temp_var = _aux->getFieldVariable<Real>(0, "temp").number();
//
//  if (_boundary)
//  {
//    // Likewise, because this flux represents the reconstruction of the flux variable
//    // that becomes a boundary condition in the nekRS model, we set the order to match
//    // the desired order of the surface. Note that this does _not_ imply anything
//    // about the order of the surface flux in the MOOSE app (such as BISON) coupled
//    // to nekRS. This is just the variable that nekRS reads from - MOOSE's transfer
//    // classes handle any additional interpolations needed from the flux on the
//    // sending app (such as BISON) into 'avg_flux'.
//    addAuxVariable("MooseVariable", "avg_flux", var_params);
//    _avg_flux_var = _aux->getFieldVariable<Real>(0, "avg_flux").number();
//
//    // add the postprocessor that receives the flux integral for normalization
//    auto pp_params = _factory.getValidParams("Receiver");
//    addPostprocessor("Receiver", "flux_integral", pp_params);
//  }
//
//  if (_volume && _has_heat_source)
//  {
//    addAuxVariable("MooseVariable", "heat_source", var_params);
//    _heat_source_var = _aux->getFieldVariable<Real>(0, "heat_source").number();
//
//    // add the postprocessor that receives the source integral for normalization
//    auto pp_params = _factory.getValidParams("Receiver");
//    addPostprocessor("Receiver", "source_integral", pp_params);
//  }
//
//  // add the displacement aux variables from the solid mechanics solver; these will
//  // be needed regardless of whether the displacement is boundary- or volume-based
//  if (_moving_mesh)
//  {
//    addAuxVariable("MooseVariable", "disp_x", var_params);
//    _disp_x_var = _aux->getFieldVariable<Real>(0, "disp_x").number();
//
//    addAuxVariable("MooseVariable", "disp_y", var_params);
//    _disp_y_var = _aux->getFieldVariable<Real>(0, "disp_y").number();
//
//    addAuxVariable("MooseVariable", "disp_z", var_params);
//    _disp_z_var = _aux->getFieldVariable<Real>(0, "disp_z").number();
//  }
//
//  if (_minimize_transfers_in)
//  {
//    auto pp_params = _factory.getValidParams("Receiver");
//    pp_params.set<std::vector<OutputName>>("outputs") = {"none"};
//    addPostprocessor("Receiver", "transfer_in", pp_params);
//  }
//}
