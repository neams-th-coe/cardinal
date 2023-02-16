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

#include "SymmetryPointGeneratorTest.h"

TEST_F(SymmetryPointGeneratorTest, reflect_y_axis)
{
  // point already on the negative side of the plane
  Point pt1(-1, -1, 0.0);
  Point pt1_r = _spg1->transformPoint(pt1);
  EXPECT_DOUBLE_EQ(pt1_r(0), pt1(0));
  EXPECT_DOUBLE_EQ(pt1_r(1), pt1(1));
  EXPECT_DOUBLE_EQ(pt1_r(2), pt1(2));

  // points on the positive side of the plane
  Point pt2(1.0, 1.0, -2.0);
  Point pt2_r = _spg1->transformPoint(pt2);
  EXPECT_DOUBLE_EQ(pt2_r(0), -1.0);
  EXPECT_DOUBLE_EQ(pt2_r(1), 1.0);
  EXPECT_DOUBLE_EQ(pt2_r(2), -2.0);
}

TEST_F(SymmetryPointGeneratorTest, reflect_xy_plane)
{
  // point already on the negative side of the plane
  Point pt1(1, -0.1, 0.0);
  Point pt1_r = _spg2->transformPoint(pt1);
  EXPECT_DOUBLE_EQ(pt1_r(0), pt1(0));
  EXPECT_DOUBLE_EQ(pt1_r(1), pt1(1));
  EXPECT_DOUBLE_EQ(pt1_r(2), pt1(2));

  // points on the positive side of the plane
  Point pt2(0.0, 1.0, -2.0);
  Point pt2_r = _spg2->transformPoint(pt2);
  EXPECT_DOUBLE_EQ(pt2_r(0), 1.0);
  EXPECT_LE(abs(pt2_r(1) - 0.0), 5e-16);
  EXPECT_DOUBLE_EQ(pt2_r(2), -2.0);

  Point pt3(-1.0, 2.0, 2.0);
  Point pt3_r = _spg2->transformPoint(pt3);
  EXPECT_DOUBLE_EQ(pt3_r(0), 2.0);
  EXPECT_DOUBLE_EQ(pt3_r(1), -1.0);
  EXPECT_DOUBLE_EQ(pt3_r(2), 2.0);
}

TEST_F(SymmetryPointGeneratorTest, reflect_general_plane)
{
  Point pt1(-1.0, -3.0, -4.0);
  Point pt1r = _spg3->transformPoint(pt1);
  EXPECT_LE(abs(pt1r(0) - -3.42857142857), 1e-8);
  EXPECT_LE(abs(pt1r(1) - 1.85714285714), 1e-8);
  EXPECT_LE(abs(pt1r(2) - 3.28571428571), 1e-8);
}

TEST_F(SymmetryPointGeneratorTest, rotate_about_z)
{
  Point pt(2.0, 3.0, 5.0);
  Point ptr = _spg4->transformPoint(pt);
  EXPECT_DOUBLE_EQ(ptr(0), -2.0);
  EXPECT_DOUBLE_EQ(ptr(1), -3.0);
  EXPECT_DOUBLE_EQ(ptr(2), 5.0);

  Point pt1(3.0, 2.0, 5.0);
  ptr = _spg4->transformPoint(pt1);
  EXPECT_DOUBLE_EQ(ptr(0), -3.0);
  EXPECT_DOUBLE_EQ(ptr(1), -2.0);
  EXPECT_DOUBLE_EQ(ptr(2), 5.0);

  Point pt2(-2.0, 3.0, 5.0);
  ptr = _spg4->transformPoint(pt2);
  EXPECT_DOUBLE_EQ(ptr(0), -2.0);
  EXPECT_DOUBLE_EQ(ptr(1), -3.0);
  EXPECT_DOUBLE_EQ(ptr(2), 5.0);

  Point pt3(-3.0, 2.0, 5.0);
  ptr = _spg4->transformPoint(pt3);
  EXPECT_DOUBLE_EQ(ptr(0), -3.0);
  EXPECT_DOUBLE_EQ(ptr(1), -2.0);
  EXPECT_DOUBLE_EQ(ptr(2), 5.0);

  Point pt4(-2.0, -3.0, 5.0);
  ptr = _spg4->transformPoint(pt4);
  EXPECT_DOUBLE_EQ(ptr(0), -2.0);
  EXPECT_DOUBLE_EQ(ptr(1), -3.0);
  EXPECT_DOUBLE_EQ(ptr(2), 5.0);

  Point pt5(-3.0, -2.0, 5.0);
  ptr = _spg4->transformPoint(pt5);
  EXPECT_DOUBLE_EQ(ptr(0), -3.0);
  EXPECT_DOUBLE_EQ(ptr(1), -2.0);
  EXPECT_DOUBLE_EQ(ptr(2), 5.0);

  Point pt6(2.0, -3.0, 5.0);
  ptr = _spg4->transformPoint(pt6);
  EXPECT_DOUBLE_EQ(ptr(0), -2.0);
  EXPECT_DOUBLE_EQ(ptr(1), -3.0);
  EXPECT_DOUBLE_EQ(ptr(2), 5.0);

  Point pt7(3.0, 2.0, 5.0);
  ptr = _spg4->transformPoint(pt7);
  EXPECT_DOUBLE_EQ(ptr(0), -3.0);
  EXPECT_DOUBLE_EQ(ptr(1), -2.0);
  EXPECT_DOUBLE_EQ(ptr(2), 5.0);
}

TEST_F(SymmetryPointGeneratorTest, sector_6)
{
  Point p1(5.0, -0.1, 0.0);
  EXPECT_EQ(_spg5->sector(p1), 0);

  Point p2(1.0, -11.0, 0.0);
  EXPECT_EQ(_spg5->sector(p2), 1);

  Point p3(-7.0, -1.0, 0.0);
  EXPECT_EQ(_spg5->sector(p3), 2);

  Point p4(-7.0, 1.0, 0.0);
  EXPECT_EQ(_spg5->sector(p4), 3);

  Point p5(0.1, 0.8, 0.0);
  EXPECT_EQ(_spg5->sector(p5), 4);

  Point p6(19.0, 1.0, 0.0);
  EXPECT_EQ(_spg5->sector(p6), 5);
}

TEST_F(SymmetryPointGeneratorTest, sector_5)
{
  Point p1(5.0, -0.1, 0.0);
  EXPECT_EQ(_spg6->sector(p1), 0);

  Point p2(1.0, -11.0, 0.0);
  EXPECT_EQ(_spg6->sector(p2), 1);

  Point p3(-7.0, -1.0, 0.0);
  EXPECT_EQ(_spg6->sector(p3), 2);

  Point p4(-7.0, 1.0, 0.0);
  EXPECT_EQ(_spg6->sector(p4), 2);

  Point p5(0.1, 2.8, 0.0);
  EXPECT_EQ(_spg6->sector(p5), 3);

  Point p6(19.0, 1.0, 0.0);
  EXPECT_EQ(_spg6->sector(p6), 4);
}
