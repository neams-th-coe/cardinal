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

#include "CardinalProblem.h"
#include "NekInterface.h"
#include "NekTimeStepper.h"
#include "NekScalarValue.h"
#include "NekRSMesh.h"
#include "Transient.h"
#include "TimeStepper.h"

#include <memory>
#include "inipp.hpp"
#include "nekrs.hpp"

/**
 * Base class for all MOOSE wrappings of NekRS. This class is used to facilitate
 * common NekRS functionality shared by different types of wrappings, such as:
 * - ensuring correct usage of NekTimeStepper and NekRSMesh
 * - specifying nondimensional scales
 * - running a single time step of NekRS
 */
class NekRSProblemBase : public CardinalProblem
{
public:
  NekRSProblemBase(const InputParameters & params);

  static InputParameters validParams();

  ~NekRSProblemBase();

  /**
   * Whether a data transfer to/from Nek is occurring
   * @param[in] direction direction of data transfer
   * @return whether a data transfer to Nek is about to occur
   */
  bool isDataTransferHappening(ExternalProblem::Direction direction);

  /**
   * Get the minimum scratch space required for coupling; this space is guaranteed to be filled
   * with data incoming from MOOSE, like heat flux boundary conditions or mesh displacements
   * @return minimum scratch space reserved for coupling
   */
  unsigned int minimumScratchSizeForCoupling() const { return _minimum_scratch_size_for_coupling; }

  /**
   * Get the index of the first slot in the usrwrk that Cardinal is managing
   * @return index of first slot
   */
  unsigned int firstReservedUsrwrkSlot() const { return _first_reserved_usrwrk_slot; }

  /// Send values from NekScalarValue userobjects to NekRS
  void sendScalarValuesToNek();

  /// Setup to fetch NekScalarValue user objects for sending scalar values into NekRS scratch space
  void getNekScalarValueUserObjects();

  /**
   * Map nodal points on a MOOSE face element to the GLL points on a Nek face element.
   * @param[in] e MOOSE element ID
   * @param[in] var_num variable index to fetch MOOSE data from
   * @param[in] multiplier multiplier to apply to the MOOSE data before sending to Nek
   * @param[out] outgoing_data data represented on Nek's GLL points, ready to be applied in Nek
   */
  void mapFaceDataToNekFace(const unsigned int & e, const unsigned int & var_num,
    const Real & multiplier, double ** outgoing_data);

  /**
   * Map nodal points on a MOOSE volume element to the GLL points on a Nek volume element.
   * @param[in] e MOOSE element ID
   * @param[in] var_num variable index to fetch MOOSE data from
   * @param[in] multiplier multiplier to apply to the MOOSE data before sending to Nek
   * @param[out] outgoing_data data represented on Nek's GLL points, ready to be applied in Nek
   */
  void mapVolumeDataToNekVolume(const unsigned int & e, const unsigned int & var_num,
    const Real & multiplier, double ** outgoing_data);

  /**
   * \brief Map nodal points on a MOOSE face element to the GLL points on a Nek volume element.
   *
   * This function is to be used when MOOSE variables are defined over the entire volume
   * (maybe the MOOSE transfer only sent meaningful values to the coupling boundaries), so we
   * need to do a volume interpolation of the incoming MOOSE data into nrs->usrwrk, rather
   * than a face interpolation. This could be optimized in the future to truly only just write
   * the boundary values into the nekRS scratch space rather than the volume values, but it
   * looks right now that our biggest expense occurs in the MOOSE transfer system, not these
   * transfers internally to nekRS.
   *
   * @param[in] e MOOSE element ID
   * @param[in] var_num variable index to fetch MOOSE data from
   * @param[in] multiplier multiplier to apply to the MOOSE data before sending to Nek
   * @param[out] outgoing_data data represented on Nek's GLL points, ready to be applied in Nek
   */
  void mapFaceDataToNekVolume(const unsigned int & e, const unsigned int & var_num,
    const Real & multiplier, double ** outgoing_data);

  /**
   * Write NekRS solution field file
   * @param[in] time solution time in NekRS (if NekRS is non-dimensional, this will be non-dimensional)
   * @param[in] step time step index
   */
  void writeFieldFile(const Real & time, const int & step) const;

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

  virtual bool converged(unsigned int) override { return true; }

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
  virtual const bool hasMovingNekMesh() const { return false; }

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

  /**
   * Get the number of usrwrk slots allocated
   * @return number of allocated usrwrk slots
   */
  unsigned int nUsrWrkSlots() const { return _n_usrwrk_slots; }

protected:
  /**
   * Copy the data sent from MOOSE->Nek from host to device.
   */
  void copyScratchToDevice();

  /**
   * Interpolate the MOOSE mesh mirror solution onto the NekRS boundary mesh (mirror -> re2)
   * @param[in] incoming_moose_value MOOSE face values
   * @param[out] outgoing_nek_value interpolated MOOSE face values onto the NekRS boundary mesh
   */
  void interpolateBoundarySolutionToNek(double * incoming_moose_value,
                                        double * outgoing_nek_value);

  /**
   * Interpolate the MOOSE mesh mirror solution onto the NekRS volume mesh (mirror -> re2)
   * @param[in] elem_id element ID
   * @param[in] incoming_moose_value MOOSE face values
   * @param[out] outgoing_nek_value interpolated MOOSE face values onto the NekRS volume mesh
   */
  void interpolateVolumeSolutionToNek(const int elem_id, double * incoming_moose_value,
                                      double * outgoing_nek_value);

  /**
   * Write into the NekRS solution space for coupling volumes; for setting a mesh position in terms of a
   * displacement, we need to add the displacement to the initial mesh coordinates. For
   * this, the 'add' parameter lets you pass in a vector of values (in NekRS's mesh order,
   * i.e. the re2 order) to add.
   * @param[in] elem_id element ID
   * @param[in] field field to write
   * @param[in] T solution values to write for the field for the given element
   * @param[in] add optional vector of values to add to each value set on the NekRS end
   */
  void writeVolumeSolution(const int elem_id,
                           const field::NekWriteEnum & field,
                           double * T,
                           const std::vector<double> * add = nullptr);

  /**
   * Write into the NekRS solution space for coupling boundaries; for setting a mesh position in terms of a
   * displacement, we need to add the displacement to the initial mesh coordinates.
   * @param[in] elem_id element ID
   * @param[in] field field to write
   * @param[in] T solution values to write for the field for the given element
   */
  void writeBoundarySolution(const int elem_id, const field::NekWriteEnum & field, double * T);

  /**
   * Interpolate the NekRS volume solution onto the volume MOOSE mesh mirror (re2 -> mirror)
   * @param[in] f field to interpolate
   * @param[out] T interpolated volume value
   */
  void volumeSolution(const field::NekFieldEnum & f, double * T);

  /**
   * Interpolate the NekRS boundary solution onto the boundary MOOSE mesh mirror (re2 -> mirror)
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

  std::unique_ptr<NumericVector<Number>> _serialized_solution;

  /**
   * Get a three-character prefix for use in writing output files for repeated
   * Nek sibling apps.
   * @param[in] number multi-app number
   */
  std::string fieldFilePrefix(const int & number) const;

  /// NekRS casename
  const std::string & _casename;

  /**
   * \brief Whether the nekRS solution is performed in nondimensional scales
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
   * nekRS is expressed in nondimensional form as \f$q^\dagger=\frac{q}{\rho_{0}C_{p,0}U_{ref}\Delta
   * T_{ref}}\f$. Here, 'Cp_0' is the specific heat capacity scale.
   *
   * Unfortunately, there is no systematic way to get these reference
   * scales from nekRS, so we need to inform MOOSE of any possible scaling so that we
   * can appropriately scale the nekRS temperature to the dimensional form that is usually
   * expected in MOOSE. Therefore, these scales just need to match what is used in the nekRS
   * input files used to specify boundary conditions. Conversion between dimensional MOOSE
   * heat flux to dimensionless nekRS heat flux is performed automatically, and does not require
   * any special treatment in the nekRS scalarNeumannBC kernel.
   */
  const bool & _nondimensional;

  /// Reference characteristic velocity
  const Real & _U_ref;

  /// Reference characteristic temperature
  const Real & _T_ref;

  /// Reference characteristic temperature delta
  const Real & _dT_ref;

  /// Reference characteristic length
  const Real & _L_ref;

  /// Reference density
  const Real & _rho_0;

  /// Reference isobaric specific heat capacity
  const Real & _Cp_0;

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
   * Number of slices/slots to allocate in nrs->usrwrk to hold fields
   * for coupling (i.e. data going into NekRS, written by Cardinal), or
   * used for custom user actions, but not for coupling. By default, we just
   * allocate 7 slots (no inherent reason, just a fairly big amount). For
   * memory-limited cases, you can reduce this number to just the bare
   * minimum necessary for your use case.
   */
  unsigned int _n_usrwrk_slots;

  /// For constant synchronization intervals, the desired frequency (in units of Nek time steps)
  const unsigned int & _constant_interval;

  /// Whether to skip writing a field file on NekRS's last time steo
  const bool & _skip_final_field_file;

  /// First slice in usrwrk for Cardinal to read/write data from
  const unsigned int & _first_reserved_usrwrk_slot;

  /// Number of surface elements in the data transfer mesh, across all processes
  int _n_surface_elems;

  /// Number of vertices per surface element of the transfer mesh
  int _n_vertices_per_surface;

  /// Number of volume elements in the data transfer mesh, across all processes
  int _n_volume_elems;

  /// Number of vertices per volume element of the transfer mesh
  int _n_vertices_per_volume;

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
  NekRSMesh * _nek_mesh;

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

  /// Minimum allowable scratch space size, strictly what is needed by Cardinal for coupling
  unsigned int _minimum_scratch_size_for_coupling;

  /// For the MOOSE mesh, the number of quadrature points in each coordinate direction
  int _moose_Nq;

  /// Slots in the nrs->o_usrwrk array to write to a field file
  const std::vector<unsigned int> * _usrwrk_output = nullptr;

  /// Filename prefix to use for naming the field files containing the nrs->o_usrwrk array slots
  const std::vector<std::string> * _usrwrk_output_prefix = nullptr;

  /// Sum of the elapsed time in NekRS solves
  double _elapsedStepSum;

  /// Sum of the total elapsed time in NekRS solves
  double _elapsedTime;

  /// Minimum step solve time
  double _tSolveStepMin;

  /// Maximum step solve time
  double _tSolveStepMax;

  /**
   * \brief When to synchronize the NekRS solution with the mesh mirror
   *
   * This parameter determines when to synchronize the NekRS solution with the mesh
   * mirror - this entails:
   *
   *  - Mapping from the NekRS spectral element mesh to the finite element mesh mirror,
   *    to extract information from NekRS and make it available to MOOSE
   *  - Mapping from the finite element mesh mirror into the NekRS spectral element mesh,
   *    to send information from MOOSE into NekRS
   *
   * Several options are available:
   *  - 'constant' will simply keep the NekRS solution and the mesh mirror entirely
   *    consistent with one another on a given constant frequency of time steps. By
   *    default, the 'constant_interval' is 1, so that NekRS and MOOSE communicate
   *    with each other on every single time step
   *
   *  - 'parent_app' will only send data between NekRS and a parent application
   *    when (1) the main application has just sent "new" information to NekRS, and
   *    when (2) the main application is just about to run a new time step (with
   *    updated BCs/source terms from NekRS).
   *
   *    nekRS is often subcycled relative to the application controlling it -
   *    that is, nekRS may be run with a time step 10x smaller than a conduction MOOSE app.
   *    If 'interpolate_transfers = false'
   *    in the master application, then the data going into nekRS is fixed for each
   *    of the subcycled time steps it takes, so these extra data transfers are
   *    completely unnecssary. This flag indicates that the information sent from MOOSE
   *    to NekRS should only be updated if the data from MOOSE is "new", and likewise
   *    whether the NekRS solution should only be interpolated to the mesh mirror once
   *    MOOSE is actually "ready" to solve a time step using it.
   *
   *    NOTE: if 'interpolate_transfers = true' in the master application, then the data
   *    coming into nekRS is _unique_ on each subcycled time step, so setting this to
   *    true will in effect override `interpolate_transfers` to be false. For the best
   *    performance, you should set `interpolate_transfers` to false so that you don't
   *    even bother computing the interpolated data, since it's not used if this parameter
   *    is set to true.
   */
  synchronization::SynchronizationEnum _synchronization_interval;

  /// quantities to write to nrs->usrwrk (and the order to write them)
  std::vector<std::string> _usrwrk_indices;

  /// Userobjects containing stochastic input data
  std::vector<NekScalarValue *> _nek_uos;

  /// flag to indicate whether this is the first pass to serialize the solution
  static bool _first;

  /// Counter for any potential NekScalarValue objects in the input file
  unsigned int _scratch_counter;

  /// Number of scratch space slots used for NekScalarValue usage
  unsigned int _n_uo_slots;
};
