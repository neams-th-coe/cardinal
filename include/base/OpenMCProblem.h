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

#pragma once

#define LIBMESH

#include "OpenMCProblemBase.h"
#include "openmc/tallies/filter_cell.h"
#include "openmc/tallies/filter_mesh.h"
#include "openmc/mesh.h"
#include "openmc/tallies/tally.h"
#include "CardinalEnums.h"

class OpenMCProblem : public OpenMCProblemBase
{
public:
  OpenMCProblem(const InputParameters & params);
  static InputParameters validParams();

  virtual ~OpenMCProblem() override;

  virtual void addExternalVariables() override;
  virtual void externalSolve() override;
  virtual void syncSolutions(ExternalProblem::Direction direction) override;

  virtual void initialSetup() override;

  virtual bool converged() override { return true; }

  /// Get the center coordinates for all cells of interest
  virtual void fillCenters();

  //! Create all tallies needed for transfers and problem checks
  void setupTallies();

  //! Creates a cell-based tally with a value for each pebble
  void setupCellTally();
  //! Creates an unstructured mesh tally using a template
  //! translated to the center of each pebble
  void setupMeshTallies();

  //! Get the global kappa-fission value for the probmlem
  double kappa_fission_total() const;

  //! Check that the tally sum matches the global kappa-fission tally
  void checkTallySum(double tally_sum) const;

  // Retrieves cell-based tally values
  // and contstructs a heat source
  std::vector<double> cellHeatSource();
  // Retrieves unstructured mesh tally values
  // and constructs a heat source
  std::vector<double> meshHeatSource();
  double getCellVolume(int cellIndex);

protected:
  /// coordinate level of the pebble cells in the OpenMC model
  const int & _pebble_cell_level;

  /// Cell volumes at the locations of the pebble centers
  std::vector<Real> _volumes;

  /// Tally type used in OpenMC
  const tally::TallyTypeEnum _tallyType;

  /// Conversion factor from electron volts to Joules
  const double JOULE_PER_EV {1.6021766208e-19};

  /// Spatial dimension of the Monte Carlo problem
  const int DIMENSION {3};

  unsigned int _heat_source_var; //! heat source variable number

  std::vector<Point> _centers;       //! Locations of the pebble centers
  std::string _meshTemplateFilename; //! Filename of the mesh template to use in the unstructured mesh tally

  std::vector<int32_t> _cellIndices {};   //! OpenMC cell indices corresponding to the pebble centers
  std::vector<int32_t> _cellInstances {}; //! OpenMC cell instances corresponding to the pebble centers

  bool _check_tally_sum; //! Check tally consistency during transfers

  const openmc::LibMesh* _meshTemplate;                //! OpenMC unstructured mesh instance
  std::vector<const openmc::CellFilter*> _cellFilters; //! OpenMC cell filters
  std::vector<const openmc::MeshFilter*> _meshFilters; //! OpenMC mesh filters
  std::vector<const openmc::Tally*> _tallies;          //! OpenMC tally instances
  const openmc::Tally* _kappa_fission_tally {nullptr}; //! Global kappa-fission tally
};

