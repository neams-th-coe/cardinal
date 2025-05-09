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

#pragma once

#include "CardinalAction.h"

#include "CardinalEnums.h"
#include "EnergyBinBase.h"

#include "TallyBase.h"

class OpenMCCellAverageProblem;

/**
 * This action automates the generation of multi-group cross sections
 * for an OpenMC case wrapped in Cardinal. These cross sections
 * are generated with Cardinal's mapped tally system.
 */
class SetupMGXSAction : public CardinalAction, public EnergyBinBase
{
public:
  static InputParameters validParams();

  SetupMGXSAction(const InputParameters & parameters);

  virtual void act() override;

  bool addingCellTallies() const { return _t_type == tally::cell; }

protected:
  /**
   * A function to cast the FEProblem to an OpenMCCellAverageProblem
   * @return a pointer to the OpenMCCellAverageProblem
   */
  OpenMCCellAverageProblem * openmcProblem();

  /// A function to add the filters necessary for MGXS generation.
  void addFilters();

  /// A function to add the tallies necessary for MGXS generation.
  void addTallies();

  /// A function which adds auxvariables that store the computed MGXS.
  void addAuxVars();

  /// A function which adds auxkernels that compute the MGXS.
  void addAuxKernels();

  /// Modify outputs to hide tally variables used to generate MGXS in outputs.
  void modifyOutputs();

  /// The tally type to add.
  const tally::TallyTypeEnum _t_type;

  /// The particle to filter for when generating cross sections.
  const MooseEnum _particle;

  /// The estimator to use when generating multi-group cross sections.
  MooseEnum _estimator;

  /// Whether or not group-wise scattering cross sections for the scattering matrix
  /// should be computed.
  const bool _add_scattering;

  /// The Legendre order.
  const unsigned int _l_order;

  /// Whether a P0 transport correction should be applied to isotropic scattering cross sections.
  bool _transport_correction;

  /// Whether or not group-wise nu-fission MGXS should be computed.
  const bool _add_fission;

  /// Whether or not group-wise kappa-fission values should be computed.
  const bool _add_kappa_fission;

  /// Whether or not group-wise inverse velocity values should be computed.
  const bool _add_inv_vel;

  /// Whether or not group-wise diffusion coefficients should be computed.
  const bool _add_diffusion;

  /// The value the diffusion coefficient should take in a void region.
  const Real & _void_diff;

  /// Whether or not group-wise absorption MGXS should be computed.
  const bool _add_absorption;

  /// Whether tally variables should be hidden.
  const bool _hide_tally_vars;

  /// A list of tallies added by this action.
  std::vector<const TallyBase *> _mgxs_tallies;

  /// Whether or not scattering reaction rates need to be at least order P1.
  bool _need_p1_scatter;
};
