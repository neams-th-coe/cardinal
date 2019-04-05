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

  int32_t _filter_id;     //! ID for cell filter in OpenMC
  int32_t _filter_index;  //! Index for cell filter in OpenMC's filter array
  int32_t _tally_id;      //! 
  int32_t _tally_index;

  openmc::CellFilter *_filter;
  std::unique_ptr<openmc::Tally> &_tally;

  std::vector<int32_t> _cell_indices {};
  std::vector<int32_t> _cell_instances {};

public:
  OpenMCProblem(const InputParameters & params);
  virtual ~OpenMCProblem() override {}

  virtual void externalSolve() override;
  virtual void syncSolutions(ExternalProblem::Direction direction) override;

  virtual bool converged() override { return true; }
  virtual void addExternalVariables() override {}

  static int32_t get_filter_id();
  static int32_t get_new_filter(int32_t filter_id, const char *type);
  static int32_t get_tally_id();
  static int32_t get_new_tally(int32_t tally_id);
};

#endif //CARDINAL_OPENMCPROBLEM_H
