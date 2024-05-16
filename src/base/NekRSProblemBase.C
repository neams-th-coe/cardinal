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

#include "NekRSProblemBase.h"
#include "CardinalUtils.h"

#include "nekrs.hpp"
#include "nekInterface/nekInterfaceAdapter.hpp"

bool NekRSProblemBase::_first = true;

InputParameters
NekRSProblemBase::validParams()
{
  InputParameters params = CardinalProblem::validParams();
  params.addRequiredParam<std::string>(
      "casename",
      "Case name for the NekRS input files; "
      "this is <case> in <case>.par, <case>.udf, <case>.oudf, and <case>.re2.");

  params.addParam<unsigned int>("n_usrwrk_slots", 7,
    "Number of slots to allocate in nrs->usrwrk to hold fields either related to coupling "
    "(which will be populated by Cardinal), or other custom usages, such as a distance-to-wall calculation");
  params.addParam<unsigned int>(
      "first_reserved_usrwrk_slot",
      0,
      "Slice (zero-indexed) in nrs->usrwrk where Cardinal will begin reading/writing data; this "
      "can be used to shift the usrwrk slots reserved by Cardinal, so that you can use earlier "
      "slices for custom purposes");

  params.addParam<bool>("nondimensional", false, "Whether NekRS is solved in non-dimensional form");
  params.addRangeCheckedParam<Real>(
      "U_ref", 1.0, "U_ref > 0.0", "Reference velocity value for non-dimensional solution");
  params.addRangeCheckedParam<Real>(
      "T_ref", 0.0, "T_ref >= 0.0", "Reference temperature value for non-dimensional solution");
  params.addRangeCheckedParam<Real>(
      "dT_ref",
      1.0,
      "dT_ref > 0.0",
      "Reference temperature range value for non-dimensional solution");
  params.addRangeCheckedParam<Real>(
      "L_ref", 1.0, "L_ref > 0.0", "Reference length scale value for non-dimensional solution");
  params.addRangeCheckedParam<Real>(
      "rho_0", 1.0, "rho_0 > 0.0", "Density parameter value for non-dimensional solution");
  params.addRangeCheckedParam<Real>(
      "Cp_0", 1.0, "Cp_0 > 0.0", "Heat capacity parameter value for non-dimensional solution");

  MultiMooseEnum nek_outputs("temperature pressure velocity scalar01 scalar02 scalar03");
  params.addParam<MultiMooseEnum>(
      "output", nek_outputs, "Field(s) to output from NekRS onto the mesh mirror");

  params.addParam<std::vector<unsigned int>>("usrwrk_output",
    "Usrwrk slot(s) to output to NekRS field files; this can be used for viewing the quantities "
    "passed from MOOSE to NekRS after interpolation to the CFD mesh. Can also be used for any slots "
    "in usrwrk that are written by the user, but unused for coupling.");
  params.addParam<std::vector<std::string>>("usrwrk_output_prefix",
    "String prefix to use for naming the field file(s); "
    "only the first three characters are used in the name based on limitations in NekRS");

  params.addParam<bool>(
      "write_fld_files",
      false,
      "Whether to write NekRS field file output "
      "from Cardinal. If true, this will disable any output writing by NekRS itself, and "
      "instead produce output files with names a01...a99pin, b01...b99pin, etc.");
  params.addParam<bool>(
      "disable_fld_file_output", false, "Whether to turn off all NekRS field file output writing "
      "(for the usual field file output - this does not affect writing the usrwrk with 'usrwrk_output')");

  params.addParam<bool>("minimize_transfers_in",
                        false,
                        "Whether to only synchronize nekRS "
                        "for the direction TO_EXTERNAL_APP on multiapp synchronization steps");
  params.addParam<bool>("minimize_transfers_out",
                        false,
                        "Whether to only synchronize nekRS "
                        "for the direction FROM_EXTERNAL_APP on multiapp synchronization steps");

  params.addParam<bool>("skip_final_field_file", false, "By default, we write a NekRS field file "
    "on the last time step; set this to true to disable");

  params.addParam<MooseEnum>("synchronization_interval", getSynchronizationEnum(),
    "When to synchronize the NekRS solution with the mesh mirror. By default, the NekRS solution "
    "is mapped to/receives data from the mesh mirror for every time step.");
  params.addParam<unsigned int>("constant_interval", 1,
    "Constant interval (in units of number of time steps) with which to synchronize the NekRS solution");
  return params;
}

NekRSProblemBase::NekRSProblemBase(const InputParameters & params)
  : CardinalProblem(params),
    _serialized_solution(NumericVector<Number>::build(_communicator).release()),
    _casename(getParam<std::string>("casename")),
    _nondimensional(getParam<bool>("nondimensional")),
    _U_ref(getParam<Real>("U_ref")),
    _T_ref(getParam<Real>("T_ref")),
    _dT_ref(getParam<Real>("dT_ref")),
    _L_ref(getParam<Real>("L_ref")),
    _rho_0(getParam<Real>("rho_0")),
    _Cp_0(getParam<Real>("Cp_0")),
    _write_fld_files(getParam<bool>("write_fld_files")),
    _disable_fld_file_output(getParam<bool>("disable_fld_file_output")),
    _n_usrwrk_slots(getParam<unsigned int>("n_usrwrk_slots")),
    _constant_interval(getParam<unsigned int>("constant_interval")),
    _skip_final_field_file(getParam<bool>("skip_final_field_file")),
    _first_reserved_usrwrk_slot(getParam<unsigned int>("first_reserved_usrwrk_slot")),
    _start_time(nekrs::startTime()),
    _elapsedStepSum(0.0),
    _elapsedTime(nekrs::getNekSetupTime()),
    _tSolveStepMin(std::numeric_limits<double>::max()),
    _tSolveStepMax(std::numeric_limits<double>::min()),
    _scratch_counter(0),
    _n_uo_slots(0)
{
  if (params.isParamSetByUser("minimize_transfers_in"))
    mooseError("The 'minimize_transfers_in' parameter has been replaced by "
      "'synchronization_interval = parent_app'! Please update your input files.");

  if (params.isParamSetByUser("minimize_transfers_out"))
    mooseError("The 'minimize_transfers_out' parameter has been replaced by "
      "'synchronization_interval = parent_app'! Please update your input files.");

  if (_n_usrwrk_slots == 0)
    checkUnusedParam(params, "first_reserved_usrwrk_slot", "not reserving any scratch space");
  else if (_first_reserved_usrwrk_slot >= _n_usrwrk_slots)
    mooseError("The 'first_reserved_usrwrk_slot' (" + std::to_string(_first_reserved_usrwrk_slot) +
               ") "
               "must be less than 'n_usrwrk_slots' (" +
               std::to_string(_n_usrwrk_slots) + ")!");

  for (unsigned int i = 0; i < _first_reserved_usrwrk_slot; ++i)
    _usrwrk_indices.push_back("unused");

  _synchronization_interval = getParam<MooseEnum>("synchronization_interval").getEnum<
    synchronization::SynchronizationEnum>();

   switch (_synchronization_interval)
   {
     case synchronization::parent_app:
     {
        // the way the data transfers are detected depend on nekRS being a sub-application,
        // so these settings are not invalid if nekRS is the master app (though you could
        // relax this in the future by reversing the synchronization step identification
        // from the nekRS-subapp case to the nekRS-master app case - it's just not implemented yet).
        if (_app.isUltimateMaster())
        {
          mooseWarning("The 'synchronization_interval = parent_app' capability "
                     "requires that nekRS is receiving and sending data to a parent application, but "
                     "in your case nekRS is the main application.\n\n"
                     "We are reverting synchronization_interval to 'constant'.");
          _synchronization_interval = synchronization::constant;
        }

        checkUnusedParam(params, "constant_interval", "synchronizing based on the 'parent_app'");
        break;
     }
     case synchronization::constant:
       break;
     default:
       mooseError("Unhandled SynchronizationEnum in NekRSProblemBase!");
  }

  if (_disable_fld_file_output && _write_fld_files)
    mooseError("Cannot both disable all field file output and write custom field files!\n"
               "'write_fld_files' and 'disable_fld_file_output' cannot both be true!");

  if (_app.isUltimateMaster() && _write_fld_files)
    mooseError("The 'write_fld_files' setting should only be true when multiple Nek simulations "
               "are run as sub-apps on a master app.\nYour input has Nek as the master app.");

  _nek_mesh = dynamic_cast<NekRSMesh *>(&mesh());

  if (!_nek_mesh)
    mooseError("Mesh for '" + type() + "' must be of type 'NekRSMesh', but you have specified a '" +
               mesh().type() + "'!");

  _moose_Nq = _nek_mesh->order() + 2;

  // the Problem constructor is called right after building the mesh. In order
  // to have pretty screen output without conflicting with the timed print messages,
  // print diagnostic info related to the mesh here. If running in JIT mode, this
  // diagnostic info was never set, so the numbers that would be printed are garbage.
  if (!nekrs::buildOnly())
    _nek_mesh->printMeshInfo();

  // if solving in nondimensional form, make sure that the user specified _all_ of the
  // necessary scaling quantities to prevent errors from forgetting one, which would take
  // a non-scaled default otherwise
  std::vector<std::string> scales = {"U_ref", "T_ref", "dT_ref", "L_ref", "rho_0", "Cp_0"};
  for (const auto & s : scales)
  {
    if (_nondimensional)
      checkRequiredParam(params, s, "solving in non-dimensional form");
    else
      checkUnusedParam(params, s, "solving in dimensional form");
  }

  // inform NekRS of the scaling that we are using if solving in non-dimensional form
  nekrs::initializeDimensionalScales(_U_ref, _T_ref, _dT_ref, _L_ref, _rho_0, _Cp_0);

  if (_nondimensional)
  {
    VariadicTable<std::string, std::string, std::string, std::string,
      std::string, std::string, std::string> vt(
      {"Time", "Length", "Velocity", "Temperature", "d(Temperature)",
       "Density", "Specific Heat"});

    auto compress = [] (Real a)
    {
      std::ostringstream v;
      v << std::setprecision(3) << std::scientific << a;
      return v.str();
    };

    vt.addRow(compress(_L_ref / _U_ref), compress(_L_ref), compress(_U_ref),
      compress(_T_ref), compress(_dT_ref), compress(_rho_0), compress(_Cp_0));
    _console << "\nNekRS characteristic scales:" << std::endl;
    vt.print(_console);
    _console << std::endl;
  }

  // It's too complicated to make sure that the dimensional form _also_ works when our
  // reference coordinates are different from what MOOSE is expecting, so just throw an error
  if (_nondimensional && !MooseUtils::absoluteFuzzyEqual(_nek_mesh->scaling(), _L_ref))
    paramError("L_ref",
               "If solving NekRS in nondimensional form, you must choose "
               "reference dimensional scales in the same units as expected by MOOSE, i.e. 'L_ref' "
               "must match 'scaling' in 'NekRSMesh'.");

  // boundary-specific data
  _boundary = _nek_mesh->boundary();
  _n_surface_elems = _nek_mesh->numSurfaceElems();
  _n_vertices_per_surface = _nek_mesh->numVerticesPerSurface();

  // volume-specific data
  _volume = _nek_mesh->volume();
  _n_volume_elems = _nek_mesh->numVolumeElems();
  _n_vertices_per_volume = _nek_mesh->numVerticesPerVolume();

  // generic data
  _n_vertices_per_elem = _nek_mesh->numVerticesPerElem();

  if (_volume)
    _n_points = _n_volume_elems * _n_vertices_per_volume * _nek_mesh->nBuildPerVolumeElem();
  else
    _n_points = _n_surface_elems * _n_vertices_per_surface * _nek_mesh->nBuildPerSurfaceElem();

  initializeInterpolationMatrices();

  // we can save some effort for the low-order situations where the interpolation
  // matrix is the identity matrix (i.e. for which equi-spaced libMesh nodes are an
  // exact subset of the nekRS GLL points). This will happen for any first-order mesh,
  // and if a second-order mesh is used with a polynomial order of 2 in nekRS. Because
  // we pretty much always use a polynomial order greater than 2 in nekRS, let's just
  // check the first case because this will simplify our code in the nekrs::boundarySolution
  // function. If you change this line, you MUST change the innermost if/else statement
  // in nekrs::boundarySolution!
  _needs_interpolation = _nek_mesh->numQuadraturePoints1D() > 2;

  if (isParamValid("output"))
  {
    _outputs = &getParam<MultiMooseEnum>("output");
    _external_data = (double *)calloc(_n_points, sizeof(double));
  }

  checkJointParams(params, {"usrwrk_output", "usrwrk_output_prefix"},
    "outputting usrwrk slots to field files");

  if (isParamValid("usrwrk_output"))
  {
    _usrwrk_output = &getParam<std::vector<unsigned int>>("usrwrk_output");
    _usrwrk_output_prefix = &getParam<std::vector<std::string>>("usrwrk_output_prefix");

    for (const auto & s : *_usrwrk_output)
      if (s >= _n_usrwrk_slots)
        mooseError("Cannot write field file for usrwrk slot greater than the total number of "
          "allocated slots: ", _n_usrwrk_slots, "!");

    if (_usrwrk_output->size() != _usrwrk_output_prefix->size())
      mooseError("The length of 'usrwrk_output' must match the length of 'usrwrk_output_prefix'!");
  }
}

NekRSProblemBase::~NekRSProblemBase()
{
  // write nekRS solution to output if not already written for this step; nekRS does this
  // behavior, so we duplicate it
  if (!_is_output_step && !_skip_final_field_file)
  {
    if (_write_fld_files)
      mooseWarning("When 'write_fld_files' is enabled, we skip Nek field file writing on end time!\n"
                   "Depending on how many ranks you used, MOOSE may use the same object to run multiple\n"
                   "sub-applications. By the time we get to the last time step, we've collapsed back to\n"
                   "this singular state and don't have access to the individual Nek solves, so we cannot\n"
                   "write the last time step solution to field files.\n\n"
                   "To hide this warning, set 'skip_final_field_file = true'.");
    else
      writeFieldFile(_time, _t_step);
  }

  if (nekrs::runTimeStatFreq())
    if (_t_step % nekrs::runTimeStatFreq())
      nekrs::printRuntimeStatistics(_t_step);

  freePointer(_external_data);
  freePointer(_interpolation_outgoing);
  freePointer(_interpolation_incoming);

  nekrs::finalize();
}

void
NekRSProblemBase::writeFieldFile(const Real & step_end_time, const int & step) const
{
  if (_disable_fld_file_output)
    return;

  Real t = _timestepper->nondimensionalDT(step_end_time);

  if (_write_fld_files)
  {
    // this is the app number, but a single app may run Nek multiple times
    auto app_number = std::to_string(_app.multiAppNumber());

    // apps may also have numbers in their names, so we first need to get the actual raw app name;
    // we strip out the app_number from the end of the app name
    if (!stringHasEnding(_app.name(), app_number))
      mooseError("Internal error: app name '" + _app.name() + "' does not end with app number: " + app_number);

    auto name = _app.name().substr(0, _app.name().size() - app_number.size());
    auto full_path = _app.getOutputFileBase();
    std::string last_element(full_path.substr(full_path.rfind(name) + name.size()));

    auto prefix = fieldFilePrefix(std::stoi(last_element));

    nekrs::write_field_file(prefix, t, step);
  }
  else
    nekrs::outfld(t, step);
}

void
NekRSProblemBase::initializeInterpolationMatrices()
{
  mesh_t * mesh = nekrs::entireMesh();

  // determine the interpolation matrix for the outgoing transfer
  int starting_points = mesh->Nq;
  int ending_points = _nek_mesh->numQuadraturePoints1D();
  _interpolation_outgoing = (double *)calloc(starting_points * ending_points, sizeof(double));
  nekrs::interpolationMatrix(_interpolation_outgoing, starting_points, ending_points);

  // determine the interpolation matrix for the incoming transfer
  std::swap(starting_points, ending_points);
  _interpolation_incoming = (double *)calloc(starting_points * ending_points, sizeof(double));
  nekrs::interpolationMatrix(_interpolation_incoming, starting_points, ending_points);
}

std::string
NekRSProblemBase::fieldFilePrefix(const int & number) const
{
  const std::string alphabet = "abcdefghijklmnopqrstuvwxyz";
  int letter = number / 26;
  int remainder = number % 100;
  std::string s = remainder < 10 ? "0" : "";

  return alphabet[letter] + s + std::to_string(remainder);
}

void
NekRSProblemBase::fillAuxVariable(const unsigned int var_number, const double * value)
{
  auto & solution = _aux->solution();
  auto sys_number = _aux->number();
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

      for (unsigned int n = 0; n < _n_vertices_per_elem; n++)
      {
        auto node_ptr = elem_ptr->node_ptr(n);

        // For each face vertex, we can only write into the MOOSE auxiliary fields if that
        // vertex is "owned" by the present MOOSE process.
        if (node_ptr->processor_id() == pid)
        {
          int node_index = _nek_mesh->nodeIndex(n);
          auto node_offset = (e * _nek_mesh->nMoosePerNek() + build) * _n_vertices_per_elem + node_index;

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

void
NekRSProblemBase::getNekScalarValueUserObjects()
{
  // Find all of the NekScalarValue user objects so that we can print a table
  TheWarehouse::Query uo_query = theWarehouse().query().condition<AttribSystem>("UserObject");
  std::vector<UserObject *> userobjs;
  uo_query.queryInto(userobjs);

  std::set<unsigned int> slots;
  for (const auto & u : userobjs)
  {
    NekScalarValue * c = dynamic_cast<NekScalarValue *>(u);
    if (c)
    {
      slots.insert(c->usrwrkSlot());
      c->setCounter(_scratch_counter);
      _nek_uos.push_back(c);
      _scratch_counter++;
    }
  }

  if (slots.size() == 0)
    return;

  auto min_for_uo = *slots.begin();
  if (min_for_uo > _minimum_scratch_size_for_coupling + _first_reserved_usrwrk_slot)
  {
    std::stringstream coupling_slots;
    coupling_slots << "0";
    for (unsigned int i = 1; i < _minimum_scratch_size_for_coupling + _first_reserved_usrwrk_slot;
         ++i)
      coupling_slots << ", " << i;

    mooseError("The 'usrwrk_slot' specified for the NekScalarValue user objects must not exhibit\n"
      "any gaps between the slots used for multiphysics coupling (", coupling_slots.str(), ") and the "
      "first\nslot used for NekScalarValue (", min_for_uo, "). Please adjust the 'usrwrk_slot' choices\n"
      "for the NekScalarValue user objects.");
  }

  auto max_for_uo = *slots.rbegin();
  _n_uo_slots = slots.size();
  if (max_for_uo - min_for_uo >= _n_uo_slots)
  {
    std::stringstream coupling_slots;
    for (const auto & s : slots)
      coupling_slots << s << ", ";

    std::string str = coupling_slots.str();
    str.pop_back();
    str.pop_back();
    mooseError("The 'usrwrk_slot' specified for the NekScalarValue user objects must not exhibit\n"
      "any gaps. You are currently allocating scalar values into non-contiguous slots (", str, ")");
  }
}

void
NekRSProblemBase::initialSetup()
{
  CardinalProblem::initialSetup();

  auto executioner = _app.getExecutioner();
  _transient_executioner = dynamic_cast<Transient *>(executioner);

  // NekRS only supports transient simulations - therefore, it does not make
  // sense to use anything except a Transient-derived executioner
  if (!_transient_executioner)
    mooseError("A 'Transient' executioner must be used with '" + type() +
               "', but "
               "you have specified the '" +
               executioner->type() + "' executioner!");

  // To get the correct time stepping information on the MOOSE side, we also
  // must use the NekTimeStepper
  TimeStepper * stepper = _transient_executioner->getTimeStepper();
  _timestepper = dynamic_cast<NekTimeStepper *>(stepper);
  if (!_timestepper)
    mooseError("The 'NekTimeStepper' stepper must be used with '" + type() +
               "', "
               "but you have specified the '" +
               stepper->type() + "' time stepper!");

  // Set the reference time for use in dimensionalizing/non-dimensionalizing the time
  _timestepper->setReferenceTime(_L_ref, _U_ref);

  // Set the NekRS start time to whatever is set on Executioner/start_time; print
  // a message if those times don't match the .par file
  const auto moose_start_time = _transient_executioner->getStartTime();
  nekrs::setStartTime(_timestepper->nondimensionalDT(moose_start_time));
  _start_time = moose_start_time;

  if (_synchronization_interval == synchronization::parent_app)
    _transfer_in = &getPostprocessorValueByName("transfer_in");

  getNekScalarValueUserObjects();

  VariadicTable<std::string, std::string, std::string> vt({"Quantity", "How to Access (.oudf)", "How to Access (.udf)"});

  // add rows for the coupling data
  int end = _minimum_scratch_size_for_coupling + _first_reserved_usrwrk_slot;
  for (int i = 0; i < end; ++i)
    vt.addRow(_usrwrk_indices[i],
              "bc->usrwrk[" + std::to_string(i) + " * bc->fieldOffset + bc->idM]",
              "nrs->usrwrk[" + std::to_string(i) + " * nrs->fieldOffset + n]");

  // add rows for the NekScalarValue(s)
  for (const auto & uo : _nek_uos)
  {
    auto slot = uo->usrwrkSlot();
    auto count = uo->counter();
    vt.addRow(uo->name(),
              "bc->usrwrk[" + std::to_string(slot) + " * bc->fieldOffset + " +
                  std::to_string(count) + "]",
              "nrs->usrwrk[" + std::to_string(slot) + " * nrs->fieldOffset + " +
                  std::to_string(count) + "]");
  }

  // add rows for the extra slices
  for (unsigned int i = end + _n_uo_slots; i < _n_usrwrk_slots; ++i)
    vt.addRow("unused",
              "bc->usrwrk[" + std::to_string(i) + " * bc->fieldOffset + bc->idM]",
              "nrs->usrwrk[" + std::to_string(i) + " * nrs->fieldOffset + n]");

  if (_n_usrwrk_slots < _minimum_scratch_size_for_coupling + _first_reserved_usrwrk_slot)
    mooseError("You did not allocate enough scratch space for Cardinal to complete its coupling!\n"
               "'n_usrwrk_slots' must be greater than or equal to ",
               _minimum_scratch_size_for_coupling + _first_reserved_usrwrk_slot,
               "!");

  if (_n_usrwrk_slots == 0)
  {
    _console << "Skipping allocation of NekRS scratch space because 'n_usrwrk_slots' is 0\n" << std::endl;
  }
  else
  {
    _console << "\n ===================>     MAPPING FROM MOOSE TO NEKRS      <===================\n" << std::endl;
    _console <<   "          Slice:  entry in NekRS scratch space" << std::endl;
    _console << "       Quantity:  physical meaning or name of data in this slice. If 'unused',\n"
             << std::endl;
    _console << "                  this means that the space has been allocated, but Cardinal\n"
             << std::endl;
    _console << "                  is not otherwise using it for coupling\n" << std::endl;
    _console <<   "  How to Access:  C++ code to use in NekRS files; for the .udf instructions," << std::endl;
    _console <<   "                  'n' indicates a loop variable over GLL points\n" << std::endl;
    vt.print(_console);
    _console << std::endl;
  }

  // nekRS calls UDF_ExecuteStep once before the time stepping begins; the isLastStep stuff is
  // copy-pasta from NekRS main(), except that if Nek is a sub-app, we give full control of
  // time stepping to the main app
  bool isLastStep = false;
  if (_app.isUltimateMaster())
    isLastStep = !((nekrs::endTime() > nekrs::startTime() || nekrs::numSteps() > _t_step));
  nekrs::lastStep(isLastStep);

  nekrs::udfExecuteStep(_timestepper->nondimensionalDT(_start_time), _t_step, false /* not an output step */);
  nekrs::resetTimer("udfExecuteStep");
}

void
NekRSProblemBase::externalSolve()
{
  if (nekrs::buildOnly())
    return;

  const double timeStartStep = MPI_Wtime();

  // _dt reflects the time step that MOOSE wants Nek to
  // take. For instance, if Nek is controlled by a master app and subcycling is used,
  // Nek must advance to the time interval taken by the master app. If the time step
  // that MOOSE wants nekRS to take (i.e. _dt) is smaller than we'd like nekRS to take, error.
  if (_dt < _timestepper->minDT())
    mooseError("Requested time step of " + std::to_string(_dt) +
               " is smaller than the minimum "
               "time step of " +
               Moose::stringify(_timestepper->minDT()) +
               " allowed in NekRS!\n\n"
               "You can control this behavior with the 'min_dt' parameter on 'NekTimeStepper'.");

  // _time represents the time that we're simulating _to_, but we need to pass sometimes slightly
  // different times into the nekRS routines, which assume that the "time" passed into their
  // routines is sometimes a different interpretation.
  double step_start_time = _time - _dt;
  double step_end_time = _time;

  _is_output_step = isOutputStep();

  // tell NekRS what the value of nrs->isOutputStep should be
  nekrs::outputStep(_is_output_step);

  // NekRS prints out verbose info for the first 1000 time steps
  if (_t_step <= 1000)
    nekrs::verboseInfo(true);

  // Tell NekRS what the time step size is
  nekrs::initStep(_timestepper->nondimensionalDT(step_start_time),
                  _timestepper->nondimensionalDT(_dt),
                  _t_step);

  // Run a nekRS time step. After the time step, this also calls UDF_ExecuteStep,
  // evaluated at (step_end_time, _t_step) == (nek_step_start_time + nek_dt, t_step)
  int corrector = 1;
  bool converged = false;
  do
  {
    converged = nekrs::runStep(corrector++);
  } while (!converged);

  // TODO: time is somehow corrected here
  nekrs::finishStep();

  // optional entry point to adjust the recently-computed NekRS solution
  adjustNekSolution();

  // copy-pasta from Nek's main() for calling timers and printing
  if (nekrs::updateFileCheckFreq())
    if (_t_step % nekrs::updateFileCheckFreq())
      nekrs::processUpdFile();

  // Note: here, we copy to both the nrs solution arrays and to the Nek5000 backend arrays,
  // because it is possible that users may interact using the legacy usr-file approach.
  // If we move away from the Nek5000 backend entirely, we could replace this line with
  // direct OCCA memcpy calls. But we do definitely need some type of copy here for _every_
  // time step, even if we're not technically passing data to another app, because we have
  // postprocessors that touch the `nrs` arrays that can be called in an arbitrary fashion
  // by the user.
  nek::ocopyToNek(_timestepper->nondimensionalDT(step_end_time), _t_step);

  if (nekrs::printInfoFreq())
    if (_t_step % nekrs::printInfoFreq() == 0)
      nekrs::printInfo(_timestepper->nondimensionalDT(_time), _t_step, false, true);

  if (_is_output_step)
  {
    writeFieldFile(step_end_time, _t_step);

    // TODO: I could not figure out why this can't be called from the destructor, to
    // add another field file on Cardinal's last time step. Revisit in the future.
    if (_usrwrk_output)
    {
      static std::vector<bool> first_fld(_usrwrk_output->size(), true);

      for (unsigned int i = 0; i < _usrwrk_output->size(); ++i)
      {
        bool write_coords = first_fld[i] ? true : false;

        nekrs::write_usrwrk_field_file((*_usrwrk_output)[i], (*_usrwrk_output_prefix)[i],
          _timestepper->nondimensionalDT(step_end_time), _t_step, write_coords);

        first_fld[i] = false;
      }
    }
  }

  MPI_Barrier(comm().get());
  const double elapsedStep = MPI_Wtime() - timeStartStep;
  _tSolveStepMin = std::min(elapsedStep, _tSolveStepMin);
  _tSolveStepMax = std::max(elapsedStep, _tSolveStepMax);
  nekrs::updateTimer("minSolveStep", _tSolveStepMin);
  nekrs::updateTimer("maxSolveStep", _tSolveStepMax);

  _elapsedStepSum += elapsedStep;
  _elapsedTime += elapsedStep;
  nekrs::updateTimer("elapsedStep", elapsedStep);
  nekrs::updateTimer("elapsedStepSum", _elapsedStepSum);
  nekrs::updateTimer("elapsed", _elapsedTime);

  if (nekrs::printInfoFreq())
    if (_t_step % nekrs::printInfoFreq() == 0)
      nekrs::printInfo(_timestepper->nondimensionalDT(_time), _t_step, true, false);

  if (nekrs::runTimeStatFreq())
    if (_t_step % nekrs::runTimeStatFreq() == 0)
      nekrs::printRuntimeStatistics(_t_step);

  _time += _dt;
}

bool
NekRSProblemBase::isDataTransferHappening(ExternalProblem::Direction direction)
{
  if (nekrs::buildOnly())
    return false;

  switch (direction)
  {
    case ExternalProblem::Direction::TO_EXTERNAL_APP:
      return synchronizeIn();
    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
      return synchronizeOut();
    default:
      mooseError("Unhandled DirectionEnum in NekRSProblemBase!");
  }
}

void
NekRSProblemBase::sendScalarValuesToNek()
{
  if (_nek_uos.size() == 0)
    return;

  for (const auto & uo : _nek_uos)
    uo->setValue();

  if (udf.properties)
  {
    nrs_t * nrs = (nrs_t *) nekrs::nrsPtr();
    evaluateProperties(nrs, _timestepper->nondimensionalDT(_time));
  }
}

void
NekRSProblemBase::syncSolutions(ExternalProblem::Direction direction)
{
  auto & solution = _aux->solution();

  if (!isDataTransferHappening(direction))
    return;

  switch (direction)
  {
    case ExternalProblem::Direction::TO_EXTERNAL_APP:
    {
      if (_first)
      {
        _serialized_solution->init(_aux->sys().n_dofs(), false, SERIAL);
        _first = false;
      }

      solution.localize(*_serialized_solution);

      sendScalarValuesToNek();

      copyScratchToDevice();

      break;

      return;
    }
    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    {
      // extract the NekRS solution onto the mesh mirror, if specified
      extractOutputs();
      break;
    }
    default:
      mooseError("Unhandled Transfer::DIRECTION enum!");
  }

  solution.close();
  _aux->system().update();
}

bool
NekRSProblemBase::synchronizeIn()
{
  bool synchronize = true;

  switch (_synchronization_interval)
  {
    case synchronization::parent_app:
    {
      // For the minimized incoming synchronization to work correctly, the value
      // of the incoming postprocessor must not be zero. We only need to check this for the very
      // first time we evaluate this function. This ensures that you don't accidentally set a
      // zero value as a default in the master application's postprocessor.
      if (_first && *_transfer_in == false)
        mooseError("The default value for the 'transfer_in' postprocessor received by nekRS "
                   "must not be false! Make sure that the master application's "
                   "postprocessor is not zero.");

      if (*_transfer_in == false)
        synchronize = false;
      else
        setPostprocessorValueByName("transfer_in", false, 0);

      break;
    }
    case synchronization::constant:
    {
      synchronize = timeStep() % _constant_interval == 0;
      break;
    }
    default:
      mooseError("Unhandled SynchronizationEnum in NekRSProblemBase!");
  }

  return synchronize;
}

bool
NekRSProblemBase::synchronizeOut()
{
  bool synchronize = true;

  switch (_synchronization_interval)
  {
    case synchronization::parent_app:
    {
      if (std::abs(_time - _dt - _transient_executioner->getTargetTime()) >
          _transient_executioner->timestepTol())
        synchronize = false;
      break;
    }
    case synchronization::constant:
    {
      synchronize = timeStep() % _constant_interval == 0;
      break;
    }
    default:
      mooseError("Unhandled SynchronizationEnum in NekRSProblemBase!");
  }

  return synchronize;
}

bool
NekRSProblemBase::isOutputStep() const
{
  if (_app.isUltimateMaster())
  {
    bool last_step = nekrs::lastStep(
        _timestepper->nondimensionalDT(_time), _t_step, 0.0 /* dummy elapsed time */);

    // if Nek is controlled by a master application, then the last time step
    // is controlled by that master application, in which case we don't want to
    // write at what nekRS thinks is the last step (since it may or may not be
    // the actual end step), especially because we already ensure that we write on the
    // last time step from MOOSE's perspective in NekRSProblem's destructor.
    if (last_step)
      return true;
  }

  // this routine does not check if we are on the last step - just whether we have
  // met the requested runtime or time step interval
  return nekrs::outputStep(_timestepper->nondimensionalDT(_time), _t_step);
}

void
NekRSProblemBase::extractOutputs()
{
  if (_outputs && _var_names.size())
  {
    _console << "Interpolating" + _var_string + " NekRS solution onto mesh mirror" << std::endl;

    for (std::size_t i = 0; i < _var_names.size(); ++i)
    {
      field::NekFieldEnum field_enum;

      if (_var_names[i] == "temp")
        field_enum = field::temperature;
      else if (_var_names[i] == "P")
        field_enum = field::pressure;
      else if (_var_names[i] == "vel_x")
        field_enum = field::velocity_x;
      else if (_var_names[i] == "vel_y")
        field_enum = field::velocity_y;
      else if (_var_names[i] == "vel_z")
        field_enum = field::velocity_z;
      else if (_var_names[i] == "scalar01")
        field_enum = field::scalar01;
      else if (_var_names[i] == "scalar02")
        field_enum = field::scalar02;
      else if (_var_names[i] == "scalar03")
        field_enum = field::scalar03;
      else
        mooseError("Unhandled NekFieldEnum in NekRSProblemBase!");

      if (!_volume)
        boundarySolution(field_enum, _external_data);

      if (_volume)
        volumeSolution(field_enum, _external_data);

      fillAuxVariable(_external_vars[i], _external_data);
    }
  }
}

InputParameters
NekRSProblemBase::getExternalVariableParameters()
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
      mooseError("Unhandled 'NekOrderEnum' in 'NekRSProblemBase'!");
  }

  return var_params;
}

void
NekRSProblemBase::addTemperatureVariable()
{
  // For the special case of temperature, we want the variable name to be
  // 'temp' instead of 'temperature' due to legacy reasons of what NekRSProblem
  // chose to name the temperature variable. For everything else, we just use
  // the name of the output parameter.
  _var_names.push_back("temp");
}

void
NekRSProblemBase::addExternalVariables()
{
  if (_outputs)
  {
    auto var_params = getExternalVariableParameters();

    for (std::size_t i = 0; i < _outputs->size(); ++i)
    {
      std::string output = (*_outputs)[i];

      if (output == "temperature")
        addTemperatureVariable();
      else if (output == "velocity")
      {
        // For the velocity, we need to explicitly output each component; Paraview
        // will then combine the components together into a vector
        _var_names.push_back("vel_x");
        _var_names.push_back("vel_y");
        _var_names.push_back("vel_z");
      }
      else if (output == "pressure")
        _var_names.push_back("P");
      else if (output == "scalar01")
        _var_names.push_back("scalar01");
      else if (output == "scalar02")
        _var_names.push_back("scalar02");
      else if (output == "scalar03")
        _var_names.push_back("scalar03");
    }

    _var_string = "";
    for (const auto & name : _var_names)
    {
      checkDuplicateVariableName(name);
      addAuxVariable("MooseVariable", name, var_params);
      _external_vars.push_back(_aux->getFieldVariable<Real>(0, name).number());

      _var_string += " " + name + ",";
    }

    _var_string.erase(std::prev(_var_string.end()));
  }

  if (_synchronization_interval == synchronization::parent_app)
  {
    auto pp_params = _factory.getValidParams("Receiver");
    pp_params.set<std::vector<OutputName>>("outputs") = {"none"};
    addPostprocessor("Receiver", "transfer_in", pp_params);
  }
}

void
NekRSProblemBase::volumeSolution(const field::NekFieldEnum & field, double * T)
{
  mesh_t * mesh = nekrs::entireMesh();
  auto vc = _nek_mesh->volumeCoupling();

  double (*f)(int);
  f = nekrs::solutionPointer(field);

  int start_1d = mesh->Nq;
  int end_1d = _moose_Nq;
  int start_3d = start_1d * start_1d * start_1d;
  int end_3d = end_1d * end_1d * end_1d;

  int n_to_write = vc.n_elems * end_3d * _nek_mesh->nBuildPerVolumeElem();

  // allocate temporary space:
  // - Ttmp: results of the search for each process
  // - Telem: scratch space for volume interpolation to avoid reallocating a bunch (only used if interpolating)
  double * Ttmp = (double *)calloc(n_to_write, sizeof(double));
  double * Telem = (double *)calloc(start_3d, sizeof(double));

  auto indices = _nek_mesh->cornerIndices();

  int c = 0;
  for (int k = 0; k < mesh->Nelements; ++k)
  {
    int offset = k * start_3d;

    for (int build = 0; build < _nek_mesh->nBuildPerVolumeElem(); ++build)
    {
      if (_needs_interpolation)
      {
        // get the solution on the element
        for (int v = 0; v < start_3d; ++v)
          Telem[v] = f(offset + v);

        // and then interpolate it
        nekrs::interpolateVolumeHex3D(_interpolation_outgoing, Telem, start_1d, &(Ttmp[c]), end_1d);
        c += end_3d;
      }
      else
      {
        // get the solution on the element - no need to interpolate
        for (int v = 0; v < end_3d; ++v, ++c)
          Ttmp[c] = f(offset + indices[build][v]);
      }
    }
  }

  // dimensionalize the solution if needed
  for (int v = 0; v < n_to_write; ++v)
  {
    nekrs::dimensionalize(field, Ttmp[v]);

    // if temperature, we need to add the reference temperature
    if (field == field::temperature)
      Ttmp[v] += _T_ref;
  }

  nekrs::allgatherv(vc.mirror_counts, Ttmp, T, end_3d);

  freePointer(Ttmp);
  freePointer(Telem);
}

void
NekRSProblemBase::boundarySolution(const field::NekFieldEnum & field, double * T)
{
  mesh_t * mesh = nekrs::entireMesh();
  auto bc = _nek_mesh->boundaryCoupling();

  double (*f)(int);
  f = nekrs::solutionPointer(field);

  int start_1d = mesh->Nq;
  int end_1d = _moose_Nq;
  int start_2d = start_1d * start_1d;
  int end_2d = end_1d * end_1d;

  int n_to_write = bc.n_faces * end_2d * _nek_mesh->nBuildPerSurfaceElem();

  // allocate temporary space:
  // - Ttmp: results of the search for each process
  // - Tface: scratch space for face solution to avoid reallocating a bunch (only used if interpolating)
  // - scratch: scratch for the interpolatino process to avoid reallocating a bunch (only used if interpolating0
  double * Ttmp = (double *)calloc(n_to_write, sizeof(double));
  double * Tface = (double *)calloc(start_2d, sizeof(double));
  double * scratch = (double *)calloc(start_1d * end_1d, sizeof(double));

  auto indices = _nek_mesh->cornerIndices();

  int c = 0;
  for (int k = 0; k < bc.total_n_faces; ++k)
  {
    if (bc.process[k] == nekrs::commRank())
    {
      int i = bc.element[k];
      int j = bc.face[k];
      int offset = i * mesh->Nfaces * start_2d + j * start_2d;

      for (int build = 0; build < _nek_mesh->nBuildPerSurfaceElem(); ++build)
      {
        if (_needs_interpolation)
        {
          // get the solution on the face
          for (int v = 0; v < start_2d; ++v)
          {
            int id = mesh->vmapM[offset + v];
            Tface[v] = f(id);
          }

          // and then interpolate it
          nekrs::interpolateSurfaceFaceHex3D(
              scratch, _interpolation_outgoing, Tface, start_1d, &(Ttmp[c]), end_1d);
          c += end_2d;
        }
        else
        {
          // get the solution on the face - no need to interpolate
          for (int v = 0; v < end_2d; ++v, ++c)
          {
            int id = mesh->vmapM[offset + indices[build][v]];
            Ttmp[c] = f(id);
          }
        }
      }
    }
  }

  // dimensionalize the solution if needed
  for (int v = 0; v < n_to_write; ++v)
  {
    nekrs::dimensionalize(field, Ttmp[v]);

    // if temperature, we need to add the reference temperature
    if (field == field::temperature)
      Ttmp[v] += _T_ref;
  }

  nekrs::allgatherv(bc.mirror_counts, Ttmp, T, end_2d);

  freePointer(Ttmp);
  freePointer(Tface);
  freePointer(scratch);
}

void
NekRSProblemBase::writeVolumeSolution(const int elem_id,
                                      const field::NekWriteEnum & field,
                                      double * T,
                                      const std::vector<double> * add)
{
  mesh_t * mesh = nekrs::entireMesh();
  void (*write_solution)(int, dfloat);
  write_solution = nekrs::solutionPointer(field);

  auto vc = _nek_mesh->volumeCoupling();
  int id = vc.element[elem_id] * mesh->Np;

  if (_nek_mesh->exactMirror())
  {
    // can write directly into the NekRS solution
    for (int v = 0; v < mesh->Np; ++v)
    {
      double extra = (add == nullptr) ? 0.0 : (*add)[id + v];
      write_solution(id + v, T[v] + extra);
    }
  }
  else
  {
    // need to interpolate onto the higher-order Nek mesh
    double * tmp = (double *)calloc(mesh->Np, sizeof(double));

    interpolateVolumeSolutionToNek(elem_id, T, tmp);

    for (int v = 0; v < mesh->Np; ++v)
    {
      double extra = (add == nullptr) ? 0.0 : (*add)[id + v];
      write_solution(id + v, tmp[v] + extra);
    }

    freePointer(tmp);
  }
}

void
NekRSProblemBase::writeBoundarySolution(const int elem_id, const field::NekWriteEnum & field,
  double * T)
{
  mesh_t * mesh = nekrs::temperatureMesh();
  void (*write_solution)(int, dfloat);
  write_solution = nekrs::solutionPointer(field);

  const auto & bc = _nek_mesh->boundaryCoupling();
  int offset = bc.element[elem_id] * mesh->Nfaces * mesh->Nfp + bc.face[elem_id] * mesh->Nfp;

  if (_nek_mesh->exactMirror())
  {
    // can write directly into the NekRS solution
    for (int i = 0; i < mesh->Nfp; ++i)
      write_solution(mesh->vmapM[offset + i], T[i]);
  }
  else
  {
    // need to interpolate onto the higher-order Nek mesh
    double * tmp = (double *)calloc(mesh->Nfp, sizeof(double));
    interpolateBoundarySolutionToNek(T, tmp);

    for (int i = 0; i < mesh->Nfp; ++i)
      write_solution(mesh->vmapM[offset + i], tmp[i]);

    freePointer(tmp);
  }
}

void
NekRSProblemBase::interpolateVolumeSolutionToNek(const int elem_id, double * incoming_moose_value,
  double * outgoing_nek_value)
{
  mesh_t * mesh = nekrs::entireMesh();

  nekrs::interpolateVolumeHex3D(_interpolation_incoming, incoming_moose_value, _moose_Nq,
    outgoing_nek_value, mesh->Nq);
}

void
NekRSProblemBase::interpolateBoundarySolutionToNek(double * incoming_moose_value,
  double * outgoing_nek_value)
{
  mesh_t * mesh = nekrs::temperatureMesh();

  double * scratch = (double *)calloc(_moose_Nq * mesh->Nq, sizeof(double));

  nekrs::interpolateSurfaceFaceHex3D(
      scratch, _interpolation_incoming, incoming_moose_value, _moose_Nq, outgoing_nek_value, mesh->Nq);

  freePointer(scratch);
}

void
NekRSProblemBase::mapFaceDataToNekFace(const unsigned int & e, const unsigned int & var_num,
  const Real & multiplier, double ** outgoing_data)
{
  auto sys_number = _aux->number();
  auto & mesh = _nek_mesh->getMesh();
  auto indices = _nek_mesh->cornerIndices();

  for (int build = 0; build < _nek_mesh->nMoosePerNek(); ++build)
  {
    auto elem_ptr = mesh.query_elem_ptr(e * _nek_mesh->nMoosePerNek() + build);

    // Only work on elements we can find on our local chunk of a
    // distributed mesh
    if (!elem_ptr)
    {
      libmesh_assert(!mesh.is_serial());
      continue;
    }

    for (unsigned int n = 0; n < _n_vertices_per_surface; n++)
    {
      auto node_ptr = elem_ptr->node_ptr(n);

      // convert libMesh node index into the ordering used by NekRS
      int node_index = _nek_mesh->exactMirror() ?
        indices[build][_nek_mesh->boundaryNodeIndex(n)] : _nek_mesh->boundaryNodeIndex(n);

      auto dof_idx = node_ptr->dof_number(sys_number, var_num, 0);
      (*outgoing_data)[node_index] = (*_serialized_solution)(dof_idx) * multiplier;
    }
  }
}

void
NekRSProblemBase::mapVolumeDataToNekVolume(const unsigned int & e, const unsigned int & var_num,
  const Real & multiplier, double ** outgoing_data)
{
  auto sys_number = _aux->number();
  auto & mesh = _nek_mesh->getMesh();
  auto indices = _nek_mesh->cornerIndices();

  for (int build = 0; build < _nek_mesh->nMoosePerNek(); ++build)
  {
    auto elem_ptr = mesh.query_elem_ptr(e * _nek_mesh->nMoosePerNek() + build);

    // Only work on elements we can find on our local chunk of a
    // distributed mesh
    if (!elem_ptr)
    {
      libmesh_assert(!mesh.is_serial());
      continue;
    }

    for (unsigned int n = 0; n < _n_vertices_per_volume; n++)
    {
      auto node_ptr = elem_ptr->node_ptr(n);

      // convert libMesh node index into the ordering used by NekRS
      int node_index = _nek_mesh->exactMirror() ?
        indices[build][_nek_mesh->volumeNodeIndex(n)] : _nek_mesh->volumeNodeIndex(n);

      auto dof_idx = node_ptr->dof_number(sys_number, var_num, 0);
      (*outgoing_data)[node_index] = (*_serialized_solution)(dof_idx) * multiplier;
    }
  }
}

void
NekRSProblemBase::mapFaceDataToNekVolume(const unsigned int & e, const unsigned int & var_num,
  const Real & multiplier, double ** outgoing_data)
{
  auto sys_number = _aux->number();
  auto & mesh = _nek_mesh->getMesh();
  auto indices = _nek_mesh->cornerIndices();

  for (int build = 0; build < _nek_mesh->nMoosePerNek(); ++build)
  {
    int n_faces_on_boundary = _nek_mesh->facesOnBoundary(e);

    // the only meaningful values are on the coupling boundaries, so we can skip this
    // interpolation if this volume element isn't on a coupling boundary
    if (n_faces_on_boundary > 0)
    {
      auto elem_ptr = mesh.query_elem_ptr(e * _nek_mesh->nMoosePerNek() + build);

      // Only work on elements we can find on our local chunk of a
      // distributed mesh
      if (!elem_ptr)
      {
        libmesh_assert(!mesh.is_serial());
        continue;
      }

      for (unsigned int n = 0; n < _n_vertices_per_volume; ++n)
      {
        auto node_ptr = elem_ptr->node_ptr(n);

        // convert libMesh node index into the ordering used by NekRS
        int node_index = _nek_mesh->exactMirror() ?
          indices[build][_nek_mesh->volumeNodeIndex(n)] : _nek_mesh->volumeNodeIndex(n);

        auto dof_idx = node_ptr->dof_number(sys_number, var_num, 0);
        (*outgoing_data)[node_index] = (*_serialized_solution)(dof_idx) * multiplier;
      }
    }
  }
}

void
NekRSProblemBase::copyScratchToDevice()
{
  if (_minimum_scratch_size_for_coupling + _n_uo_slots > 0)
  {
    auto n = nekrs::scalarFieldOffset();
    auto nbytes = n * sizeof(dfloat);

    nrs_t * nrs = (nrs_t *)nekrs::nrsPtr();
    nrs->o_usrwrk.copyFrom(nrs->usrwrk + _first_reserved_usrwrk_slot * n,
                           (_minimum_scratch_size_for_coupling + _n_uo_slots) * nbytes,
                           _first_reserved_usrwrk_slot * nbytes);
  }

  if (nekrs::hasMovingMesh())
    nekrs::copyDeformationToDevice();
}

#endif
