#ifndef CARDINAL_OPENMCPROBLEM_H
#define CARDINAL_OPENMCPROBLEM_H

#define LIBMESH

#include "ExternalProblem.h"
#include "openmc/tallies/filter_cell.h"
#include "openmc/tallies/filter_mesh.h"
#include "openmc/mesh.h"
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
  std::vector<double> mesh_heat_source();
  xt::xtensor<double, 1> heat_source();
  double get_cell_volume(int cellIndex);

private:
  std::vector<Point> _centers;
  Real _power;
  std::vector<Real> _volumes;
  std::string _mesh_template_filename;

  int32_t _filterId;     //! ID for cell filter in OpenMC
  int32_t _filterIndex;  //! Index for cell filter in OpenMC's filter array
  int32_t _tallyId;
  int32_t _tallyIndex;

  openmc::CellFilter *_filter;

  std::vector<int32_t> _cellIndices {};
  std::vector<int32_t> _cellInstances {};

  const openmc::LibMesh* _mesh_template;               //! OpenMC unstructured mesh instance
  std::vector<const openmc::MeshFilter*> _meshFilters; //! OpenMC mesh filters
  std::vector<const openmc::Tally*> _meshTallies;      //! OpenMC mesh tallies
};

#endif //CARDINAL_OPENMCPROBLEM_H
