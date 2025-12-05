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
#include "CardinalUtils.h"
#include "DimensionalizeAction.h"
#include "FieldTransferBase.h"
#include "NekMeshDeformation.h"

registerMooseObject("CardinalApp", NekRSProblem);

bool NekRSProblem::_first = true;

InputParameters
NekRSProblem::validParams()
{
  InputParameters params = CardinalProblem::validParams();
  params.addRequiredParam<std::string>(
      "casename",
      "Case name for the NekRS input files; "
      "this is <case> in <case>.par, <case>.udf, <case>.oudf, and <case>.re2.");

  params.addParam<unsigned int>(
      "n_usrwrk_slots",
      0,
      "Number of slots to allocate in nrs->usrwrk to hold fields either related to coupling "
      "(which will be populated by Cardinal), or other custom usages, such as a distance-to-wall "
      "calculation (which will be populated by the user from the case files)");

  params.addParam<std::vector<unsigned int>>(
      "usrwrk_output",
      "Usrwrk slot(s) to output to NekRS field files; this can be used for viewing the quantities "
      "passed from MOOSE to NekRS after interpolation to the CFD mesh. Can also be used for any "
      "slots "
      "in usrwrk that are written by the user, but unused for coupling.");
  params.addParam<std::vector<std::string>>(
      "usrwrk_output_prefix",
      "String prefix to use for naming the field file(s); "
      "only the first three characters are used in the name based on limitations in NekRS");

  params.addParam<bool>(
      "write_fld_files",
      false,
      "Whether to write NekRS field file output "
      "from Cardinal. If true, this will disable any output writing by NekRS itself, and "
      "instead produce output files with names a01...a99pin, b01...b99pin, etc.");
  params.addParam<bool>(
      "disable_fld_file_output", false, "Whether to turn off all NekRS field file output writing");

  params.addParam<bool>("skip_final_field_file",
                        false,
                        "By default, we write a NekRS field file "
                        "on the last time step; set this to true to disable");

  params.addParam<MooseEnum>(
      "synchronization_interval",
      getSynchronizationEnum(),
      "When to synchronize the NekRS solution with the mesh mirror. By default, the NekRS solution "
      "is mapped to/receives data from the mesh mirror for every time step.");
  params.addParam<unsigned int>("constant_interval",
                                1,
                                "Constant interval (in units of number of time steps) with which "
                                "to synchronize the NekRS solution");
  return params;
}

NekRSProblem::NekRSProblem(const InputParameters & params)
  : CardinalProblem(params),
    _serialized_solution(NumericVector<Number>::build(_communicator).release()),
    _casename(getParam<std::string>("casename")),
    _write_fld_files(getParam<bool>("write_fld_files")),
    _disable_fld_file_output(getParam<bool>("disable_fld_file_output")),
    _n_usrwrk_slots(getParam<unsigned int>("n_usrwrk_slots")),
    _constant_interval(getParam<unsigned int>("constant_interval")),
    _skip_final_field_file(getParam<bool>("skip_final_field_file")),
    _start_time(nekrs::startTime()),
    _elapsedStepSum(0.0),
    _elapsedTime(nekrs::getNekSetupTime()),
    _tSolveStepMin(std::numeric_limits<double>::max()),
    _tSolveStepMax(std::numeric_limits<double>::min())
{
  const auto & actions = getMooseApp().actionWarehouse().getActions<DimensionalizeAction>();
  _nondimensional = actions.size();
  nekrs::nondimensional(_nondimensional);

  _sync_interval = getParam<MooseEnum>("synchronization_interval")
                       .getEnum<synchronization::SynchronizationEnum>();
  if (_sync_interval == synchronization::parent_app)
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
      _sync_interval = synchronization::constant;
    }

    checkUnusedParam(params, "constant_interval", "synchronizing based on the 'parent_app'");
  }

  if (_disable_fld_file_output && _write_fld_files)
    mooseError("Cannot both disable all field file output and write custom field files! "
               "'write_fld_files' and 'disable_fld_file_output' cannot both be true!");

  if (_app.isUltimateMaster() && _write_fld_files)
    mooseError("The 'write_fld_files' setting should only be true when multiple Nek simulations "
               "are run as sub-apps on a master app. Your input has Nek as the master app.");

  _nek_mesh = dynamic_cast<NekRSMesh *>(&mesh());

  if (!_nek_mesh)
    mooseError("The mesh for NekRSProblem must be of type 'NekRSMesh', but you have specified a '" +
               mesh().type() + "'!");

  // The mesh movement error checks are triggered based on whether the NekRS input files
  // have a moving mesh. From there, we impose the necessary checks on the [Mesh] block
  // and the existence of the NekMeshDeformation object.
  if (nekrs::hasMovingMesh())
  {
    if (!_nek_mesh->getMesh().is_replicated())
      mooseError("Distributed mesh features are not yet implemented for moving mesh cases!");
  }

  _moose_Nq = _nek_mesh->order() + 2;

  // the Problem constructor is called right after building the mesh. In order
  // to have pretty screen output without conflicting with the timed print messages,
  // print diagnostic info related to the mesh here. If running in JIT mode, this
  // diagnostic info was never set, so the numbers that would be printed are garbage.
  if (!nekrs::buildOnly())
    _nek_mesh->printMeshInfo();

  // boundary-specific data
  _n_surface_elems = _nek_mesh->numSurfaceElems();
  _n_vertices_per_surface = _nek_mesh->numVerticesPerSurface();

  // volume-specific data
  _n_vertices_per_volume = _nek_mesh->numVerticesPerVolume();

  if (_nek_mesh->volume())
    _n_points =
        _nek_mesh->numVolumeElems() * _n_vertices_per_volume * _nek_mesh->nBuildPerVolumeElem();
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

  checkJointParams(
      params, {"usrwrk_output", "usrwrk_output_prefix"}, "outputting usrwrk slots to field files");

  if (isParamValid("usrwrk_output"))
  {
    _usrwrk_output = &getParam<std::vector<unsigned int>>("usrwrk_output");
    _usrwrk_output_prefix = &getParam<std::vector<std::string>>("usrwrk_output_prefix");

    for (const auto & s : *_usrwrk_output)
      if (s >= _n_usrwrk_slots)
        mooseError("Cannot write field file for usrwrk slot greater than the total number of "
                   "allocated slots: ",
                   _n_usrwrk_slots,
                   "! Please increase 'n_usrwrk_slots'.");

    if (_usrwrk_output->size() != _usrwrk_output_prefix->size())
      mooseError("The length of 'usrwrk_output' must match the length of 'usrwrk_output_prefix'!");
  }
}

NekRSProblem::~NekRSProblem()
{
  // write nekRS solution to output if not already written for this step; nekRS does this
  // behavior, so we duplicate it
  if (!_is_output_step && !_skip_final_field_file)
  {
    if (_write_fld_files)
      mooseWarning(
          "When 'write_fld_files' is enabled, we skip Nek field file writing on end time!\n"
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

  freePointer(_interpolation_outgoing);
  freePointer(_interpolation_incoming);
  nekrs::freeScratch();

  nekrs::finalize();
}

void
NekRSProblem::writeFieldFile(const Real & step_end_time, const int & step) const
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
      mooseError("Internal error: app name '" + _app.name() +
                 "' does not end with app number: " + app_number);

    auto name = _app.name().substr(0, _app.name().size() - app_number.size());
    auto full_path = _app.getOutputFileBase();
    std::string last_element(full_path.substr(full_path.rfind(name) + name.size()));

    auto prefix = fieldFilePrefix(std::stoi(last_element));

    nekrs::write_field_file(prefix, t, step);
  }
  else
    nekrs::writeCheckpoint(t);
}

void
NekRSProblem::initializeInterpolationMatrices()
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
NekRSProblem::fieldFilePrefix(const int & number) const
{
  const std::string alphabet = "abcdefghijklmnopqrstuvwxyz";
  int letter = number / 26;
  int remainder = number % 100;
  std::string s = remainder < 10 ? "0" : "";

  return alphabet[letter] + s + std::to_string(remainder);
}

void
NekRSProblem::initialSetup()
{
  CardinalProblem::initialSetup();

  auto executioner = _app.getExecutioner();
  _transient_executioner = dynamic_cast<Transient *>(executioner);

  // NekRS only supports transient simulations - therefore, it does not make
  // sense to use anything except a Transient-derived executioner
  if (!_transient_executioner)
    mooseError(
        "A 'Transient' executioner must be used with NekRSProblem, but you have specified the '" +
        executioner->type() + "' executioner!");

  // To get the correct time stepping information on the MOOSE side, we also
  // must use the NekTimeStepper
  TimeStepper * stepper = _transient_executioner->getTimeStepper();
  _timestepper = dynamic_cast<NekTimeStepper *>(stepper);
  if (!_timestepper)
    mooseError("The 'NekTimeStepper' stepper must be used with NekRSProblem, but you have "
               "specified the '" +
               stepper->type() + "' time stepper!");

  // Set the NekRS start time to whatever is set on Executioner/start_time; print
  // a message if those times don't match the .par file
  const auto moose_start_time = _transient_executioner->getStartTime();
  nekrs::setStartTime(_timestepper->nondimensionalDT(moose_start_time));
  _start_time = moose_start_time;

  if (_sync_interval == synchronization::parent_app)
    _transfer_in = &getPostprocessorValueByName("transfer_in");

  // Find all of the data transfer objects
  TheWarehouse::Query query = theWarehouse().query().condition<AttribSystem>("FieldTransfer");
  query.queryInto(_field_transfers);

  // Find all of the scalar data transfer objects
  TheWarehouse::Query uo_query = theWarehouse().query().condition<AttribSystem>("ScalarTransfer");
  uo_query.queryInto(_scalar_transfers);

  // We require a NekMeshDeformation object to exist if the NekRS model has a moving mesh
  if (nekrs::hasMovingMesh())
  {
    bool has_deformation = false;
    for (const auto & t : _field_transfers)
    {
      NekMeshDeformation * deform = dynamic_cast<NekMeshDeformation *>(t);
      if (deform)
        has_deformation = true;
    }

    if (has_deformation && !_app.actionWarehouse().displacedMesh())
      mooseError("Moving mesh problems require 'displacements' in the [Mesh] block! The names of "
                 "the 'displacements' variables must match the variables created by a "
                 "NekMeshDeformation object.");
  }

  // save initial mesh for moving mesh problems to match deformation in exodus output files
  if (nekrs::hasMovingMesh() && !_disable_fld_file_output)
    nekrs::writeCheckpoint(_timestepper->nondimensionalDT(_time));

  VariadicTable<int, std::string, std::string, std::string> vt(
      {"Slot", "Data Written", "How to Access (.oudf)", "How to Access (.udf)"});

  // fill a set with all of the slots managed by Cardinal, coming from either field transfers
  // or userobjects
  auto field_usrwrk_map = FieldTransferBase::usrwrkMap();
  auto field_usrwrk_scales = FieldTransferBase::usrwrkScales();
  for (const auto & field : field_usrwrk_map)
    _usrwrk_slots.insert(field.first);
  for (const auto & uo : _scalar_transfers)
    _usrwrk_slots.insert(uo->usrwrkSlot());

  // fill out table, being careful to only write information if owned by a field transfer,
  // a user object, or neither
  for (int i = 0; i < _n_usrwrk_slots; ++i)
  {
    std::string oudf = "bc->usrwrk[" + std::to_string(i) + "*bc->fieldOffset+bc->idM]";
    std::string udf = "nrs->usrwrk[" + std::to_string(i) + "*nrs->fieldOffset+n]";

    if (field_usrwrk_map.find(i) != field_usrwrk_map.end())
    {
      // a field transfer owns it
      auto scales = field_usrwrk_scales[field_usrwrk_map[i]];

      std::string top;
      if (MooseUtils::absoluteFuzzyEqual(scales.first, 0.0))
        top = field_usrwrk_map[i];
      else
        top = field_usrwrk_map[i] + "-" + std::to_string(scales.first);

      if (!MooseUtils::absoluteFuzzyEqual(scales.second, 1.0))
      {
        if (MooseUtils::absoluteFuzzyEqual(scales.first, 0.0))
          top = "(" + top + ")/" + std::to_string(scales.second);
        else
          top = top + "/" + std::to_string(scales.second);
      }

      vt.addRow(i, top, oudf, udf);
    }
    else
    {
      // a user object might own it, or it could be unused
      bool owned_by_uo = false;
      for (const auto & uo : _scalar_transfers)
      {
        if (uo->usrwrkSlot() == i)
        {
          owned_by_uo = true;
          auto slot = std::to_string(uo->usrwrkSlot());
          auto count = std::to_string(uo->offset());

          std::string top = uo->name();
          if (!MooseUtils::absoluteFuzzyEqual(uo->scaling(), 1.0))
            top += top + "*" + std::to_string(uo->scaling());

          vt.addRow(i,
                    top,
                    "bc->usrwrk[" + slot + "*bc->fieldOffset+" + count + "]",
                    "nrs->usrwrk[" + slot + "*nrs->fieldOffset+" + count + "]");
        }
      }

      if (!owned_by_uo)
        vt.addRow(i, "unused", oudf, udf);
    }
  }

  if (_n_usrwrk_slots == 0)
  {
    _console << "Skipping allocation of NekRS scratch space because 'n_usrwrk_slots' is 0\n"
             << std::endl;
  }
  else
  {
    _console
        << "\n ===================>     MAPPING FROM MOOSE TO NEKRS      <===================\n"
        << std::endl;
    _console << "           Slot:  slice in scratch space holding the data\n" << std::endl;
    _console << "   Data written:  data that gets written into this slot. This data is shown"
             << std::endl;
    _console << "                  in the form actually written into NekRS (which will be"
             << std::endl;
    _console << "                  non-dimensional quantities if using the [Dimensionalize]"
             << std::endl;
    _console << "                  block). Words refer to MOOSE AuxVariables/Postprocessors."
             << std::endl;
    _console << "                  If 'unused', this means that the space has been allocated,"
             << std::endl;
    _console << "                  but Cardinal is not otherwise using it for coupling.\n"
             << std::endl;
    _console << "  How to Access:  C++ code to use in NekRS files; for the .udf instructions,"
             << std::endl;
    _console << "                  'n' indicates a loop variable over GLL points\n" << std::endl;
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

  nekrs::udfExecuteStep(
      _timestepper->nondimensionalDT(_start_time), _t_step, false /* not an output step */);
  nekrs::resetTimer("udfExecuteStep");
}

void
NekRSProblem::externalSolve()
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
  nekrs::checkpointStep(_is_output_step);

  // NekRS prints out verbose info for the first 1000 time steps
  if (_t_step <= 1000)
    platform->options.setArgs("VERBOSE","TRUE");

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
  auto nrs = nekrs::nrsPtr();
  nrs->copyToNek(_timestepper->nondimensionalDT(step_end_time), _t_step);

  if (nekrs::printStepInfoFreq())
    if (_t_step % nekrs::printStepInfoFreq() == 0)
      nekrs::printStepInfo(_timestepper->nondimensionalDT(_time), _t_step, false, true);

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

        nekrs::write_usrwrk_field_file((*_usrwrk_output)[i],
                                       (*_usrwrk_output_prefix)[i],
                                       _timestepper->nondimensionalDT(step_end_time),
                                       _t_step,
                                       write_coords);

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

  if (nekrs::printStepInfoFreq())
    if (_t_step % nekrs::printStepInfoFreq() == 0)
      nekrs::printStepInfo(_timestepper->nondimensionalDT(_time), _t_step, true, false);

  if (nekrs::runTimeStatFreq())
    if (_t_step % nekrs::runTimeStatFreq() == 0)
      nekrs::printRuntimeStatistics(_t_step);

  _time += _dt;
}

bool
NekRSProblem::isDataTransferHappening(ExternalProblem::Direction direction)
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
      mooseError("Unhandled DirectionEnum in NekRSProblem!");
  }
}

void
NekRSProblem::syncSolutions(ExternalProblem::Direction direction)
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

      // execute all incoming field transfers
      for (const auto & t : _field_transfers)
        if (t->direction() == "to_nek")
          t->sendDataToNek();

      // execute all incoming scalar transfers
      for (const auto & t : _scalar_transfers)
        if (t->direction() == "to_nek")
          t->sendDataToNek();

      auto nrs = nekrs::nrsPtr();
      if (nrs->userProperties)
      {
        nrs->evaluateProperties(_timestepper->nondimensionalDT(_time));
      }

      copyScratchToDevice();

      break;

      return;
    }
    case ExternalProblem::Direction::FROM_EXTERNAL_APP:
    {
      // execute all outgoing field transfers
      for (const auto & t : _field_transfers)
        if (t->direction() == "from_nek")
          t->readDataFromNek();

      // execute all outgoing scalar transfers
      for (const auto & t : _scalar_transfers)
        if (t->direction() == "from_nek")
          t->sendDataToNek();

      break;
    }
    default:
      mooseError("Unhandled Transfer::DIRECTION enum!");
  }

  solution.close();
  _aux->system().update();
}

bool
NekRSProblem::synchronizeIn()
{
  bool synchronize = true;

  switch (_sync_interval)
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
      mooseError("Unhandled SynchronizationEnum in NekRSProblem!");
  }

  return synchronize;
}

bool
NekRSProblem::synchronizeOut()
{
  bool synchronize = true;

  switch (_sync_interval)
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
      mooseError("Unhandled SynchronizationEnum in NekRSProblem!");
  }

  return synchronize;
}

bool
NekRSProblem::isOutputStep() const
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

  return nekrs::checkpointStep(_timestepper->nondimensionalDT(_time), _t_step);
}

void
NekRSProblem::addExternalVariables()
{
  // Creation of variables for data transfers is handled by the FieldTransferBase objects

  if (_sync_interval == synchronization::parent_app)
  {
    auto pp_params = _factory.getValidParams("Receiver");
    pp_params.set<std::vector<OutputName>>("outputs") = {"none"};

    // we do not need to check for duplicate names because MOOSE already handles it
    addPostprocessor("Receiver", "transfer_in", pp_params);
  }
}

void
NekRSProblem::interpolateVolumeSolutionToNek(const int elem_id,
                                             double * incoming_moose_value,
                                             double * outgoing_nek_value)
{
  mesh_t * mesh = nekrs::entireMesh();

  nekrs::interpolateVolumeHex3D(
      _interpolation_incoming, incoming_moose_value, _moose_Nq, outgoing_nek_value, mesh->Nq);
}

void
NekRSProblem::interpolateBoundarySolutionToNek(double * incoming_moose_value,
                                               double * outgoing_nek_value)
{
  mesh_t * mesh = nekrs::temperatureMesh();

  double * scratch = (double *)calloc(_moose_Nq * mesh->Nq, sizeof(double));

  nekrs::interpolateSurfaceFaceHex3D(scratch,
                                     _interpolation_incoming,
                                     incoming_moose_value,
                                     _moose_Nq,
                                     outgoing_nek_value,
                                     mesh->Nq);

  freePointer(scratch);
}

void
NekRSProblem::copyScratchToDevice()
{
  for (const auto & slot : _usrwrk_slots)
    copyIndividualScratchSlot(slot);

  if (nekrs::hasMovingMesh())
    nekrs::copyDeformationToDevice();
}

bool
NekRSProblem::isUsrWrkSlotReservedForCoupling(const unsigned int & slot) const
{
  return std::find(_usrwrk_slots.begin(), _usrwrk_slots.end(), slot) != _usrwrk_slots.end();
}

void
NekRSProblem::copyIndividualScratchSlot(const unsigned int & slot) const
{
  auto n = nekrs::fieldOffset();
  auto nbytes = n * sizeof(dfloat);

  auto nrs = nekrs::nrsPtr();
  platform->app->bc->o_usrwrk.copyFrom(nekrs::host_wrk() + slot * n, nbytes, slot * nbytes);
}

void
NekRSProblem::mapFaceDataToNekFace(const unsigned int & e,
                                   const unsigned int & var_num,
                                   const Real & divisor_scale,
                                   const Real & additive_scale,
                                   double ** outgoing_data)
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
      int node_index = _nek_mesh->exactMirror() ? indices[build][_nek_mesh->boundaryNodeIndex(n)]
                                                : _nek_mesh->boundaryNodeIndex(n);

      auto dof_idx = node_ptr->dof_number(sys_number, var_num, 0);
      (*outgoing_data)[node_index] =
          ((*_serialized_solution)(dof_idx)-additive_scale) / divisor_scale;
    }
  }
}

void
NekRSProblem::mapFaceDataToNekVolume(const unsigned int & e,
                                     const unsigned int & var_num,
                                     const Real & divisor_scale,
                                     const Real & additive_scale,
                                     double ** outgoing_data)
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
        int node_index = _nek_mesh->exactMirror() ? indices[build][_nek_mesh->volumeNodeIndex(n)]
                                                  : _nek_mesh->volumeNodeIndex(n);

        auto dof_idx = node_ptr->dof_number(sys_number, var_num, 0);
        (*outgoing_data)[node_index] =
            ((*_serialized_solution)(dof_idx)-additive_scale) / divisor_scale;
      }
    }
  }
}

void
NekRSProblem::mapVolumeDataToNekVolume(const unsigned int & e,
                                       const unsigned int & var_num,
                                       const Real & divisor,
                                       const Real & additive,
                                       double ** outgoing_data)
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
      int node_index = _nek_mesh->exactMirror() ? indices[build][_nek_mesh->volumeNodeIndex(n)]
                                                : _nek_mesh->volumeNodeIndex(n);

      auto dof_idx = node_ptr->dof_number(sys_number, var_num, 0);
      (*outgoing_data)[node_index] = ((*_serialized_solution)(dof_idx)-additive) / divisor;
    }
  }
}

void
NekRSProblem::writeVolumeDisplacement(const int elem_id,
                                      double * s,
                                      const field::NekWriteEnum f,
                                      const std::vector<double> * add)
{
  mesh_t * mesh = nekrs::entireMesh();
  auto nrs = nekrs::nrsPtr();

  auto vc = _nek_mesh->volumeCoupling();
  int id = vc.element[elem_id] * mesh->Np;

  auto [x, y, z] = nekrs::host_xyz();

  if (_nek_mesh->exactMirror())
  {
    // can write directly into the NekRS solution
    for (int v = 0; v < mesh->Np; ++v)
    {
      double extra = (add == nullptr) ? 0.0 : (*add)[id + v];

      if (f == field::x_displacement)
        x[id + v] = s[v] + extra;
      else if (f == field::y_displacement)
        y[id + v] = s[v] + extra;
      else if (f == field::z_displacement)
        z[id + v] = s[v] + extra;
      else
        mooseError("Unhandled NekWriteEnum in writeVolumeDisplacement!");
    }
  }
  else
  {
    // need to interpolate onto the higher-order Nek mesh
    double * tmp = (double *)calloc(mesh->Np, sizeof(double));

    interpolateVolumeSolutionToNek(elem_id, s, tmp);

    for (int v = 0; v < mesh->Np; ++v)
    {
      double extra = (add == nullptr) ? 0.0 : (*add)[id + v];
      if (f == field::x_displacement)
        x[id + v] = s[v] + extra;
      else if (f == field::y_displacement)
        y[id + v] = s[v] + extra;
      else if (f == field::z_displacement)
        z[id + v] = s[v] + extra;
      else
        mooseError("Unhandled NekWriteEnum in writeVolumeDisplacement!");
    }

    freePointer(tmp);
  }
}

void
NekRSProblem::writeVolumeSolution(const int slot,
                                  const int elem_id,
                                  double * s,
                                  const std::vector<double> * add)
{
  mesh_t * mesh = nekrs::entireMesh();
  auto nrs = nekrs::nrsPtr();

  auto vc = _nek_mesh->volumeCoupling();
  int id = vc.element[elem_id] * mesh->Np;

  auto usrwrk = nekrs::host_wrk();

  if (_nek_mesh->exactMirror())
  {
    // can write directly into the NekRS solution
    for (int v = 0; v < mesh->Np; ++v)
    {
      double extra = (add == nullptr) ? 0.0 : (*add)[id + v];
      usrwrk[slot + id + v] = s[v] + extra;
    }
  }
  else
  {
    // need to interpolate onto the higher-order Nek mesh
    double * tmp = (double *)calloc(mesh->Np, sizeof(double));

    interpolateVolumeSolutionToNek(elem_id, s, tmp);

    for (int v = 0; v < mesh->Np; ++v)
    {
      double extra = (add == nullptr) ? 0.0 : (*add)[id + v];
      usrwrk[slot + id + v] = tmp[v] + extra;
    }

    freePointer(tmp);
  }
}

void
NekRSProblem::writeBoundarySolution(const int slot, const int elem_id, double * s)
{
  mesh_t * mesh = nekrs::temperatureMesh();
  auto nrs = nekrs::nrsPtr();

  const auto & bc = _nek_mesh->boundaryCoupling();
  int offset = bc.element[elem_id] * mesh->Nfaces * mesh->Nfp + bc.face[elem_id] * mesh->Nfp;

  auto usrwrk = nekrs::host_wrk();

  if (_nek_mesh->exactMirror())
  {
    // can write directly into the NekRS solution
    for (int i = 0; i < mesh->Nfp; ++i)
      usrwrk[slot + mesh->vmapM[offset + i]] = s[i];
  }
  else
  {
    // need to interpolate onto the higher-order Nek mesh
    double * tmp = (double *)calloc(mesh->Nfp, sizeof(double));
    interpolateBoundarySolutionToNek(s, tmp);

    for (int i = 0; i < mesh->Nfp; ++i)
      usrwrk[slot + mesh->vmapM[offset + i]] = tmp[i];

    freePointer(tmp);
  }
}
#endif
