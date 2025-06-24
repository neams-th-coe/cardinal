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

#ifdef ENABLE_OPENMC_COUPLING

#include "FDTallyGradAux.h"

#include "CardinalEnums.h"

registerMooseObject("CardinalApp", FDTallyGradAux);

InputParameters
FDTallyGradAux::validParams()
{
  auto params = OpenMCVectorAuxKernel::validParams();
  params.addClassDescription(
      "An auxkernel which approximates tally gradients at element centroids using "
      "forward finite differences.");
  params.addRequiredParam<MooseEnum>(
      "score",
      getSingleTallyScoreEnum(),
      "The tally score this auxkernel should approximate the gradient of.");
  params.addParam<unsigned int>("ext_filter_bin",
                                0,
                                "The filter bin for the case where any filters are added to this "
                                "tally with [Filters] (bin indices start at 0). This parameter "
                                "should be specified if you wish to extract the relative error "
                                "of a different non-spatial tally bin.");

  params.addRelationshipManager("ElementSideNeighborLayers",
                                Moose::RelationshipManagerType::ALGEBRAIC |
                                    Moose::RelationshipManagerType::GEOMETRIC,
                                [](const InputParameters &, InputParameters & rm_params)
                                { rm_params.set<unsigned short>("layers") = 2; });

  return params;
}

FDTallyGradAux::FDTallyGradAux(const InputParameters & parameters)
  : OpenMCVectorAuxKernel(parameters),
    _bin_index(getParam<unsigned int>("ext_filter_bin")),
    _sum_y_y_t(RealEigenMatrix::Zero(3, 3)),
    _sum_y_du_dy(RealEigenVector::Zero(3))
{
  if (_var.feType() != FEType(libMesh::CONSTANT, libMesh::MONOMIAL_VEC))
    paramError("variable",
               "FDTallyGradAux only supports CONSTANT MONOMIAL_VEC shape functions. Please "
               "ensure that 'variable' is of type MONOMIAL_VEC and order CONSTANT.");

  std::string score = getParam<MooseEnum>("score");
  std::replace(score.begin(), score.end(), '_', '-');

  // Error check and fetch the tally score.
  if (!_openmc_problem->hasScore(score))
    paramError("score",
               "The problem does not contain any score named " +
                   std::string(getParam<MooseEnum>("score")) +
                   "! Please "
                   "ensure that one of your [Tallies] is scoring the requested score.");

  auto score_vars = _openmc_problem->getTallyScoreVariables(score, _tid);
  auto score_bins = _openmc_problem->getTallyScoreVariableValues(score, _tid);
  auto neighbor_score_bins = _openmc_problem->getTallyScoreNeighborVariableValues(score, _tid);

  if (_bin_index >= score_bins.size())
    paramError("ext_filter_bin",
               "The external filter bin provided is invalid for the number of "
               "external filter bins (" +
                   std::to_string(score_bins.size()) +
                   ") "
                   "applied to " +
                   std::string(getParam<MooseEnum>("score")) + "!");

  if (score_vars[_bin_index]->feType() != FEType(libMesh::CONSTANT, libMesh::MONOMIAL))
    paramError(
        "score",
        "FDTallyGradAux only supports CONSTANT MONOMIAL shape functions for tally variables.");

  _tally_val = score_bins[_bin_index];
  _tally_neighbor_val = neighbor_score_bins[_bin_index];
}

void
FDTallyGradAux::compute()
{
  auto elem_c = _current_elem->true_centroid();
  for (unsigned int side = 0; side < _current_elem->n_sides(); side++)
  {
    const Elem * neighbor = _current_elem->neighbor_ptr(side);

    // If the neighbor is null, the current side is a boundary and we can skip
    // it. Geometric ghosting ensures that element neighbors are on the
    // processors that need them during compute().
    if (!neighbor)
      continue;

    if (!neighbor->active())
      continue;

    // Can skip the gradient computation for this side if the neighbor isn't on
    // this block.
    if (!hasBlocks(neighbor->subdomain_id()))
      continue;

    _openmc_problem->reinitNeighbor(_current_elem, side, _tid);

    // Fetch the vector pointing from the current element's centroid to the
    // neighbor's centroid (y').
    auto y_prime = neighbor->true_centroid() - elem_c;
    RealEigenVector y_prime_eig(3);
    y_prime_eig << y_prime(0), y_prime(1), y_prime(2);

    // Compute du/dy along the direction pointing towards the neighbor's centroid.
    // Add to the b vector.
    _sum_y_du_dy +=
        y_prime_eig * ((*_tally_neighbor_val)[0] - (*_tally_val)[0]) / y_prime.norm_sq();
    // Compute the outer product between y' and y'.T.
    // Add to the A matrix.
    _sum_y_y_t += y_prime_eig * y_prime_eig.transpose();
  }

  // Solve for an estimate of the tally gradient at the current element's centroid.
  RealEigenVector val = _sum_y_y_t.fullPivLu().solve(_sum_y_du_dy);
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  _var.setDofValue(val(0), 0);
  _var.setDofValue(val(1), 1);
  _var.setDofValue(val(2), 2);

  _sum_y_y_t = RealEigenMatrix::Zero(3, 3);
  _sum_y_du_dy = RealEigenVector::Zero(3);
}

#endif
