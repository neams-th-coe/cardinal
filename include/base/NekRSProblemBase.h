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

  virtual void addExternalVariables() override;

  virtual void syncSolutions(ExternalProblem::Direction direction) override;

  /**
   * Whether the solve is in nondimensional form
   * @return whether solve is in nondimensional form
   */
  virtual bool nondimensional() const { return _nondimensional; }

  /**
   * Whether the mesh is moving
   * @return whether the mesh is moving
   */
  virtual bool movingMesh() const = 0;

protected:
  /**
   * Fill an outgoing auxiliary variable field with nekRS solution data
   * \param[in] var_number auxiliary variable number
   * \param[in] value nekRS solution data to fill the variable with
   */
  virtual void fillAuxVariable(const unsigned int var_number, const double * value);

  /**
   * Extract user-specified parts of the NekRS CFD solution onto the mesh mirror
   */
  virtual void extractOutputs();

  /**
   * Get the parameters for the external variables to be added
   * @return external variable parameters
   */
  virtual InputParameters getExternalVariableParameters();

  /**
   * Add a variable to represent the temperature of the NekRS solution.
   * We allow this to be an entry point for derived classes because they
   * might already be adding a temperature variable for coupling purposes.
   */
  virtual void addTemperatureVariable();

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

  /// Number of surface elements in the data transfer mesh, across all processes
  int _n_surface_elems;

  /// Number of vertices per surface element of the transfer mesh
  int _n_vertices_per_surface;

  /// Number of volume elements in the data transfer mesh, across all processes
  int _n_volume_elems;

  /// Number of vertices per volume element of the transfer mesh
  int _n_vertices_per_volume;

  /// Number of elements in the data transfer mesh, which depends on whether boundary/volume coupling
  int _n_elems;

  /// Number of vertices per element in the data transfer mesh, which depends on whether boundary/volume coupling
  int _n_vertices_per_elem;

  /// Boundary IDs through which to couple nekRS and MOOSE
  const std::vector<int> * _boundary = nullptr;

  /// Whether the mesh contains volume-based coupling
  bool _volume;

  /// Start time of the simulation based on NekRS's .par file
  double _start_time;

  /// Whether the most recent time step was an output file writing step
  bool _is_output_step;

  /**
   * Underlying mesh object on which NekRS exchanges fields with MOOSE
   * or extracts NekRS's solution for I/O features
   */
  const NekRSMesh * _nek_mesh;

  /// The time stepper used for selection of time step size
  NekTimeStepper * _timestepper = nullptr;

  /// Underlying executioner
  Transient * _transient_executioner = nullptr;

  /**
   * Whether an interpolation needs to be performed on the nekRS temperature solution, or
   * if we can just grab the solution at specified points
   */
  bool _needs_interpolation;

  /// NekRS solution fields to output to the mesh mirror
  const MultiMooseEnum * _outputs = nullptr;

  /// Names of external variables when extracting the NekRS solution
  std::vector<std::string> _var_names;

  /// Numeric identifiers for the external variables
  std::vector<unsigned int> _external_vars;

  /// Descriptive string for the variables extracted from NekRS
  std::string _var_string;

  /// Scratch space to place external NekRS fields before writing into auxiliary variables
  double * _external_data = nullptr;

  /// Number of points for interpolated fields on the MOOSE mesh
  int _n_points;
};
