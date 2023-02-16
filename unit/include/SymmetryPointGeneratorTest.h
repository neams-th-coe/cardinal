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

#include "SymmetryPointGenerator.h"
#include "MooseObjectUnitTest.h"

class SymmetryPointGeneratorTest : public MooseObjectUnitTest
{
public:
  SymmetryPointGeneratorTest() : MooseObjectUnitTest("CardinalUnitApp") { buildObjects(); }

protected:
  void buildObjects()
  {
    InputParameters p1 = _factory.getValidParams("SymmetryPointGenerator");
    p1.set<Point>("normal") = {1.0, 0.0, 0.0};
    _fe_problem->addUserObject("SymmetryPointGenerator", "spg1", p1);
    _spg1 = &_fe_problem->getUserObject<SymmetryPointGenerator>("spg1");

    InputParameters p2 = _factory.getValidParams("SymmetryPointGenerator");
    p2.set<Point>("normal") = {-1.0, 1.0, 0.0};
    _fe_problem->addUserObject("SymmetryPointGenerator", "spg2", p2);
    _spg2 = &_fe_problem->getUserObject<SymmetryPointGenerator>("spg2");

    InputParameters p3 = _factory.getValidParams("SymmetryPointGenerator");
    p3.set<Point>("normal") = {1.0, -2.0, -3.0};
    _fe_problem->addUserObject("SymmetryPointGenerator", "spg3", p3);
    _spg3 = &_fe_problem->getUserObject<SymmetryPointGenerator>("spg3");

    InputParameters p4 = _factory.getValidParams("SymmetryPointGenerator");
    p4.set<Point>("normal") = {1.0, 0.0, 0.0};
    p4.set<Point>("rotation_axis") = {0.0, 0.0, 1.0};
    p4.set<Real>("rotation_angle") = 90.0;
    _fe_problem->addUserObject("SymmetryPointGenerator", "spg4", p4);
    _spg4 = &_fe_problem->getUserObject<SymmetryPointGenerator>("spg4");

    InputParameters p5 = _factory.getValidParams("SymmetryPointGenerator");
    p5.set<Point>("normal") = {0.0, 1.0, 0.0};
    p5.set<Point>("rotation_axis") = {0.0, 0.0, 1.0};
    p5.set<Real>("rotation_angle") = 60.0;
    _fe_problem->addUserObject("SymmetryPointGenerator", "spg5", p5);
    _spg5 = &_fe_problem->getUserObject<SymmetryPointGenerator>("spg5");

    InputParameters p6 = _factory.getValidParams("SymmetryPointGenerator");
    p6.set<Point>("normal") = {0.0, 1.0, 0.0};
    p6.set<Point>("rotation_axis") = {0.0, 0.0, 1.0};
    p6.set<Real>("rotation_angle") = 360.0 / 5.0;
    _fe_problem->addUserObject("SymmetryPointGenerator", "spg6", p6);
    _spg6 = &_fe_problem->getUserObject<SymmetryPointGenerator>("spg6");
  }

  const SymmetryPointGenerator * _spg1;
  const SymmetryPointGenerator * _spg2;
  const SymmetryPointGenerator * _spg3;
  const SymmetryPointGenerator * _spg4;
  const SymmetryPointGenerator * _spg5;
  const SymmetryPointGenerator * _spg6;
};
