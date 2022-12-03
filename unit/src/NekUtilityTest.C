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

#include "NekUtilityTest.h"

TEST_F(NekUtilityTest, corner_gll)
{
  int n_nodes_per_el = 8;

  int n = 1;
  auto first = nekrs::cornerGLLIndices(n, false);
  EXPECT_EQ(first.size(), 1);
  for (const auto & s : first)
    EXPECT_EQ(s.size(), n_nodes_per_el);

  EXPECT_EQ(first[0][0], 0);
  EXPECT_EQ(first[0][1], 1);
  EXPECT_EQ(first[0][2], 2);
  EXPECT_EQ(first[0][3], 3);
  EXPECT_EQ(first[0][4], 4);
  EXPECT_EQ(first[0][5], 5);
  EXPECT_EQ(first[0][6], 6);
  EXPECT_EQ(first[0][7], 7);

  first = nekrs::cornerGLLIndices(n, true);
  EXPECT_EQ(first.size(), 1);
  for (const auto & s : first)
    EXPECT_EQ(s.size(), n_nodes_per_el);

  EXPECT_EQ(first[0][0], 0);
  EXPECT_EQ(first[0][1], 1);
  EXPECT_EQ(first[0][2], 2);
  EXPECT_EQ(first[0][3], 3);
  EXPECT_EQ(first[0][4], 4);
  EXPECT_EQ(first[0][5], 5);
  EXPECT_EQ(first[0][6], 6);
  EXPECT_EQ(first[0][7], 7);

  n = 2;
  auto second = nekrs::cornerGLLIndices(n, false);
  EXPECT_EQ(second.size(), 1);
  for (const auto & s : second)
    EXPECT_EQ(s.size(), n_nodes_per_el);

  EXPECT_EQ(second[0][0], 0);
  EXPECT_EQ(second[0][1], 2);
  EXPECT_EQ(second[0][2], 6);
  EXPECT_EQ(second[0][3], 8);
  EXPECT_EQ(second[0][4], 18);
  EXPECT_EQ(second[0][5], 20);
  EXPECT_EQ(second[0][6], 24);
  EXPECT_EQ(second[0][7], 26);

  second = nekrs::cornerGLLIndices(n, true);
  EXPECT_EQ(second.size(), 8);
  for (const auto & s : second)
    EXPECT_EQ(s.size(), n_nodes_per_el);

  EXPECT_EQ(second[0][0], 0);
  EXPECT_EQ(second[0][1], 1);
  EXPECT_EQ(second[0][2], 3);
  EXPECT_EQ(second[0][3], 4);
  EXPECT_EQ(second[0][4], 9);
  EXPECT_EQ(second[0][5], 10);
  EXPECT_EQ(second[0][6], 12);
  EXPECT_EQ(second[0][7], 13);

  EXPECT_EQ(second[1][0], 1);
  EXPECT_EQ(second[1][1], 2);
  EXPECT_EQ(second[1][2], 4);
  EXPECT_EQ(second[1][3], 5);
  EXPECT_EQ(second[1][4], 10);
  EXPECT_EQ(second[1][5], 11);
  EXPECT_EQ(second[1][6], 13);
  EXPECT_EQ(second[1][7], 14);

  EXPECT_EQ(second[2][0], 3);
  EXPECT_EQ(second[2][1], 4);
  EXPECT_EQ(second[2][2], 6);
  EXPECT_EQ(second[2][3], 7);
  EXPECT_EQ(second[2][4], 12);
  EXPECT_EQ(second[2][5], 13);
  EXPECT_EQ(second[2][6], 15);
  EXPECT_EQ(second[2][7], 16);

  EXPECT_EQ(second[3][0], 4);
  EXPECT_EQ(second[3][1], 5);
  EXPECT_EQ(second[3][2], 7);
  EXPECT_EQ(second[3][3], 8);
  EXPECT_EQ(second[3][4], 13);
  EXPECT_EQ(second[3][5], 14);
  EXPECT_EQ(second[3][6], 16);
  EXPECT_EQ(second[3][7], 17);

  EXPECT_EQ(second[4][0], 9);
  EXPECT_EQ(second[4][1], 10);
  EXPECT_EQ(second[4][2], 12);
  EXPECT_EQ(second[4][3], 13);
  EXPECT_EQ(second[4][4], 18);
  EXPECT_EQ(second[4][5], 19);
  EXPECT_EQ(second[4][6], 21);
  EXPECT_EQ(second[4][7], 22);

  EXPECT_EQ(second[5][0], 10);
  EXPECT_EQ(second[5][1], 11);
  EXPECT_EQ(second[5][2], 13);
  EXPECT_EQ(second[5][3], 14);
  EXPECT_EQ(second[5][4], 19);
  EXPECT_EQ(second[5][5], 20);
  EXPECT_EQ(second[5][6], 22);
  EXPECT_EQ(second[5][7], 23);

  EXPECT_EQ(second[6][0], 12);
  EXPECT_EQ(second[6][1], 13);
  EXPECT_EQ(second[6][2], 15);
  EXPECT_EQ(second[6][3], 16);
  EXPECT_EQ(second[6][4], 21);
  EXPECT_EQ(second[6][5], 22);
  EXPECT_EQ(second[6][6], 24);
  EXPECT_EQ(second[6][7], 25);

  EXPECT_EQ(second[7][0], 13);
  EXPECT_EQ(second[7][1], 14);
  EXPECT_EQ(second[7][2], 16);
  EXPECT_EQ(second[7][3], 17);
  EXPECT_EQ(second[7][4], 22);
  EXPECT_EQ(second[7][5], 23);
  EXPECT_EQ(second[7][6], 25);
  EXPECT_EQ(second[7][7], 26);

  n = 3;
  auto third = nekrs::cornerGLLIndices(n, false);
  EXPECT_EQ(third.size(), 1);
  for (const auto & s : third)
    EXPECT_EQ(s.size(), n_nodes_per_el);

  EXPECT_EQ(third[0][0], 0);
  EXPECT_EQ(third[0][1], 3);
  EXPECT_EQ(third[0][2], 12);
  EXPECT_EQ(third[0][3], 15);
  EXPECT_EQ(third[0][4], 48);
  EXPECT_EQ(third[0][5], 51);
  EXPECT_EQ(third[0][6], 60);
  EXPECT_EQ(third[0][7], 63);

  third = nekrs::cornerGLLIndices(n, true);
  EXPECT_EQ(third.size(), 27);
  for (const auto & s : third)
    EXPECT_EQ(s.size(), n_nodes_per_el);

  EXPECT_EQ(third[0][0], 0);
  EXPECT_EQ(third[0][1], 1);
  EXPECT_EQ(third[0][2], 4);
  EXPECT_EQ(third[0][3], 5);
  EXPECT_EQ(third[0][4], 16);
  EXPECT_EQ(third[0][5], 17);
  EXPECT_EQ(third[0][6], 20);
  EXPECT_EQ(third[0][7], 21);

  EXPECT_EQ(third[1][0], 1);
  EXPECT_EQ(third[1][1], 2);
  EXPECT_EQ(third[1][2], 5);
  EXPECT_EQ(third[1][3], 6);
  EXPECT_EQ(third[1][4], 17);
  EXPECT_EQ(third[1][5], 18);
  EXPECT_EQ(third[1][6], 21);
  EXPECT_EQ(third[1][7], 22);

  EXPECT_EQ(third[2][0], 2);
  EXPECT_EQ(third[2][1], 3);
  EXPECT_EQ(third[2][2], 6);
  EXPECT_EQ(third[2][3], 7);
  EXPECT_EQ(third[2][4], 18);
  EXPECT_EQ(third[2][5], 19);
  EXPECT_EQ(third[2][6], 22);
  EXPECT_EQ(third[2][7], 23);

  EXPECT_EQ(third[3][0], 4);
  EXPECT_EQ(third[3][1], 5);
  EXPECT_EQ(third[3][2], 8);
  EXPECT_EQ(third[3][3], 9);
  EXPECT_EQ(third[3][4], 20);
  EXPECT_EQ(third[3][5], 21);
  EXPECT_EQ(third[3][6], 24);
  EXPECT_EQ(third[3][7], 25);

  EXPECT_EQ(third[4][0], 5);
  EXPECT_EQ(third[4][1], 6);
  EXPECT_EQ(third[4][2], 9);
  EXPECT_EQ(third[4][3], 10);
  EXPECT_EQ(third[4][4], 21);
  EXPECT_EQ(third[4][5], 22);
  EXPECT_EQ(third[4][6], 25);
  EXPECT_EQ(third[4][7], 26);

  EXPECT_EQ(third[5][0], 6);
  EXPECT_EQ(third[5][1], 7);
  EXPECT_EQ(third[5][2], 10);
  EXPECT_EQ(third[5][3], 11);
  EXPECT_EQ(third[5][4], 22);
  EXPECT_EQ(third[5][5], 23);
  EXPECT_EQ(third[5][6], 26);
  EXPECT_EQ(third[5][7], 27);

  EXPECT_EQ(third[6][0], 8);
  EXPECT_EQ(third[6][1], 9);
  EXPECT_EQ(third[6][2], 12);
  EXPECT_EQ(third[6][3], 13);
  EXPECT_EQ(third[6][4], 24);
  EXPECT_EQ(third[6][5], 25);
  EXPECT_EQ(third[6][6], 28);
  EXPECT_EQ(third[6][7], 29);

  EXPECT_EQ(third[7][0], 9);
  EXPECT_EQ(third[7][1], 10);
  EXPECT_EQ(third[7][2], 13);
  EXPECT_EQ(third[7][3], 14);
  EXPECT_EQ(third[7][4], 25);
  EXPECT_EQ(third[7][5], 26);
  EXPECT_EQ(third[7][6], 29);
  EXPECT_EQ(third[7][7], 30);

  EXPECT_EQ(third[8][0], 10);
  EXPECT_EQ(third[8][1], 11);
  EXPECT_EQ(third[8][2], 14);
  EXPECT_EQ(third[8][3], 15);
  EXPECT_EQ(third[8][4], 26);
  EXPECT_EQ(third[8][5], 27);
  EXPECT_EQ(third[8][6], 30);
  EXPECT_EQ(third[8][7], 31);

  EXPECT_EQ(third[9][0], 16);
  EXPECT_EQ(third[9][1], 17);
  EXPECT_EQ(third[9][2], 20);
  EXPECT_EQ(third[9][3], 21);
  EXPECT_EQ(third[9][4], 32);
  EXPECT_EQ(third[9][5], 33);
  EXPECT_EQ(third[9][6], 36);
  EXPECT_EQ(third[9][7], 37);

  EXPECT_EQ(third[10][0], 17);
  EXPECT_EQ(third[10][1], 18);
  EXPECT_EQ(third[10][2], 21);
  EXPECT_EQ(third[10][3], 22);
  EXPECT_EQ(third[10][4], 33);
  EXPECT_EQ(third[10][5], 34);
  EXPECT_EQ(third[10][6], 37);
  EXPECT_EQ(third[10][7], 38);

  EXPECT_EQ(third[11][0], 18);
  EXPECT_EQ(third[11][1], 19);
  EXPECT_EQ(third[11][2], 22);
  EXPECT_EQ(third[11][3], 23);
  EXPECT_EQ(third[11][4], 34);
  EXPECT_EQ(third[11][5], 35);
  EXPECT_EQ(third[11][6], 38);
  EXPECT_EQ(third[11][7], 39);

  EXPECT_EQ(third[12][0], 20);
  EXPECT_EQ(third[12][1], 21);
  EXPECT_EQ(third[12][2], 24);
  EXPECT_EQ(third[12][3], 25);
  EXPECT_EQ(third[12][4], 36);
  EXPECT_EQ(third[12][5], 37);
  EXPECT_EQ(third[12][6], 40);
  EXPECT_EQ(third[12][7], 41);

  EXPECT_EQ(third[13][0], 21);
  EXPECT_EQ(third[13][1], 22);
  EXPECT_EQ(third[13][2], 25);
  EXPECT_EQ(third[13][3], 26);
  EXPECT_EQ(third[13][4], 37);
  EXPECT_EQ(third[13][5], 38);
  EXPECT_EQ(third[13][6], 41);
  EXPECT_EQ(third[13][7], 42);

  EXPECT_EQ(third[14][0], 22);
  EXPECT_EQ(third[14][1], 23);
  EXPECT_EQ(third[14][2], 26);
  EXPECT_EQ(third[14][3], 27);
  EXPECT_EQ(third[14][4], 38);
  EXPECT_EQ(third[14][5], 39);
  EXPECT_EQ(third[14][6], 42);
  EXPECT_EQ(third[14][7], 43);

  EXPECT_EQ(third[15][0], 24);
  EXPECT_EQ(third[15][1], 25);
  EXPECT_EQ(third[15][2], 28);
  EXPECT_EQ(third[15][3], 29);
  EXPECT_EQ(third[15][4], 40);
  EXPECT_EQ(third[15][5], 41);
  EXPECT_EQ(third[15][6], 44);
  EXPECT_EQ(third[15][7], 45);

  EXPECT_EQ(third[16][0], 25);
  EXPECT_EQ(third[16][1], 26);
  EXPECT_EQ(third[16][2], 29);
  EXPECT_EQ(third[16][3], 30);
  EXPECT_EQ(third[16][4], 41);
  EXPECT_EQ(third[16][5], 42);
  EXPECT_EQ(third[16][6], 45);
  EXPECT_EQ(third[16][7], 46);

  EXPECT_EQ(third[17][0], 26);
  EXPECT_EQ(third[17][1], 27);
  EXPECT_EQ(third[17][2], 30);
  EXPECT_EQ(third[17][3], 31);
  EXPECT_EQ(third[17][4], 42);
  EXPECT_EQ(third[17][5], 43);
  EXPECT_EQ(third[17][6], 46);
  EXPECT_EQ(third[17][7], 47);

  EXPECT_EQ(third[18][0], 32);
  EXPECT_EQ(third[18][1], 33);
  EXPECT_EQ(third[18][2], 36);
  EXPECT_EQ(third[18][3], 37);
  EXPECT_EQ(third[18][4], 48);
  EXPECT_EQ(third[18][5], 49);
  EXPECT_EQ(third[18][6], 52);
  EXPECT_EQ(third[18][7], 53);

  EXPECT_EQ(third[19][0], 33);
  EXPECT_EQ(third[19][1], 34);
  EXPECT_EQ(third[19][2], 37);
  EXPECT_EQ(third[19][3], 38);
  EXPECT_EQ(third[19][4], 49);
  EXPECT_EQ(third[19][5], 50);
  EXPECT_EQ(third[19][6], 53);
  EXPECT_EQ(third[19][7], 54);

  EXPECT_EQ(third[20][0], 34);
  EXPECT_EQ(third[20][1], 35);
  EXPECT_EQ(third[20][2], 38);
  EXPECT_EQ(third[20][3], 39);
  EXPECT_EQ(third[20][4], 50);
  EXPECT_EQ(third[20][5], 51);
  EXPECT_EQ(third[20][6], 54);
  EXPECT_EQ(third[20][7], 55);

  EXPECT_EQ(third[21][0], 36);
  EXPECT_EQ(third[21][1], 37);
  EXPECT_EQ(third[21][2], 40);
  EXPECT_EQ(third[21][3], 41);
  EXPECT_EQ(third[21][4], 52);
  EXPECT_EQ(third[21][5], 53);
  EXPECT_EQ(third[21][6], 56);
  EXPECT_EQ(third[21][7], 57);

  EXPECT_EQ(third[22][0], 37);
  EXPECT_EQ(third[22][1], 38);
  EXPECT_EQ(third[22][2], 41);
  EXPECT_EQ(third[22][3], 42);
  EXPECT_EQ(third[22][4], 53);
  EXPECT_EQ(third[22][5], 54);
  EXPECT_EQ(third[22][6], 57);
  EXPECT_EQ(third[22][7], 58);

  EXPECT_EQ(third[23][0], 38);
  EXPECT_EQ(third[23][1], 39);
  EXPECT_EQ(third[23][2], 42);
  EXPECT_EQ(third[23][3], 43);
  EXPECT_EQ(third[23][4], 54);
  EXPECT_EQ(third[23][5], 55);
  EXPECT_EQ(third[23][6], 58);
  EXPECT_EQ(third[23][7], 59);

  EXPECT_EQ(third[24][0], 40);
  EXPECT_EQ(third[24][1], 41);
  EXPECT_EQ(third[24][2], 44);
  EXPECT_EQ(third[24][3], 45);
  EXPECT_EQ(third[24][4], 56);
  EXPECT_EQ(third[24][5], 57);
  EXPECT_EQ(third[24][6], 60);
  EXPECT_EQ(third[24][7], 61);

  EXPECT_EQ(third[25][0], 41);
  EXPECT_EQ(third[25][1], 42);
  EXPECT_EQ(third[25][2], 45);
  EXPECT_EQ(third[25][3], 46);
  EXPECT_EQ(third[25][4], 57);
  EXPECT_EQ(third[25][5], 58);
  EXPECT_EQ(third[25][6], 61);
  EXPECT_EQ(third[25][7], 62);

  EXPECT_EQ(third[26][0], 42);
  EXPECT_EQ(third[26][1], 43);
  EXPECT_EQ(third[26][2], 46);
  EXPECT_EQ(third[26][3], 47);
  EXPECT_EQ(third[26][4], 58);
  EXPECT_EQ(third[26][5], 59);
  EXPECT_EQ(third[26][6], 62);
  EXPECT_EQ(third[26][7], 63);
}

TEST_F(NekUtilityTest, nested_elems)
{
  int n = 1;
  auto first = nekrs::nestedElementsOnFace(n);
  for (const auto & s : first)
    EXPECT_EQ(s.size(), 1);

  EXPECT_EQ(first[0][0], 0);
  EXPECT_EQ(first[1][0], 0);
  EXPECT_EQ(first[2][0], 0);
  EXPECT_EQ(first[3][0], 0);
  EXPECT_EQ(first[4][0], 0);
  EXPECT_EQ(first[5][0], 0);

  n = 2;
  auto second = nekrs::nestedElementsOnFace(n);
  for (const auto & s : second)
    EXPECT_EQ(s.size(), 4);

  EXPECT_EQ(second[0][0], 0);
  EXPECT_EQ(second[0][1], 1);
  EXPECT_EQ(second[0][2], 2);
  EXPECT_EQ(second[0][3], 3);

  EXPECT_EQ(second[1][0], 0);
  EXPECT_EQ(second[1][1], 1);
  EXPECT_EQ(second[1][2], 4);
  EXPECT_EQ(second[1][3], 5);

  EXPECT_EQ(second[2][0], 1);
  EXPECT_EQ(second[2][1], 3);
  EXPECT_EQ(second[2][2], 5);
  EXPECT_EQ(second[2][3], 7);

  EXPECT_EQ(second[3][0], 2);
  EXPECT_EQ(second[3][1], 3);
  EXPECT_EQ(second[3][2], 6);
  EXPECT_EQ(second[3][3], 7);

  EXPECT_EQ(second[4][0], 0);
  EXPECT_EQ(second[4][1], 2);
  EXPECT_EQ(second[4][2], 4);
  EXPECT_EQ(second[4][3], 6);

  EXPECT_EQ(second[5][0], 4);
  EXPECT_EQ(second[5][1], 5);
  EXPECT_EQ(second[5][2], 6);
  EXPECT_EQ(second[5][3], 7);

  n = 3;
  auto third = nekrs::nestedElementsOnFace(n);
  for (const auto & s : third)
    EXPECT_EQ(s.size(), 9);

  EXPECT_EQ(third[0][0], 0);
  EXPECT_EQ(third[0][1], 1);
  EXPECT_EQ(third[0][2], 2);
  EXPECT_EQ(third[0][3], 3);
  EXPECT_EQ(third[0][4], 4);
  EXPECT_EQ(third[0][5], 5);
  EXPECT_EQ(third[0][6], 6);
  EXPECT_EQ(third[0][7], 7);
  EXPECT_EQ(third[0][8], 8);

  EXPECT_EQ(third[1][0], 0);
  EXPECT_EQ(third[1][1], 1);
  EXPECT_EQ(third[1][2], 2);
  EXPECT_EQ(third[1][3], 9);
  EXPECT_EQ(third[1][4], 10);
  EXPECT_EQ(third[1][5], 11);
  EXPECT_EQ(third[1][6], 18);
  EXPECT_EQ(third[1][7], 19);
  EXPECT_EQ(third[1][8], 20);

  EXPECT_EQ(third[2][0], 2);
  EXPECT_EQ(third[2][1], 5);
  EXPECT_EQ(third[2][2], 8);
  EXPECT_EQ(third[2][3], 11);
  EXPECT_EQ(third[2][4], 14);
  EXPECT_EQ(third[2][5], 17);
  EXPECT_EQ(third[2][6], 20);
  EXPECT_EQ(third[2][7], 23);
  EXPECT_EQ(third[2][8], 26);

  EXPECT_EQ(third[3][0], 6);
  EXPECT_EQ(third[3][1], 7);
  EXPECT_EQ(third[3][2], 8);
  EXPECT_EQ(third[3][3], 15);
  EXPECT_EQ(third[3][4], 16);
  EXPECT_EQ(third[3][5], 17);
  EXPECT_EQ(third[3][6], 24);
  EXPECT_EQ(third[3][7], 25);
  EXPECT_EQ(third[3][8], 26);

  EXPECT_EQ(third[4][0], 0);
  EXPECT_EQ(third[4][1], 3);
  EXPECT_EQ(third[4][2], 6);
  EXPECT_EQ(third[4][3], 9);
  EXPECT_EQ(third[4][4], 12);
  EXPECT_EQ(third[4][5], 15);
  EXPECT_EQ(third[4][6], 18);
  EXPECT_EQ(third[4][7], 21);
  EXPECT_EQ(third[4][8], 24);

  EXPECT_EQ(third[5][0], 18);
  EXPECT_EQ(third[5][1], 19);
  EXPECT_EQ(third[5][2], 20);
  EXPECT_EQ(third[5][3], 21);
  EXPECT_EQ(third[5][4], 22);
  EXPECT_EQ(third[5][5], 23);
  EXPECT_EQ(third[5][6], 24);
  EXPECT_EQ(third[5][7], 25);
  EXPECT_EQ(third[5][8], 26);
}
