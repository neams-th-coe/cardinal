#include "NekInterface.h"
#include "nekrs.cpp"

namespace nekrs
{

bool endControlElapsedTime()
{
  return !nrs->options.getArgs("STOP AT ELAPSED TIME").empty();
}

bool endControlTime()
{
  return endTime() > 0;
}

bool endControlNumSteps()
{
  return !endControlElapsedTime() && !endControlTime();
}

} // end namespace nekrs
