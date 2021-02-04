#ifndef CARDINAL_OPENMCPROBLEM_H
#define CARDINAL_OPENMCPROBLEM_H

#define LIBMESH

#include "ExternalProblem.h"
#include "openmc/tallies/filter_cell.h"
#include "openmc/tallies/filter_mesh.h"
#include "openmc/mesh.h"
#include "openmc/tallies/tally.h"
#include "CardinalEnums.h"

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

  //! Creates a cell-based tally with a value for each pebble
  void setupCellTally();
  //! Creates an unstructured mesh tally using a template
  //! translated to the center of each pebble
  void setupMeshTallies();

  // Retrieves cell-based tally values
  // and contstructs a heat source
  std::vector<double> cellHeatSource();
  // Retrieves unstructured mesh tally values
  // and constructs a heat source
  std::vector<double> meshHeatSource();
  double getCellVolume(int cellIndex);

private:
  const double JOULE_PER_EV {1.6021766208e-19};

  unsigned int _heat_source_var; //! heat source variable number
  int _pebble_cell_level; //! coordinate level of the pebble cells in the OpenMC model

  std::vector<Point> _centers;       //! Locations of the pebble centers
  Real _power;                       //! Total power produced in the problem (used for heating normalization)
  std::vector<Real> _volumes;        //! Cell volumes at the location of the pebble centers
  std::string _meshTemplateFilename; //! Filename of the mesh template to use in the unstructured mesh tally
  const tally::TallyTypeEnum _tallyType;              //! Tally type used in the OpenMC problem (CELL, MESH)

  std::vector<int32_t> _cellIndices {};   //! OpenMC cell indices corresponding to the pebble centers
  std::vector<int32_t> _cellInstances {}; //! OpenMC cell instances corresponding to the pebble centers

  const openmc::LibMesh* _meshTemplate;                //! OpenMC unstructured mesh instance
  std::vector<const openmc::CellFilter*> _cellFilters; //! OpenMC cell filters
  std::vector<const openmc::MeshFilter*> _meshFilters; //! OpenMC mesh filters
  std::vector<const openmc::Tally*> _tallies;          //! OpenMC tally instances
};

#endif //CARDINAL_OPENMCPROBLEM_H
