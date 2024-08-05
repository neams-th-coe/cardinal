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

#ifdef ENABLE_OPENMC_COUPLING
#include "AddTallyAction.h"

#include "OpenMCCellAverageProblem.h"
#include "DelimitedFileReader.h"

registerMooseAction("CardinalApp", AddTallyAction, "add_tallies");

InputParameters
AddTallyAction::validParams()
{
  auto params = MooseObjectAction::validParams();
  params.addClassDescription("Adds tally(s) for use in simulations containing an "
                             "OpenMCCellAverageProblem.");
  /**
   * These params are used to add multiple mesh tallies that use the same mesh but are
   * translated through the domain.
   */
  params.addParam<std::vector<Point>>(
      "mesh_translations",
      "Coordinates to which each mesh template should be "
      "translated, if multiple unstructured meshes "
      "are desired. Units must match those used to define the [Mesh].");
  params.addParam<std::vector<FileName>>(
      "mesh_translations_file",
      "File providing the coordinates to which each mesh "
      "template should be translated, if multiple "
      "unstructured meshes are desired. Units must match those used to define the [Mesh]");

  return params;
}

AddTallyAction::AddTallyAction(const InputParameters & parameters) : MooseObjectAction(parameters)
{
  if (_type == "MeshTally")
  {
    if (isParamValid("mesh_translations") && isParamValid("mesh_translations_file"))
      mooseError("Both 'mesh_translations' and 'mesh_translations_file' cannot be specified");

    fillMeshTranslations();
  }
  else
  {
    if (isParamValid("mesh_translations") || isParamValid("mesh_translations_file"))
      mooseError("Mesh translations only apply to mesh-based tallies. 'mesh_translations' / "
                 "'mesh_translations_file' cannot be specified");
  }
}

void
AddTallyAction::act()
{
  if (_current_task == "add_tallies")
  {
    if (_type == "MeshTally")
      for (unsigned int i = 0; i < _mesh_translations.size(); ++i)
        addMeshTally(i, _mesh_translations[i]);
    else
      addTally();
  }
}

void
AddTallyAction::addMeshTally(unsigned int instance, const Point & translation)
{
  auto openmc_problem = dynamic_cast<OpenMCCellAverageProblem *>(_problem.get());

  if (!openmc_problem)
    mooseError("The simulation must use an OpenMCCellAverageProblem when using the tally system!");

  std::string obj_name = _name;
  if (_mesh_translations.size() > 1)
  {
    obj_name += "_" + Moose::stringify(instance);
    _moose_object_pars.set<unsigned int>("instance") = instance;
    _moose_object_pars.set<Point>("mesh_translation") = translation * openmc_problem->scaling();
  }

  _moose_object_pars.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmc_problem;
  openmc_problem->addTallyObject(_type, obj_name, _moose_object_pars);
}

void
AddTallyAction::addTally()
{
  auto openmc_problem = dynamic_cast<OpenMCCellAverageProblem *>(_problem.get());

  if (!openmc_problem)
    mooseError("The simulation must use an OpenMCCellAverageProblem when using the tally system!");

  _moose_object_pars.set<OpenMCCellAverageProblem *>("_openmc_problem") = openmc_problem;
  openmc_problem->addTallyObject(_type, _name, _moose_object_pars);
}

void
AddTallyAction::fillMeshTranslations()
{
  if (isParamValid("mesh_translations"))
  {
    _mesh_translations = getParam<std::vector<Point>>("mesh_translations");
    if (_mesh_translations.empty())
      mooseError("mesh_translations cannot be empty!");
  }
  else if (isParamValid("mesh_translations_file"))
  {
    std::vector<FileName> mesh_translations_file =
        getParam<std::vector<FileName>>("mesh_translations_file");
    if (mesh_translations_file.empty())
      mooseError("mesh_translations_file cannot be empty!");

    for (const auto & f : mesh_translations_file)
    {
      MooseUtils::DelimitedFileReader file(f, &_communicator);
      file.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::ROWS);
      file.read();

      const std::vector<std::vector<double>> & data = file.getData();
      readMeshTranslations(data);
    }
  }
  else
    _mesh_translations = {Point(0.0, 0.0, 0.0)};
}

void
AddTallyAction::readMeshTranslations(const std::vector<std::vector<double>> & data)
{
  for (const auto & d : data)
  {
    if (d.size() != OpenMCCellAverageProblem::DIMENSION)
      paramError("mesh_translations_file",
                 "All entries in 'mesh_translations_file' "
                 "must contain exactly ",
                 OpenMCCellAverageProblem::DIMENSION,
                 " coordinates.");

    // OpenMCCellAverageProblem::DIMENSION will always be 3
    _mesh_translations.push_back(Point(d[0], d[1], d[2]));
  }
}
#endif
