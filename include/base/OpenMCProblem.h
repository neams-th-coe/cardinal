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
public:
  OpenMCProblem(const InputParameters & params);
  virtual ~OpenMCProblem() override {}

  virtual void addExternalVariables() override;
  virtual void externalSolve() override;
  virtual void syncSolutions(ExternalProblem::Direction direction) override;

  virtual bool converged() override { return true; }

  static int32_t getFilterId();
  static int32_t getNewFilter(int32_t filterId, const char *type);
  static int32_t getTallyId();
  static int32_t getNewTally(int32_t tallyId);
  xt::xtensor<double, 1> heat_source();
  double get_cell_volume(int cellIndex);

private:
  std::vector<Point> _centers;
  Real _power;
  std::vector<Real> _volumes;

  int32_t _filterId;     //! ID for cell filter in OpenMC
  int32_t _filterIndex;  //! Index for cell filter in OpenMC's filter array
  int32_t _tallyId;
  int32_t _tallyIndex;

  openmc::CellFilter *_filter;
  std::unique_ptr<openmc::Tally> &_tally;

  std::vector<int32_t> _cellIndices {};
  std::vector<int32_t> _cellInstances {};
  std::vector<std::vector<int32_t>> _pebbleCells {};
};

#endif //CARDINAL_OPENMCPROBLEM_H
