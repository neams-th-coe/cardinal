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

#include "NekInterfaceTest.h"

TEST_F(NekInterfaceTest, corner_gll)
{
  int n = 1;
  auto first = nekrs::cornerGLLIndices(n, false);
  EXPECT_EQ(first.size(), 1);
  EXPECT_EQ(first[0].size(), 4);
  EXPECT_EQ(first[0][0], 0);
  EXPECT_EQ(first[0][1], 1);
  EXPECT_EQ(first[0][2], 2);
  EXPECT_EQ(first[0][3], 3);

  first = nekrs::cornerGLLIndices(n, true);
  EXPECT_EQ(first.size(), 1);
  EXPECT_EQ(first[0].size(), 4);
  EXPECT_EQ(first[0][0], 0);
  EXPECT_EQ(first[0][1], 1);
  EXPECT_EQ(first[0][2], 2);
  EXPECT_EQ(first[0][3], 3);

  n = 2;
  auto second = nekrs::cornerGLLIndices(n, false);
  EXPECT_EQ(second.size(), 1);
  EXPECT_EQ(second[0].size(), 4);
  EXPECT_EQ(second[0][0], 0);
  EXPECT_EQ(second[0][1], 2);
  EXPECT_EQ(second[0][2], 6);
  EXPECT_EQ(second[0][3], 8);

  second = nekrs::cornerGLLIndices(n, true);
  EXPECT_EQ(second.size(), 4);
  EXPECT_EQ(second[0].size(), 4);
  EXPECT_EQ(second[0][0], 0);
  EXPECT_EQ(second[0][1], 1);
  EXPECT_EQ(second[0][2], 3);
  EXPECT_EQ(second[0][3], 4);

  EXPECT_EQ(second[1].size(), 4);
  EXPECT_EQ(second[1][0], 1);
  EXPECT_EQ(second[1][1], 2);
  EXPECT_EQ(second[1][2], 4);
  EXPECT_EQ(second[1][3], 5);

  EXPECT_EQ(second[2].size(), 4);
  EXPECT_EQ(second[2][0], 3);
  EXPECT_EQ(second[2][1], 4);
  EXPECT_EQ(second[2][2], 6);
  EXPECT_EQ(second[2][3], 7);

  EXPECT_EQ(second[3].size(), 4);
  EXPECT_EQ(second[3][0], 4);
  EXPECT_EQ(second[3][1], 5);
  EXPECT_EQ(second[3][2], 7);
  EXPECT_EQ(second[3][3], 8);

  n = 3;
  auto third = nekrs::cornerGLLIndices(n, false);
  EXPECT_EQ(third.size(), 1);
  EXPECT_EQ(third[0].size(), 4);
  EXPECT_EQ(third[0][0], 0);
  EXPECT_EQ(third[0][1], 3);
  EXPECT_EQ(third[0][2], 12);
  EXPECT_EQ(third[0][3], 15);

  third = nekrs::cornerGLLIndices(n, true);
  EXPECT_EQ(third.size(), 9);
  EXPECT_EQ(third[0].size(), 4);
  EXPECT_EQ(third[0][0], 0);
  EXPECT_EQ(third[0][1], 1);
  EXPECT_EQ(third[0][2], 4);
  EXPECT_EQ(third[0][3], 5);

  EXPECT_EQ(third[1].size(), 4);
  EXPECT_EQ(third[1][0], 1);
  EXPECT_EQ(third[1][1], 2);
  EXPECT_EQ(third[1][2], 5);
  EXPECT_EQ(third[1][3], 6);

  EXPECT_EQ(third[2].size(), 4);
  EXPECT_EQ(third[2][0], 2);
  EXPECT_EQ(third[2][1], 3);
  EXPECT_EQ(third[2][2], 6);
  EXPECT_EQ(third[2][3], 7);

  EXPECT_EQ(third[3].size(), 4);
  EXPECT_EQ(third[3][0], 4);
  EXPECT_EQ(third[3][1], 5);
  EXPECT_EQ(third[3][2], 8);
  EXPECT_EQ(third[3][3], 9);

  EXPECT_EQ(third[4].size(), 4);
  EXPECT_EQ(third[4][0], 5);
  EXPECT_EQ(third[4][1], 6);
  EXPECT_EQ(third[4][2], 9);
  EXPECT_EQ(third[4][3], 10);

  EXPECT_EQ(third[5].size(), 4);
  EXPECT_EQ(third[5][0], 6);
  EXPECT_EQ(third[5][1], 7);
  EXPECT_EQ(third[5][2], 10);
  EXPECT_EQ(third[5][3], 11);

  EXPECT_EQ(third[6].size(), 4);
  EXPECT_EQ(third[6][0], 8);
  EXPECT_EQ(third[6][1], 9);
  EXPECT_EQ(third[6][2], 12);
  EXPECT_EQ(third[6][3], 13);

  EXPECT_EQ(third[7].size(), 4);
  EXPECT_EQ(third[7][0], 9);
  EXPECT_EQ(third[7][1], 10);
  EXPECT_EQ(third[7][2], 13);
  EXPECT_EQ(third[7][3], 14);

  EXPECT_EQ(third[8].size(), 4);
  EXPECT_EQ(third[8][0], 10);
  EXPECT_EQ(third[8][1], 11);
  EXPECT_EQ(third[8][2], 14);
  EXPECT_EQ(third[8][3], 15);
}
