#pragma once

#include "ExternalProblem.h"
#include "NekTimeStepper.h"
#include "NekRSMesh.h"
#include "Transient.h"

#include <memory>

class NekRSProblemBase;

template<>
InputParameters validParams<NekRSProblemBase>();

/**
 * Base class for all MOOSE wrappings of NekRS. This class is used to facilitate
 * common NekRS functionality shared by different types of wrappings, such as:
 * - ensuring correct usage of NekTimeStepper and NekRSMesh
 * - specifying nondimensional scales
 * - running a single time step of NekRS
 */
class NekRSProblemBase : public ExternalProblem
{
public:
  NekRSProblemBase(const InputParameters & params);

  ~NekRSProblemBase();

  /**
   * Optional entry point called in externalSolve() where we can adjust
   * the NekRS solution (such as to clip out high/low values of temperature).
   * This is called after runStep(), but before writing anything to the Nek5000
   * backend or to output files.
   */
  virtual void adjustNekSolution() {}

  /**
   * \brief Whether nekRS should write an output file for the current time step
   *
   * A nekRS output file (suffix .f000xx) is written if the time step is an integer
   * multiple of the output writing interval or if the time step is the last time step.
   * \return whether to write a nekRS output file
   **/
  virtual bool isOutputStep() const;

  virtual void initialSetup() override;

  virtual void externalSolve() override;

  virtual bool converged() override { return true; }

protected:
  /// Whether the nekRS solution is performed in nondimensional scales
  const bool & _nondimensional;

  //@{
  /**
   * \brief Reference scales for nekRS solution in non-dimensional form.
   *
   * If nekRS is solving in non-dimensional form, this means that the nekRS solution
   * is performed for:
   *
   * - nondimensional temperature \f$T^\dagger\f$, defined as
   *   \f$T^\dagger=\frac{T-T_{ref}}{\Delta T_{ref}}\f$.
   *   The 'T_ref' and 'dT_ref' variables here represent these scales.
   *
   * - nondimensional velocity \f$U^\dagger=\frac{u}{U_{ref}}\f$. The 'U_ref' variable here
   *   represents this velocity scale.
   *
   * - nondimensional pressure \f$P^dagger=\frac{P}{\rho_{0}U_{ref}^2}\f$. The 'rho_0'
   *   variable here represents this density parameter.
   *
   * In terms of heat flux boundary conditions, the entire energy conservation equation
   * in nekRS is multiplied by \f$\frac{L_{ref}}{\rho_{0}C_{p,0}U_{ref}\Delta T_{ref}}\f$
   * in order to clear the coefficient on the convective. Therefore, the heat source in
   * nekRS is expressed in nondimensional form as \f$q^\dagger=\frac{q}{\rho_{0}C_{p,0}U_{ref}\Delta T_{ref}}\f$.
   * Here, 'Cp_0' is the specific heat capacity scale.
   *
   * Unfortunately, there is no systematic way to get these reference
   * scales from nekRS, so we need to inform MOOSE of any possible scaling so that we
   * can appropriately scale the nekRS temperature to the dimensional form that is usually
   * expected in MOOSE. Therefore, these scales just need to match what is used in the nekRS
   * input files used to specify boundary conditions. Conversion between dimensional MOOSE
   * heat flux to dimensionless nekRS heat flux is performed automatically, and does not require
   * any special treatment in the nekRS scalarNeumannBC kernel.
   */
  const Real & _U_ref;

  const Real & _T_ref;

  const Real & _dT_ref;

  const Real & _L_ref;

  const Real & _rho_0;

  const Real & _Cp_0;
  //@}

  /// Start time of the simulation based on NekRS's .par file
  double _start_time;

  /**
   * Underlying mesh object on which NekRS exchanges fields with MOOSE
   * or extracts NekRS's solution for I/O features
   */
  const NekRSMesh * _nek_mesh;

  /// The time stepper used for selection of time step size
  NekTimeStepper * _timestepper = nullptr;

  /// Underlying executioner
  Transient * _transient_executioner = nullptr;
};
