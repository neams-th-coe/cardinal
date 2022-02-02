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

#include "ExternalProblem.h"
#include "NekTimeStepper.h"
#include "NekRSMesh.h"
#include "Transient.h"

#include <memory>

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

  static InputParameters validParams();

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

  /**
   * Whether data should be synchronized in to nekRS
   * \return whether inward data synchronization should occur
   */
  virtual bool synchronizeIn();

  /**
   * Whether data should be synchronized out of nekRS
   * \return whether outward data synchronization should occur
   */
  virtual bool synchronizeOut();

  /**
   * Get the characteristic length
   * @return characteristic length
   */
  double L_ref() const { return _L_ref; }

protected:
  /**
   * Write into the NekRS solution space
   * @param[in] elem_id element ID
   * @param[in] field field to write
   * @param[in] T solution values to write for the field for the given element
   */
  void writeVolumeSolution(const int elem_id, const field::NekWriteEnum & field, double * T);

  /**
   * Interpolate the nekRS volume solution onto the volume data transfer mesh
   * @param[in] f field to interpolate
   * @param[out] T interpolated volume value
   */
  void volumeSolution(const field::NekFieldEnum & f, double * T);

  /**
   * Interpolate the nekRS boundary solution onto the boundary data transfer mesh
   * @param[in] f field to interpolate
   * @param[out] T interpolated boundary value
   */
  void boundarySolution(const field::NekFieldEnum & f, double * T);

  /// Initialize interpolation matrices for transfers in/out of nekRS
  void initializeInterpolationMatrices();

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

  /**
   * Get a three-character prefix for use in writing output files for repeated
   * Nek sibling apps.
   * @param[in] number multi-app number
   */
  std::string fieldFilePrefix(const int & number) const;

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

  /**
   * Whether to disable output file writing by NekRS and replace it by output
   * file writing in Cardinal. Suppose the case name is 'channel'. If this parameter
   * is false, then NekRS will write output files as usual, with names like
   * channel0.f00001 for write step 1, channel0.f00002 for write step 2, and so on.
   * If true, then NekRS itself does not output any files like this, and instead
   * output files are written with names a01channel0.f00001, a01channel0.f00002 (for
   * first Nek app), a02channel0.f00001, a02channel0.f00002 (for second Nek app),
   * and so on. This feature should only be used when running repeated Nek sub
   * apps so that the output from each app is retained. Otherwise, if running N
   * repeated Nek sub apps, only a single output file is obtained because each app
   * overwrites the output files of the other apps in the order that the apps
   * reach the nekrs::outfld function.
   */
  const bool & _write_fld_files;

  /// Whether to turn off all field file writing
  const bool & _disable_fld_file_output;

  /**
   * \brief Whether to only send data to nekRS on the multiapp synchronization steps
   *
   * nekRS is often subcycled relative to the application controlling it -
   * that is, nekRS may be run with a time step 10x smaller than a conduction MOOSE app.
   * The NekRSProblemBase interface in MOOSE, however, currently synchronizes (i.e. sends
   * data to/from nekRS) on _every_ nekRS time step. If 'interpolate_transfers = false'
   * in the master application, then the data going into nekRS is fixed for each
   * of the subcycled time steps it takes, so these extra data transfers are
   * completely unnecssary. This flag indicates whether a somewhat hacky postprocessor
   * data transfer should be used for nekRS to figure out whether each time step is
   * the first immediately following a transfer from it's master application, and
   * then only send data into nekRS's arrays if the data from MOOSE is new.
   *
   * NOTE: if 'interpolate_transfers = true' in the master application, then the data
   * coming into nekRS is _unique_ on each subcycled time step, so setting this to
   * true will in effect override `interpolate_transfers` to be false. For the best
   * performance, you should set `interpolate_transfers` to false so that you don't
   * even bother computing the interpolated data, since it's not used if this parameter
   * is set to true.
   */
  const bool & _minimize_transfers_in;

  /**
   * \brief Whether to only get data from nekRS on the multiapp synchronization steps
   *
   * nekRS is often subcycled relative to the application controlling it -
   * that is, nekRS may be run with a time step 10x smaller than a conduction MOOSE app.
   * The NekRSProblemBase interface in MOOSE, however, currently synchronizes (i.e. sends
   * data to/from nekRS) on _every_ nekRS time step. If 'interpolate_transfers = false'
   * in the master application, then the data coming from nekRS is fixed
   * (from the master's perspective) for each
   * of the subcycled time steps it takes, so these extra data transfers are
   * completely unnecssary. This flag indicates whether the coincidence of the
   * simulation time with the 'target_time' should be used to only transfer data out
   * of nekRS on the synchronization steps when that data will actually be used.
   *
   * NOTE: if 'interpolate_transfers = true' in the master application, then the data
   * sent from nekRS is _unique_ on each subcycled time step, so setting this to
   * true will in effect override `interpolate_transfers` to be false. For the best
   * performance, you should set `interpolate_transfers` to false so that you don't
   * even bother computing the interpolated data, since it's not used if this parameter
   * is set to true.
   */
  const bool & _minimize_transfers_out;

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

  /// Prefix to apply to field files when writing automatically from Cardinal
  std::string _prefix;

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

  /// Postprocessor containing the signal of when a synchronization has occurred
  const PostprocessorValue * _transfer_in = nullptr;

  /// Vandermonde interpolation matrix (for outgoing transfers)
  double * _interpolation_outgoing = nullptr;

  /// Vandermonde interpolation matrix (for incoming transfers)
  double * _interpolation_incoming = nullptr;
};
