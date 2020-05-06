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

  void setupCellTally();
  void setupMeshTallies();

  std::vector<double> mesh_heat_source();
  std::vector<double> heat_source();
  double get_cell_volume(int cellIndex);

private:

  enum class TallyType {
    CELL,
    MESH,
  };

  std::vector<Point> _centers;
  Real _power;
  std::vector<Real> _volumes;
  std::string _meshTemplateFilename;
  TallyType _tallyType;

  openmc::CellFilter *_filter;

  std::vector<int32_t> _cellIndices {};
  std::vector<int32_t> _cellInstances {};

  const openmc::LibMesh* _meshTemplate;               //! OpenMC unstructured mesh instance
  std::vector<const openmc::MeshFilter*> _meshFilters; //! OpenMC mesh filters
  std::vector<const openmc::CellFilter*> _cellFilters;
  std::vector<const openmc::Tally*> _tallies;
};

#endif //CARDINAL_OPENMCPROBLEM_H
