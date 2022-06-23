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

TEST_F(SymmetryPointGeneratorTest, errors)
{
  try
  {
    Point n1(0.0, 0.0, 0.0);
    SymmetryPointGenerator sg(n1);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("The 'symmetry_plane_normal' cannot have zero norm!"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  try
  {
    Point n1(1.0, 0.0, 0.0);
    SymmetryPointGenerator sg(n1);

    Point n2(1.0, 0.0, 0.0);
    sg.initializeAngularSymmetry(n2, 90.0);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("The 'symmetry_axis' must be perpendicular to the 'symmetry_plane_normal'!"),
              std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  try
  {
    Point n1(1.0, 0.0, 0.0);
    SymmetryPointGenerator sg(n1);

    Point n2(0.0, 0.0, 0.0);
    sg.initializeAngularSymmetry(n2, 90.0);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("The 'symmetry_axis' cannot have zero norm!"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  try
  {
    Point n1(1.0, 0.0, 0.0);
    SymmetryPointGenerator sg(n1);

    Point n2(0.0, 0.0, 1.0);
    sg.initializeAngularSymmetry(n2, 92.0);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("The unit circle must be divisible by the 'symmetry_angle'!"),
              std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

TEST_F(SymmetryPointGeneratorTest, reflect_y_axis)
{
  Point n(1.0, 0.0, 0.0);
  SymmetryPointGenerator sg(n);

  // point already on the negative side of the plane
  Point pt1(-1, -1, 0.0);
  Point pt1_r = sg.transformPoint(pt1);
  EXPECT_DOUBLE_EQ(pt1_r(0), pt1(0));
  EXPECT_DOUBLE_EQ(pt1_r(1), pt1(1));
  EXPECT_DOUBLE_EQ(pt1_r(2), pt1(2));

  // points on the positive side of the plane
  Point pt2(1.0, 1.0, -2.0);
  Point pt2_r = sg.transformPoint(pt2);
  EXPECT_DOUBLE_EQ(pt2_r(0), -1.0);
  EXPECT_DOUBLE_EQ(pt2_r(1), 1.0);
  EXPECT_DOUBLE_EQ(pt2_r(2), -2.0);
}

TEST_F(SymmetryPointGeneratorTest, reflect_xy_plane)
{
  Point n(-1.0, 1.0, 0.0);
  n = n / n.norm();
  SymmetryPointGenerator sg(n);

  // point already on the negative side of the plane
  Point pt1(1, -0.1, 0.0);
  Point pt1_r = sg.transformPoint(pt1);
  EXPECT_DOUBLE_EQ(pt1_r(0), pt1(0));
  EXPECT_DOUBLE_EQ(pt1_r(1), pt1(1));
  EXPECT_DOUBLE_EQ(pt1_r(2), pt1(2));

  // points on the positive side of the plane
  Point pt2(0.0, 1.0, -2.0);
  Point pt2_r = sg.transformPoint(pt2);
  EXPECT_DOUBLE_EQ(pt2_r(0), 1.0);
  EXPECT_LE(abs(pt2_r(1) - 0.0), 5e-16);
  EXPECT_DOUBLE_EQ(pt2_r(2), -2.0);

  Point pt3(-1.0, 2.0, 2.0);
  Point pt3_r = sg.transformPoint(pt3);
  EXPECT_DOUBLE_EQ(pt3_r(0), 2.0);
  EXPECT_DOUBLE_EQ(pt3_r(1), -1.0);
  EXPECT_DOUBLE_EQ(pt3_r(2), 2.0);
}

TEST_F(SymmetryPointGeneratorTest, reflect_general_plane)
{
  Point n(1.0, -2.0, -3.0);
  n = n / n.norm();

  SymmetryPointGenerator sg(n);
  Point pt1(-1.0, -3.0, -4.0);

  Point pt1r = sg.transformPoint(pt1);
  EXPECT_LE(abs(pt1r(0) - -3.42857142857), 1e-8);
  EXPECT_LE(abs(pt1r(1) - 1.85714285714), 1e-8);
  EXPECT_LE(abs(pt1r(2) - 3.28571428571), 1e-8);
}

TEST_F(SymmetryPointGeneratorTest, rotate_about_z)
{
  Point n(1.0, 0.0, 0.0);
  Point a(0.0, 0.0, 1.0);
  SymmetryPointGenerator sg(n);
  sg.initializeAngularSymmetry(a, 90.0);

  Point pt(2.0, 3.0, 5.0);
  Point ptr = sg.transformPoint(pt);
  EXPECT_DOUBLE_EQ(ptr(0), -2.0);
  EXPECT_DOUBLE_EQ(ptr(1), -3.0);
  EXPECT_DOUBLE_EQ(ptr(2), 5.0);

  Point pt1(3.0, 2.0, 5.0);
  ptr = sg.transformPoint(pt1);
  EXPECT_DOUBLE_EQ(ptr(0), -3.0);
  EXPECT_DOUBLE_EQ(ptr(1), -2.0);
  EXPECT_DOUBLE_EQ(ptr(2), 5.0);

  Point pt2(-2.0, 3.0, 5.0);
  ptr = sg.transformPoint(pt2);
  EXPECT_DOUBLE_EQ(ptr(0), -2.0);
  EXPECT_DOUBLE_EQ(ptr(1), -3.0);
  EXPECT_DOUBLE_EQ(ptr(2), 5.0);

  Point pt3(-3.0, 2.0, 5.0);
  ptr = sg.transformPoint(pt3);
  EXPECT_DOUBLE_EQ(ptr(0), -3.0);
  EXPECT_DOUBLE_EQ(ptr(1), -2.0);
  EXPECT_DOUBLE_EQ(ptr(2), 5.0);

  Point pt4(-2.0, -3.0, 5.0);
  ptr = sg.transformPoint(pt4);
  EXPECT_DOUBLE_EQ(ptr(0), -2.0);
  EXPECT_DOUBLE_EQ(ptr(1), -3.0);
  EXPECT_DOUBLE_EQ(ptr(2), 5.0);

  Point pt5(-3.0, -2.0, 5.0);
  ptr = sg.transformPoint(pt5);
  EXPECT_DOUBLE_EQ(ptr(0), -3.0);
  EXPECT_DOUBLE_EQ(ptr(1), -2.0);
  EXPECT_DOUBLE_EQ(ptr(2), 5.0);

  Point pt6(2.0, -3.0, 5.0);
  ptr = sg.transformPoint(pt6);
  EXPECT_DOUBLE_EQ(ptr(0), -2.0);
  EXPECT_DOUBLE_EQ(ptr(1), -3.0);
  EXPECT_DOUBLE_EQ(ptr(2), 5.0);

  Point pt7(3.0, 2.0, 5.0);
  ptr = sg.transformPoint(pt7);
  EXPECT_DOUBLE_EQ(ptr(0), -3.0);
  EXPECT_DOUBLE_EQ(ptr(1), -2.0);
  EXPECT_DOUBLE_EQ(ptr(2), 5.0);
}

TEST_F(SymmetryPointGeneratorTest, sector_6)
{
  Point n(0.0, 1.0, 0.0);
  Point a(0.0, 0.0, 1.0);
  SymmetryPointGenerator sg(n);
  sg.initializeAngularSymmetry(a, 360.0 / 6.0);

  Point p1(5.0, -0.1, 0.0);
  EXPECT_EQ(sg.sector(p1), 0);

  Point p2(1.0, -11.0, 0.0);
  EXPECT_EQ(sg.sector(p2), 1);

  Point p3(-7.0, -1.0, 0.0);
  EXPECT_EQ(sg.sector(p3), 2);

  Point p4(-7.0, 1.0, 0.0);
  EXPECT_EQ(sg.sector(p4), 3);

  Point p5(0.1, 0.8, 0.0);
  EXPECT_EQ(sg.sector(p5), 4);

  Point p6(19.0, 1.0, 0.0);
  EXPECT_EQ(sg.sector(p6), 5);
}

TEST_F(SymmetryPointGeneratorTest, sector_5)
{
  Point n(0.0, 1.0, 0.0);
  Point a(0.0, 0.0, 1.0);
  SymmetryPointGenerator sg(n);
  sg.initializeAngularSymmetry(a, 360.0 / 5.0);

  Point p1(5.0, -0.1, 0.0);
  EXPECT_EQ(sg.sector(p1), 0);

  Point p2(1.0, -11.0, 0.0);
  EXPECT_EQ(sg.sector(p2), 1);

  Point p3(-7.0, -1.0, 0.0);
  EXPECT_EQ(sg.sector(p3), 2);

  Point p4(-7.0, 1.0, 0.0);
  EXPECT_EQ(sg.sector(p4), 2);

  Point p5(0.1, 2.8, 0.0);
  EXPECT_EQ(sg.sector(p5), 3);

  Point p6(19.0, 1.0, 0.0);
  EXPECT_EQ(sg.sector(p6), 4);
}
