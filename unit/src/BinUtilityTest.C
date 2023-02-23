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

#include "BinUtilityTest.h"

TEST_F(BinUtilityTest, linear_bins)
{
  // five bins, between 100 and 300
  const std::vector<Real> bounds = {100.0, 140.0, 180.0, 220.0, 260.0, 300.0};

  // below lower bound
  Real v = 99.0;
  auto bin = bin_utility::linearBin(v, bounds);
  EXPECT_EQ(bin, 0);

  // above upper bound
  v = 301.0;
  bin = bin_utility::linearBin(v, bounds);
  EXPECT_EQ(bin, 4);

  // on lower bound
  v = 100.0;
  bin = bin_utility::linearBin(v, bounds);
  EXPECT_EQ(bin, 0);

  // on upper bound
  v = 300.0;
  bin = bin_utility::linearBin(v, bounds);
  EXPECT_EQ(bin, 4);

  // on inner bounds
  v = 140.0;
  bin = bin_utility::linearBin(v, bounds);
  EXPECT_EQ(bin, 1);

  v = 180.0;
  bin = bin_utility::linearBin(v, bounds);
  EXPECT_EQ(bin, 2);

  v = 200.0;
  bin = bin_utility::linearBin(v, bounds);
  EXPECT_EQ(bin, 2);

  v = 260.0;
  bin = bin_utility::linearBin(v, bounds);
  EXPECT_EQ(bin, 4);

  // within inner bins
  v = 100.0;
  bin = bin_utility::linearBin(v, bounds);
  EXPECT_EQ(bin, 0);

  v = 150.0;
  bin = bin_utility::linearBin(v, bounds);
  EXPECT_EQ(bin, 1);

  v = 210.0;
  bin = bin_utility::linearBin(v, bounds);
  EXPECT_EQ(bin, 2);

  v = 245.0;
  bin = bin_utility::linearBin(v, bounds);
  EXPECT_EQ(bin, 3);

  v = 265.0;
  bin = bin_utility::linearBin(v, bounds);
  EXPECT_EQ(bin, 4);

  auto mid = bin_utility::midpoint(0, bounds);
  EXPECT_DOUBLE_EQ(mid, 120.0);

  mid = bin_utility::midpoint(1, bounds);
  EXPECT_DOUBLE_EQ(mid, 160.0);

  mid = bin_utility::midpoint(2, bounds);
  EXPECT_DOUBLE_EQ(mid, 200.0);

  mid = bin_utility::midpoint(3, bounds);
  EXPECT_DOUBLE_EQ(mid, 240.0);

  mid = bin_utility::midpoint(4, bounds);
  EXPECT_DOUBLE_EQ(mid, 280.0);
}
