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
    Point pt1(0.0, 0.0, 0.0);
    Point n1(0.0, 0.0, 0.0);
    SymmetryPointGenerator invalid_norm(pt1, n1, 90.0);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("Symmetry plane normal cannot have zero norm!"),
      std::string::npos) << "failed with unexpected error: " << msg;
  }

  try
  {
    Point pt1(0.0, 0.0, 0.0);
    Point n1(1.0, 0.0, 0.0);
    SymmetryPointGenerator invalid_length(pt1, n1, 23.0);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("The 'symmetry_angle' must be an integer division of the unit circle!"),
      std::string::npos) << "failed with unexpected error: " << msg;
  }
}

TEST_F(SymmetryPointGeneratorTest, reflect_axis)
{
  Point pt(0.0, 0.0, 0.0);
  Point n(1.0, 0.0, 0.0);
  SymmetryPointGenerator sg(pt, n, 90.0);

  // point already on the negative side of the plane
  Point pt1(-1, -1, 0.0);
  Point pt1_r = sg.reflectPointAcrossPlane(pt1);
  EXPECT_DOUBLE_EQ(pt1_r(0), pt1(0));
  EXPECT_DOUBLE_EQ(pt1_r(1), pt1(1));
  EXPECT_DOUBLE_EQ(pt1_r(2), pt1(2));

  // points on the positive side of the plane
  Point pt2(1.0, 1.0, -2.0);
  Point pt2_r = sg.reflectPointAcrossPlane(pt2);
  EXPECT_DOUBLE_EQ(pt2_r(0), -1.0);
  EXPECT_DOUBLE_EQ(pt2_r(1),  1.0);
  EXPECT_DOUBLE_EQ(pt2_r(2), -2.0);
}

TEST_F(SymmetryPointGeneratorTest, reflect)
{
  Point pt(0.0, 0.0, 0.0);
  Point n(-1.0, 1.0, 0.0);
  SymmetryPointGenerator sg(pt, n, 90.0);

  // point already on the negative side of the plane
  Point pt1(1, -0.1, 0.0);
  Point pt1_r = sg.reflectPointAcrossPlane(pt1);
  EXPECT_DOUBLE_EQ(pt1_r(0), pt1(0));
  EXPECT_DOUBLE_EQ(pt1_r(1), pt1(1));
  EXPECT_DOUBLE_EQ(pt1_r(2), pt1(2));

  // points on the positive side of the plane
  Point pt2(0.0, 1.0, -2.0);
  Point pt2_r = sg.reflectPointAcrossPlane(pt2);
  EXPECT_DOUBLE_EQ(pt2_r(0),  1.0);
  EXPECT_DOUBLE_EQ(pt2_r(1),  0.0);
  EXPECT_DOUBLE_EQ(pt2_r(2), -2.0);

  Point pt3(-1.0, 2.0, 2.0);
  Point pt3_r = sg.reflectPointAcrossPlane(pt3);
  EXPECT_DOUBLE_EQ(pt3_r(0),  2.0);
  EXPECT_DOUBLE_EQ(pt3_r(1), -1.0);
  EXPECT_DOUBLE_EQ(pt3_r(2),  2.0);
}
