#pragma once

/**
 * \brief Cardinal-specific nekRS API
 *
 * nekRS ships with a rudimentary API in their nekrs namespace, but we need additional
 * functionality from within Cardinal. Many of these functions are quite basic and could
 * eventually be ported back into nekRS itself.
 **/

namespace nekrs
{

/**
 * Whether nekRS's input file intends to terminate the simulation based on a wall time
 * \return whether a wall time is used in nekRS to end the simulation
 **/
bool endControlElapsedTime();

/**
 * Whether nekRS's input file intends to terminate the simulation based on an end time
 * \return whether an end time is used in nekRS to end the simulation
 **/
bool endControlTime();

/**
 * Whether nekRS's input file intends to terminate the simulation based on a number of steps
 * \return whether a time step interval is used in nekRS to end the simulation
 **/
bool endControlNumSteps();

} // end namespace nekrs
