#ifndef CARDINAL_OPENMCPROBLEM_H
#define CARDINAL_OPENMCPROBLEM_H

#include "ExternalProblem.h"
#include "openmc/tallies/filter_cell.h"
#include "openmc/tallies/tally.h"

class OpenMCProblem;

template<>
InputParameters validParams<OpenMCProblem>();

class OpenMCProblem : public ExternalProblem
{
private:
  std::vector<Point> _centers;

  int32_t _filterId;     //! ID for cell filter in OpenMC
  int32_t _filterIndex;  //! Index for cell filter in OpenMC's filter array
  int32_t _tallyId;      //! 
  int32_t _tallyIndex;

  openmc::CellFilter *_filter;
  std::unique_ptr<openmc::Tally> &_tally;

  std::vector<int32_t> _cellIndices {};
  std::vector<int32_t> _cellInstances {};

public:
  OpenMCProblem(const InputParameters & params);
  virtual ~OpenMCProblem() override {}

  virtual void externalSolve() override;
  virtual void syncSolutions(ExternalProblem::Direction direction) override;

  virtual bool converged() override { return true; }
  virtual void addExternalVariables() override {}

  static int32_t getFilterId();
  static int32_t getNewFilter(int32_t filterId, const char *type);
  static int32_t getTallyId();
  static int32_t getNewTally(int32_t tallyId);
  xt::xtensor<double, 1> heat_source(double power);
  double get_cell_volume(int cellIndex);
};

#endif //CARDINAL_OPENMCPROBLEM_H
