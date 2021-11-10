#include "HexagonalLatticeTest.h"

TEST_F(HexagonalLatticeTest, rings_and_pins)
{
  HexagonalLatticeUtility hl1(10.0, 0.1, 0.04, 0.01, 50.0, 1, 2);

  // Test number of rings given number of pins
  EXPECT_EQ(hl1.rings(1), 1);
  EXPECT_EQ(hl1.rings(7), 2);
  EXPECT_EQ(hl1.rings(19), 3);
  EXPECT_EQ(hl1.rings(37), 4);

  try
  {
    hl1.rings(100);
    FAIL() << "missing expected error";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("Number of pins 100 not evenly divisible in a hexagonal lattice!"), std::string::npos)
      << "failed with unexpected error: " << msg;
  }


  // Test number of pins in given ring
  EXPECT_EQ(hl1.pins(1), 1);
  EXPECT_EQ(hl1.pins(2), 6);
  EXPECT_EQ(hl1.pins(3), 12);
}

TEST_F(HexagonalLatticeTest, variation_with_rings)
{
  HexagonalLatticeUtility hl1(10.0, 0.1, 0.04, 0.01, 50.0, 1, 2);
  HexagonalLatticeUtility hl2(10.0, 0.1, 0.04, 0.01, 50.0, 2, 2);
  HexagonalLatticeUtility hl3(10.0, 0.1, 0.04, 0.01, 50.0, 3, 2);
  HexagonalLatticeUtility hl4(10.0, 0.1, 0.04, 0.01, 50.0, 4, 2);

  EXPECT_EQ(hl1.nPins(), 1);
  EXPECT_EQ(hl1.nInteriorPins(), 1);
  EXPECT_EQ(hl1.nEdgePins(), 0);
  EXPECT_EQ(hl1.nCornerPins(), 0);
  EXPECT_EQ(hl1.nChannels(), 6);
  EXPECT_EQ(hl1.nInteriorChannels(), 0);
  EXPECT_EQ(hl1.nEdgeChannels(), 0);
  EXPECT_EQ(hl1.nCornerChannels(), 6);

  EXPECT_EQ(hl2.nPins(), 7);
  EXPECT_EQ(hl2.nInteriorPins(), 1);
  EXPECT_EQ(hl2.nEdgePins(), 0);
  EXPECT_EQ(hl2.nCornerPins(), 6);
  EXPECT_EQ(hl2.nChannels(), 18);
  EXPECT_EQ(hl2.nInteriorChannels(), 6);
  EXPECT_EQ(hl2.nEdgeChannels(), 6);
  EXPECT_EQ(hl2.nCornerChannels(), 6);

  EXPECT_EQ(hl3.nPins(), 19);
  EXPECT_EQ(hl3.nInteriorPins(), 7);
  EXPECT_EQ(hl3.nEdgePins(), 6);
  EXPECT_EQ(hl3.nCornerPins(), 6);
  EXPECT_EQ(hl3.nChannels(), 42);
  EXPECT_EQ(hl3.nInteriorChannels(), 24);
  EXPECT_EQ(hl3.nEdgeChannels(), 12);
  EXPECT_EQ(hl3.nCornerChannels(), 6);

  EXPECT_EQ(hl4.nPins(), 37);
  EXPECT_EQ(hl4.nInteriorPins(), 19);
  EXPECT_EQ(hl4.nEdgePins(), 12);
  EXPECT_EQ(hl4.nCornerPins(), 6);
  EXPECT_EQ(hl4.nChannels(), 78);
  EXPECT_EQ(hl4.nInteriorChannels(), 54);
  EXPECT_EQ(hl4.nEdgeChannels(), 18);
  EXPECT_EQ(hl4.nCornerChannels(), 6);

  auto interior_pins2 = hl2.interiorChannelPinIndices();
  EXPECT_EQ(interior_pins2[0][0], 0);
  EXPECT_EQ(interior_pins2[0][1], 1);
  EXPECT_EQ(interior_pins2[0][2], 2);
  EXPECT_EQ(interior_pins2[1][0], 0);
  EXPECT_EQ(interior_pins2[1][1], 2);
  EXPECT_EQ(interior_pins2[1][2], 3);
  EXPECT_EQ(interior_pins2[2][0], 0);
  EXPECT_EQ(interior_pins2[2][1], 3);
  EXPECT_EQ(interior_pins2[2][2], 4);
  EXPECT_EQ(interior_pins2[3][0], 0);
  EXPECT_EQ(interior_pins2[3][1], 4);
  EXPECT_EQ(interior_pins2[3][2], 5);
  EXPECT_EQ(interior_pins2[4][0], 0);
  EXPECT_EQ(interior_pins2[4][1], 5);
  EXPECT_EQ(interior_pins2[4][2], 6);
  EXPECT_EQ(interior_pins2[5][0], 0);
  EXPECT_EQ(interior_pins2[5][1], 6);
  EXPECT_EQ(interior_pins2[5][2], 1);

  auto edge_pins2 = hl2.edgeChannelPinIndices();
  EXPECT_EQ(edge_pins2[0][0], 1);
  EXPECT_EQ(edge_pins2[0][1], 2);
  EXPECT_EQ(edge_pins2[1][0], 2);
  EXPECT_EQ(edge_pins2[1][1], 3);
  EXPECT_EQ(edge_pins2[2][0], 3);
  EXPECT_EQ(edge_pins2[2][1], 4);
  EXPECT_EQ(edge_pins2[3][0], 4);
  EXPECT_EQ(edge_pins2[3][1], 5);
  EXPECT_EQ(edge_pins2[4][0], 5);
  EXPECT_EQ(edge_pins2[4][1], 6);
  EXPECT_EQ(edge_pins2[5][0], 6);
  EXPECT_EQ(edge_pins2[5][1], 1);

  auto corner_pins2 = hl2.cornerChannelPinIndices();
  EXPECT_EQ(corner_pins2[0][0], 1);
  EXPECT_EQ(corner_pins2[1][0], 2);
  EXPECT_EQ(corner_pins2[2][0], 3);
  EXPECT_EQ(corner_pins2[3][0], 4);
  EXPECT_EQ(corner_pins2[4][0], 5);
  EXPECT_EQ(corner_pins2[5][0], 6);

  auto interior_pins3 = hl3.interiorChannelPinIndices();
  EXPECT_EQ(interior_pins3[0][0], 0);
  EXPECT_EQ(interior_pins3[0][1], 1);
  EXPECT_EQ(interior_pins3[0][2], 2);
  EXPECT_EQ(interior_pins3[1][0], 0);
  EXPECT_EQ(interior_pins3[1][1], 2);
  EXPECT_EQ(interior_pins3[1][2], 3);
  EXPECT_EQ(interior_pins3[2][0], 0);
  EXPECT_EQ(interior_pins3[2][1], 3);
  EXPECT_EQ(interior_pins3[2][2], 4);
  EXPECT_EQ(interior_pins3[3][0], 0);
  EXPECT_EQ(interior_pins3[3][1], 4);
  EXPECT_EQ(interior_pins3[3][2], 5);
  EXPECT_EQ(interior_pins3[4][0], 0);
  EXPECT_EQ(interior_pins3[4][1], 5);
  EXPECT_EQ(interior_pins3[4][2], 6);
  EXPECT_EQ(interior_pins3[5][0], 0);
  EXPECT_EQ(interior_pins3[5][1], 6);
  EXPECT_EQ(interior_pins3[5][2], 1);
  EXPECT_EQ(interior_pins3[6][0], 1);
  EXPECT_EQ(interior_pins3[6][1], 7);
  EXPECT_EQ(interior_pins3[6][2], 8);
  EXPECT_EQ(interior_pins3[7][0], 8);
  EXPECT_EQ(interior_pins3[7][1], 2);
  EXPECT_EQ(interior_pins3[7][2], 1);
  EXPECT_EQ(interior_pins3[8][0], 2);
  EXPECT_EQ(interior_pins3[8][1], 8);
  EXPECT_EQ(interior_pins3[8][2], 9);
  EXPECT_EQ(interior_pins3[9][0], 2);
  EXPECT_EQ(interior_pins3[9][1], 9);
  EXPECT_EQ(interior_pins3[9][2], 10);
  EXPECT_EQ(interior_pins3[10][0], 10);
  EXPECT_EQ(interior_pins3[10][1], 3);
  EXPECT_EQ(interior_pins3[10][2], 2);
  EXPECT_EQ(interior_pins3[11][0], 3);
  EXPECT_EQ(interior_pins3[11][1], 10);
  EXPECT_EQ(interior_pins3[11][2], 11);
  EXPECT_EQ(interior_pins3[12][0], 3);
  EXPECT_EQ(interior_pins3[12][1], 11);
  EXPECT_EQ(interior_pins3[12][2], 12);
  EXPECT_EQ(interior_pins3[13][0], 12);
  EXPECT_EQ(interior_pins3[13][1], 4);
  EXPECT_EQ(interior_pins3[13][2], 3);
  EXPECT_EQ(interior_pins3[14][0], 4);
  EXPECT_EQ(interior_pins3[14][1], 12);
  EXPECT_EQ(interior_pins3[14][2], 13);
  EXPECT_EQ(interior_pins3[15][0], 4);
  EXPECT_EQ(interior_pins3[15][1], 13);
  EXPECT_EQ(interior_pins3[15][2], 14);
  EXPECT_EQ(interior_pins3[16][0], 14);
  EXPECT_EQ(interior_pins3[16][1], 5);
  EXPECT_EQ(interior_pins3[16][2], 4);
  EXPECT_EQ(interior_pins3[17][0], 5);
  EXPECT_EQ(interior_pins3[17][1], 14);
  EXPECT_EQ(interior_pins3[17][2], 15);
  EXPECT_EQ(interior_pins3[18][0], 5);
  EXPECT_EQ(interior_pins3[18][1], 15);
  EXPECT_EQ(interior_pins3[18][2], 16);
  EXPECT_EQ(interior_pins3[19][0], 16);
  EXPECT_EQ(interior_pins3[19][1], 6);
  EXPECT_EQ(interior_pins3[19][2], 5);
  EXPECT_EQ(interior_pins3[20][0], 6);
  EXPECT_EQ(interior_pins3[20][1], 16);
  EXPECT_EQ(interior_pins3[20][2], 17);
  EXPECT_EQ(interior_pins3[21][0], 6);
  EXPECT_EQ(interior_pins3[21][1], 17);
  EXPECT_EQ(interior_pins3[21][2], 18);
  EXPECT_EQ(interior_pins3[22][0], 18);
  EXPECT_EQ(interior_pins3[22][1], 1);
  EXPECT_EQ(interior_pins3[22][2], 6);
  EXPECT_EQ(interior_pins3[23][0], 1);
  EXPECT_EQ(interior_pins3[23][1], 18);
  EXPECT_EQ(interior_pins3[23][2], 7);

  auto edge_pins3 = hl3.edgeChannelPinIndices();
  EXPECT_EQ(edge_pins3[0][0], 7);
  EXPECT_EQ(edge_pins3[0][1], 8);
  EXPECT_EQ(edge_pins3[1][0], 8);
  EXPECT_EQ(edge_pins3[1][1], 9);
  EXPECT_EQ(edge_pins3[2][0], 9);
  EXPECT_EQ(edge_pins3[2][1], 10);
  EXPECT_EQ(edge_pins3[3][0], 10);
  EXPECT_EQ(edge_pins3[3][1], 11);
  EXPECT_EQ(edge_pins3[4][0], 11);
  EXPECT_EQ(edge_pins3[4][1], 12);
  EXPECT_EQ(edge_pins3[5][0], 12);
  EXPECT_EQ(edge_pins3[5][1], 13);
  EXPECT_EQ(edge_pins3[6][0], 13);
  EXPECT_EQ(edge_pins3[6][1], 14);
  EXPECT_EQ(edge_pins3[7][0], 14);
  EXPECT_EQ(edge_pins3[7][1], 15);
  EXPECT_EQ(edge_pins3[8][0], 15);
  EXPECT_EQ(edge_pins3[8][1], 16);
  EXPECT_EQ(edge_pins3[9][0], 16);
  EXPECT_EQ(edge_pins3[9][1], 17);
  EXPECT_EQ(edge_pins3[10][0], 17);
  EXPECT_EQ(edge_pins3[10][1], 18);
  EXPECT_EQ(edge_pins3[11][0], 18);
  EXPECT_EQ(edge_pins3[11][1], 7);

  auto corner_pins3 = hl3.cornerChannelPinIndices();
  EXPECT_EQ(corner_pins3[0][0], 7);
  EXPECT_EQ(corner_pins3[1][0], 9);
  EXPECT_EQ(corner_pins3[2][0], 11);
  EXPECT_EQ(corner_pins3[3][0], 13);
  EXPECT_EQ(corner_pins3[4][0], 15);
  EXPECT_EQ(corner_pins3[5][0], 17);
}

TEST_F(HexagonalLatticeTest, pin_bundle_spacing)
{
  HexagonalLatticeUtility hl5(5.0, 1.0, 0.8, 0.05, 50.0, 1, 2);
  HexagonalLatticeUtility hl6(5.0, 1.0, 0.8, 0.05, 50.0, 2, 2);
  HexagonalLatticeUtility hl7(5.0, 1.0, 0.8, 0.00, 50.0 /* unused */, 1, 2);
  HexagonalLatticeUtility hl8(5.0, 1.0, 0.8, 0.00, 50.0 /* unused */, 2, 2);

  // wire-wrapped bundles
  EXPECT_DOUBLE_EQ(hl5.pinBundleSpacing(), 2.1);
  EXPECT_DOUBLE_EQ(hl6.pinBundleSpacing(), (5.0 - (std::sqrt(3.0) + 0.8)) / 2.);

  // bare bundles
  EXPECT_DOUBLE_EQ(hl7.pinBundleSpacing(), 2.1);
  EXPECT_DOUBLE_EQ(hl8.pinBundleSpacing(), (5.0 - (std::sqrt(3.0) + 0.8)) / 2.);
}

TEST_F(HexagonalLatticeTest, hydraulic_diameters)
{
  int n_interior, n_edge, n_corner;
  Real flow_interior, flow_edge, flow_corner;
  Real area_interior, area_edge, area_corner;

  HexagonalLatticeUtility hl9(5.0, 1.0, 0.8, 0.1, 50.0, 1, 2);
  HexagonalLatticeUtility hl10(5.0, 1.0, 0.8, 0.1, 50.0, 2, 2);
  HexagonalLatticeUtility hl11(5.0, 0.99, 0.8, 0.1, 50.0, 3, 2);
  HexagonalLatticeUtility hl12(5.0, 1.0, 0.8, 0.0, 50.0 /* unused */, 1, 2);
  HexagonalLatticeUtility hl13(5.0, 1.0, 0.8, 0.0, 50.0 /* unused */, 2, 2);
  HexagonalLatticeUtility hl14(5.0, 0.99, 0.8, 0.0, 50.0 /* unused */, 3, 2);

  // values are obtained from a completely separately-developed Python script for
  // verifying the bundle-wide hydraulic diameter. We verify the diameters for each
  // individual channel by requiring that the n-channel-weighted sum equals the
  // bundle-wide values for volume and areas, since we can verify that easily.
  EXPECT_DOUBLE_EQ(hl9.hydraulicDiameter(), 4.196872851813099);
  n_interior    = hl9.nInteriorChannels();
  n_edge        = hl9.nEdgeChannels();
  n_corner      = hl9.nCornerChannels();
  flow_interior = hl9.interiorFlowVolume();
  flow_edge     = hl9.edgeFlowVolume();
  flow_corner   = hl9.cornerFlowVolume();
  area_interior = hl9.interiorWettedArea();
  area_edge     = hl9.edgeWettedArea();
  area_corner   = hl9.cornerWettedArea();
  EXPECT_DOUBLE_EQ(hl9.flowVolume(), n_interior * flow_interior +
    n_edge * flow_edge + n_corner * flow_corner);
  EXPECT_DOUBLE_EQ(hl9.wettedArea(), n_interior * area_interior +
    n_edge * area_edge + n_corner * area_corner);

  EXPECT_DOUBLE_EQ(hl10.hydraulicDiameter(), 1.948158075579034);
  n_interior    = hl10.nInteriorChannels();
  n_edge        = hl10.nEdgeChannels();
  n_corner      = hl10.nCornerChannels();
  flow_interior = hl10.interiorFlowVolume();
  flow_edge     = hl10.edgeFlowVolume();
  flow_corner   = hl10.cornerFlowVolume();
  area_interior = hl10.interiorWettedArea();
  area_edge     = hl10.edgeWettedArea();
  area_corner   = hl10.cornerWettedArea();
  EXPECT_DOUBLE_EQ(hl10.flowVolume(), n_interior * flow_interior +
    n_edge * flow_edge + n_corner * flow_corner);
  EXPECT_DOUBLE_EQ(hl10.wettedArea(), n_interior * area_interior +
    n_edge * area_edge + n_corner * area_corner);

  EXPECT_DOUBLE_EQ(hl11.hydraulicDiameter(), 0.6727946134052672);
  n_interior    = hl11.nInteriorChannels();
  n_edge        = hl11.nEdgeChannels();
  n_corner      = hl11.nCornerChannels();
  flow_interior = hl11.interiorFlowVolume();
  flow_edge     = hl11.edgeFlowVolume();
  flow_corner   = hl11.cornerFlowVolume();
  area_interior = hl11.interiorWettedArea();
  area_edge     = hl11.edgeWettedArea();
  area_corner   = hl11.cornerWettedArea();
  EXPECT_DOUBLE_EQ(hl11.flowVolume(), n_interior * flow_interior +
    n_edge * flow_edge + n_corner * flow_corner);
  EXPECT_DOUBLE_EQ(hl11.wettedArea(), n_interior * area_interior +
    n_edge * area_edge + n_corner * area_corner);

  // bare bundles
  EXPECT_DOUBLE_EQ(hl12.hydraulicDiameter(), 4.2650423521483205);
  n_interior    = hl12.nInteriorChannels();
  n_edge        = hl12.nEdgeChannels();
  n_corner      = hl12.nCornerChannels();
  flow_interior = hl12.interiorFlowVolume();
  flow_edge     = hl12.edgeFlowVolume();
  flow_corner   = hl12.cornerFlowVolume();
  area_interior = hl12.interiorWettedArea();
  area_edge     = hl12.edgeWettedArea();
  area_corner   = hl12.cornerWettedArea();
  EXPECT_DOUBLE_EQ(hl12.flowVolume(), n_interior * flow_interior +
    n_edge * flow_edge + n_corner * flow_corner);
  EXPECT_DOUBLE_EQ(hl12.wettedArea(), n_interior * area_interior +
    n_edge * area_edge + n_corner * area_corner);

  EXPECT_DOUBLE_EQ(hl13.hydraulicDiameter(), 2.077372852104904);
  n_interior    = hl13.nInteriorChannels();
  n_edge        = hl13.nEdgeChannels();
  n_corner      = hl13.nCornerChannels();
  flow_interior = hl13.interiorFlowVolume();
  flow_edge     = hl13.edgeFlowVolume();
  flow_corner   = hl13.cornerFlowVolume();
  area_interior = hl13.interiorWettedArea();
  area_edge     = hl13.edgeWettedArea();
  area_corner   = hl13.cornerWettedArea();
  EXPECT_DOUBLE_EQ(hl13.flowVolume(), n_interior * flow_interior +
    n_edge * flow_edge + n_corner * flow_corner);
  EXPECT_DOUBLE_EQ(hl13.wettedArea(), n_interior * area_interior +
    n_edge * area_edge + n_corner * area_corner);

  EXPECT_DOUBLE_EQ(hl14.hydraulicDiameter(), 0.7437951937590452);
  n_interior    = hl14.nInteriorChannels();
  n_edge        = hl14.nEdgeChannels();
  n_corner      = hl14.nCornerChannels();
  flow_interior = hl14.interiorFlowVolume();
  flow_edge     = hl14.edgeFlowVolume();
  flow_corner   = hl14.cornerFlowVolume();
  area_interior = hl14.interiorWettedArea();
  area_edge     = hl14.edgeWettedArea();
  area_corner   = hl14.cornerWettedArea();
  EXPECT_DOUBLE_EQ(hl14.flowVolume(), n_interior * flow_interior +
    n_edge * flow_edge + n_corner * flow_corner);
  EXPECT_DOUBLE_EQ(hl14.wettedArea(), n_interior * area_interior +
    n_edge * area_edge + n_corner * area_corner);
}

TEST_F(HexagonalLatticeTest, pin_centers)
{
  HexagonalLatticeUtility hl14(5.0, 0.99, 0.8, 0.0, 50.0 /* unused */, 3, 2);
  HexagonalLatticeUtility hl15(5.0, 0.99, 0.8, 0.0, 50.0 /* unused */, 1, 2);
  HexagonalLatticeUtility hl16(5.0, 0.99, 0.8, 0.0, 50.0 /* unused */, 2, 2);
  HexagonalLatticeUtility hl17(10.0, 0.99, 0.8, 0.0, 50.0 /* unused */, 4, 2);

  Real cos60 = 0.5;
  Real sin60 = std::sqrt(3.0) / 2.0;

  auto & centers = hl15.pinCenters();
  EXPECT_EQ(centers.size(), hl15.nPins());
  EXPECT_DOUBLE_EQ(centers[0](0), 0.0);
  EXPECT_DOUBLE_EQ(centers[0](1), 0.0);
  EXPECT_DOUBLE_EQ(centers[0](2), 0.0);

  Real p = 0.99;
  auto & centers2 = hl16.pinCenters();
  EXPECT_EQ(centers2.size(), hl16.nPins());
  EXPECT_DOUBLE_EQ(centers2[0](0), 0.0);
  EXPECT_DOUBLE_EQ(centers2[0](1), 0.0);
  EXPECT_DOUBLE_EQ(centers2[1](0), cos60 * p);
  EXPECT_DOUBLE_EQ(centers2[1](1), sin60 * p);
  EXPECT_DOUBLE_EQ(centers2[2](0), -cos60 * p);
  EXPECT_DOUBLE_EQ(centers2[2](1), sin60 * p);
  EXPECT_DOUBLE_EQ(centers2[3](0), -p);
  EXPECT_DOUBLE_EQ(centers2[3](1), 0.0);
  EXPECT_DOUBLE_EQ(centers2[4](0), -cos60 * p);
  EXPECT_DOUBLE_EQ(centers2[4](1), -sin60 * p);
  EXPECT_DOUBLE_EQ(centers2[5](0), cos60 * p);
  EXPECT_DOUBLE_EQ(centers2[5](1), -sin60 * p);
  EXPECT_DOUBLE_EQ(centers2[6](0), p);
  EXPECT_DOUBLE_EQ(centers2[6](1), 0);

  for (int i = 0; i < hl16.nPins(); ++i)
    EXPECT_DOUBLE_EQ(centers2[i](2), 0.0);

  auto & centers3 = hl14.pinCenters();
  EXPECT_EQ(centers3.size(), hl14.nPins());
  EXPECT_DOUBLE_EQ(centers3[0](0), 0.0);
  EXPECT_DOUBLE_EQ(centers3[0](1), 0.0);
  EXPECT_DOUBLE_EQ(centers3[1](0), cos60 * p);
  EXPECT_DOUBLE_EQ(centers3[1](1), sin60 * p);
  EXPECT_DOUBLE_EQ(centers3[2](0), -cos60 * p);
  EXPECT_DOUBLE_EQ(centers3[2](1), sin60 * p);
  EXPECT_DOUBLE_EQ(centers3[3](0), -p);
  EXPECT_DOUBLE_EQ(centers3[3](1), 0.0);
  EXPECT_DOUBLE_EQ(centers3[4](0), -cos60 * p);
  EXPECT_DOUBLE_EQ(centers3[4](1), -sin60 * p);
  EXPECT_DOUBLE_EQ(centers3[5](0), cos60 * p);
  EXPECT_DOUBLE_EQ(centers3[5](1), -sin60 * p);
  EXPECT_DOUBLE_EQ(centers3[6](0), p);
  EXPECT_DOUBLE_EQ(centers3[6](1), 0);

  EXPECT_DOUBLE_EQ(centers3[7](0), p);
  EXPECT_DOUBLE_EQ(centers3[7](1), 2*p*sin60);
  EXPECT_DOUBLE_EQ(centers3[8](0), 0);
  EXPECT_DOUBLE_EQ(centers3[8](1), 2*p*sin60);
  EXPECT_DOUBLE_EQ(centers3[9](0), -p);
  EXPECT_DOUBLE_EQ(centers3[9](1), 2*p*sin60);
  EXPECT_DOUBLE_EQ(centers3[10](0), -p-p*cos60);
  EXPECT_DOUBLE_EQ(centers3[10](1), p*sin60);
  EXPECT_DOUBLE_EQ(centers3[11](0), -2*p);
  EXPECT_DOUBLE_EQ(centers3[11](1), 0);
  EXPECT_DOUBLE_EQ(centers3[12](0), -p-p*cos60);
  EXPECT_DOUBLE_EQ(centers3[12](1), -p*sin60);
  EXPECT_DOUBLE_EQ(centers3[13](0), -p);
  EXPECT_DOUBLE_EQ(centers3[13](1), -2*p*sin60);
  EXPECT_DOUBLE_EQ(centers3[14](0), 0);
  EXPECT_DOUBLE_EQ(centers3[14](1), -2*p*sin60);
  EXPECT_DOUBLE_EQ(centers3[15](0), p);
  EXPECT_DOUBLE_EQ(centers3[15](1), -2*p*sin60);
  EXPECT_DOUBLE_EQ(centers3[16](0), p+p*cos60);
  EXPECT_DOUBLE_EQ(centers3[16](1), -p*sin60);
  EXPECT_DOUBLE_EQ(centers3[17](0), 2*p);
  EXPECT_DOUBLE_EQ(centers3[17](1), 0.0);
  EXPECT_DOUBLE_EQ(centers3[18](0), p+p*cos60);
  EXPECT_DOUBLE_EQ(centers3[18](1), p*sin60);

  for (int i = 0; i < hl14.nPins(); ++i)
    EXPECT_DOUBLE_EQ(centers3[i](2), 0.0);

  auto & centers4 = hl17.pinCenters();
  EXPECT_EQ(centers4.size(), hl17.nPins());
  EXPECT_DOUBLE_EQ(centers4[0](0), 0.0);
  EXPECT_DOUBLE_EQ(centers4[0](1), 0.0);
  EXPECT_DOUBLE_EQ(centers4[1](0), cos60 * p);
  EXPECT_DOUBLE_EQ(centers4[1](1), sin60 * p);
  EXPECT_DOUBLE_EQ(centers4[2](0), -cos60 * p);
  EXPECT_DOUBLE_EQ(centers4[2](1), sin60 * p);
  EXPECT_DOUBLE_EQ(centers4[3](0), -p);
  EXPECT_DOUBLE_EQ(centers4[3](1), 0.0);
  EXPECT_DOUBLE_EQ(centers4[4](0), -cos60 * p);
  EXPECT_DOUBLE_EQ(centers4[4](1), -sin60 * p);
  EXPECT_DOUBLE_EQ(centers4[5](0), cos60 * p);
  EXPECT_DOUBLE_EQ(centers4[5](1), -sin60 * p);
  EXPECT_DOUBLE_EQ(centers4[6](0), p);
  EXPECT_DOUBLE_EQ(centers4[6](1), 0);

  EXPECT_DOUBLE_EQ(centers4[7](0), p);
  EXPECT_DOUBLE_EQ(centers4[7](1), 2*p*sin60);
  EXPECT_DOUBLE_EQ(centers4[8](0), 0);
  EXPECT_DOUBLE_EQ(centers4[8](1), 2*p*sin60);
  EXPECT_DOUBLE_EQ(centers4[9](0), -p);
  EXPECT_DOUBLE_EQ(centers4[9](1), 2*p*sin60);
  EXPECT_DOUBLE_EQ(centers4[10](0), -p-p*cos60);
  EXPECT_DOUBLE_EQ(centers4[10](1), p*sin60);
  EXPECT_DOUBLE_EQ(centers4[11](0), -2*p);
  EXPECT_DOUBLE_EQ(centers4[11](1), 0);
  EXPECT_DOUBLE_EQ(centers4[12](0), -p-p*cos60);
  EXPECT_DOUBLE_EQ(centers4[12](1), -p*sin60);
  EXPECT_DOUBLE_EQ(centers4[13](0), -p);
  EXPECT_DOUBLE_EQ(centers4[13](1), -2*p*sin60);
  EXPECT_DOUBLE_EQ(centers4[14](0), 0);
  EXPECT_DOUBLE_EQ(centers4[14](1), -2*p*sin60);
  EXPECT_DOUBLE_EQ(centers4[15](0), p);
  EXPECT_DOUBLE_EQ(centers4[15](1), -2*p*sin60);
  EXPECT_DOUBLE_EQ(centers4[16](0), p+p*cos60);
  EXPECT_DOUBLE_EQ(centers4[16](1), -p*sin60);
  EXPECT_DOUBLE_EQ(centers4[17](0), 2*p);
  EXPECT_DOUBLE_EQ(centers4[17](1), 0.0);
  EXPECT_DOUBLE_EQ(centers4[18](0), p+p*cos60);
  EXPECT_DOUBLE_EQ(centers4[18](1), p*sin60);

  EXPECT_DOUBLE_EQ(centers4[19](0), p+p*cos60);
  EXPECT_DOUBLE_EQ(centers4[19](1), 3*p*sin60);
  EXPECT_DOUBLE_EQ(centers4[20](0), p*cos60);
  EXPECT_DOUBLE_EQ(centers4[20](1), 3*p*sin60);
  EXPECT_DOUBLE_EQ(centers4[21](0), -p*cos60);
  EXPECT_DOUBLE_EQ(centers4[21](1), 3*p*sin60);
  EXPECT_DOUBLE_EQ(centers4[22](0), -p-p*cos60);
  EXPECT_DOUBLE_EQ(centers4[22](1), 3*p*sin60);
  EXPECT_DOUBLE_EQ(centers4[23](0), -2*p);
  EXPECT_DOUBLE_EQ(centers4[23](1), 2*p*sin60);
  EXPECT_DOUBLE_EQ(centers4[24](0), -2*p-p*cos60);
  EXPECT_DOUBLE_EQ(centers4[24](1), p*sin60);
  EXPECT_DOUBLE_EQ(centers4[25](0), -3*p);
  EXPECT_DOUBLE_EQ(centers4[25](1), 0);
  EXPECT_DOUBLE_EQ(centers4[26](0), -2*p-p*cos60);
  EXPECT_DOUBLE_EQ(centers4[26](1), -p*sin60);
  EXPECT_DOUBLE_EQ(centers4[27](0), -2*p);
  EXPECT_DOUBLE_EQ(centers4[27](1), -2*p*sin60);
  EXPECT_DOUBLE_EQ(centers4[28](0), -p-p*cos60);
  EXPECT_DOUBLE_EQ(centers4[28](1), -3*p*sin60);
  EXPECT_DOUBLE_EQ(centers4[29](0), -p*cos60);
  EXPECT_DOUBLE_EQ(centers4[29](1), -3*p*sin60);
  EXPECT_DOUBLE_EQ(centers4[30](0), p*cos60);
  EXPECT_DOUBLE_EQ(centers4[30](1), -3*p*sin60);
  EXPECT_DOUBLE_EQ(centers4[31](0), p+p*cos60);
  EXPECT_DOUBLE_EQ(centers4[31](1), -3*p*sin60);
  EXPECT_DOUBLE_EQ(centers4[32](0), 2*p);
  EXPECT_DOUBLE_EQ(centers4[32](1), -2*p*sin60);
  EXPECT_DOUBLE_EQ(centers4[33](0), 2*p+p*cos60);
  EXPECT_DOUBLE_EQ(centers4[33](1), -p*sin60);
  EXPECT_DOUBLE_EQ(centers4[34](0), 3*p);
  EXPECT_DOUBLE_EQ(centers4[34](1), 0);
  EXPECT_DOUBLE_EQ(centers4[35](0), 2*p+p*cos60);
  EXPECT_DOUBLE_EQ(centers4[35](1), p*sin60);
  EXPECT_DOUBLE_EQ(centers4[36](0), 2*p);
  EXPECT_DOUBLE_EQ(centers4[36](1), 2*p*sin60);

  for (int i = 0; i < hl17.nPins(); ++i)
    EXPECT_DOUBLE_EQ(centers4[i](2), 0.0);
}

TEST_F(HexagonalLatticeTest, point_in_polygon)
{
  HexagonalLatticeUtility hl(5.0, 0.99, 0.8, 0.05, 50.0, 3, 2);

  // triangle
  Point pt1(1.0, 1.0, 0.0);
  Point pt2(3.0, 2.0, 0.0);
  Point pt3(2.0, 2.0, 0.0);

  Point pt_in(2.0, 1.9, 0.0);
  Point pt_not_in(2.0, 3.0, 0.0);
  Point pt_edge = pt1;

  EXPECT_TRUE(hl.pointInPolygon(pt_in, {pt1, pt2, pt3}));
  EXPECT_FALSE(hl.pointInPolygon(pt_not_in, {pt1, pt2, pt3}));
  EXPECT_TRUE(hl.pointInPolygon(pt_edge, {pt1, pt2, pt3}));

  // rectangle
  Point pt4(1.0, 2.0, 0.0);
  Point pt5(2.0, 1.0, 0.0);
  Point pt6(4.0, 3.0, 0.0);
  Point pt7(3.0, 4.0, 0.0);

  Point pt_in1(2.0, 2.0, 0.0);
  Point pt_not_in1(3.0, 1.0, 0.0);
  pt_edge = pt5;

  EXPECT_TRUE(hl.pointInPolygon(pt_in1, {pt4, pt5, pt6, pt7}));
  EXPECT_FALSE(hl.pointInPolygon(pt_not_in1, {pt4, pt5, pt6, pt7}));
  EXPECT_TRUE(hl.pointInPolygon(pt_edge, {pt4, pt5, pt6, pt7}));

  // general polygon
  Point pt8(1.0, 2.0, 0.0);
  Point pt9(2.0, 3.0, 0.0);
  Point pt10(3.0, 3.0, 0.0);
  Point pt11(3.0, 1.0, 0.0);

  Point pt_in2(2.0, 2.0, 0.0);
  Point pt_not_in2(1.0, 3.0, 0.0);
  Point pt_edge2(3.0, 2.0, 0.0);

  EXPECT_TRUE(hl.pointInPolygon(pt_in2, {pt8, pt9, pt10, pt11}));
  EXPECT_FALSE(hl.pointInPolygon(pt_not_in2, {pt8, pt9, pt10, pt11}));
  EXPECT_TRUE(hl.pointInPolygon(pt_edge2, {pt8, pt9, pt10, pt11}));
}

TEST_F(HexagonalLatticeTest, channel_index)
{
  HexagonalLatticeUtility hl(4.0, 0.8, 0.6, 0.05, 50.0, 3, 2);

  Point pt0(0.06, 0.35, 0.0);
  Point pt1(-0.47, 0.28, 0.0);
  Point pt2(-0.26, -0.21, 0.0);
  Point pt3(-0.12, -0.349, 0.0);
  Point pt4(0.46, -0.27, 0.0);
  Point pt5(0.37, 0.6, 0.0);
  EXPECT_EQ(hl.channelIndex(pt0), 0);
  EXPECT_EQ(hl.channelIndex(pt1), 1);
  EXPECT_EQ(hl.channelIndex(pt2), 2);
  EXPECT_EQ(hl.channelIndex(pt3), 3);
  EXPECT_EQ(hl.channelIndex(pt4), 4);
  EXPECT_EQ(hl.channelIndex(pt5), 5);

  // check that the z-coordinate doesn't affect the channel index identification
  EXPECT_EQ(hl.channelIndex(pt0 + Point(0.0, 0.0, 3.5)), 0);
  EXPECT_EQ(hl.channelIndex(pt1 + Point(0.0, 0.0, 3.5)), 1);
  EXPECT_EQ(hl.channelIndex(pt2 + Point(0.0, 0.0, 3.5)), 2);
  EXPECT_EQ(hl.channelIndex(pt3 + Point(0.0, 0.0, 3.5)), 3);
  EXPECT_EQ(hl.channelIndex(pt4 + Point(0.0, 0.0, 3.5)), 4);
  EXPECT_EQ(hl.channelIndex(pt5 + Point(0.0, 0.0, 3.5)), 5);

  Point pt6(0.36, 1.06, 0.0);
  Point pt7(0.11, 0.98, 0.0);
  Point pt8(-0.43, 1.27, 0.0);
  Point pt9(-0.81, 0.93, 0.0);
  Point pt10(-0.75, 0.47, 0.0);
  Point pt11(-1.06, 0.28, 0.0);
  Point pt12(-1.16, -0.13, 0.0);
  Point pt13(-0.73, -0.41, 0.0);
  Point pt14(-0.73, -0.81, 0.0);
  Point pt15(-0.46, -1.18, 0.0);
  Point pt16(0.05, -0.98, 0.0);
  Point pt17(0.27, -1.00, 0.0);
  Point pt18(0.72, -0.98, 0.0);
  Point pt19(0.75, -0.58, 0.0);
  Point pt20(1.23, -0.23, 0.0);
  Point pt21(1.17, 0.09, 0.0);
  Point pt22(0.78, 0.38, 0.0);
  Point pt23(0.74, 0.84, 0.0);

  EXPECT_EQ(hl.channelIndex(pt6), 6);
  EXPECT_EQ(hl.channelIndex(pt7), 7);
  EXPECT_EQ(hl.channelIndex(pt8), 8);
  EXPECT_EQ(hl.channelIndex(pt9), 9);
  EXPECT_EQ(hl.channelIndex(pt10), 10);
  EXPECT_EQ(hl.channelIndex(pt11), 11);
  EXPECT_EQ(hl.channelIndex(pt12), 12);
  EXPECT_EQ(hl.channelIndex(pt13), 13);
  EXPECT_EQ(hl.channelIndex(pt14), 14);
  EXPECT_EQ(hl.channelIndex(pt15), 15);
  EXPECT_EQ(hl.channelIndex(pt16), 16);
  EXPECT_EQ(hl.channelIndex(pt17), 17);
  EXPECT_EQ(hl.channelIndex(pt18), 18);
  EXPECT_EQ(hl.channelIndex(pt19), 19);
  EXPECT_EQ(hl.channelIndex(pt20), 20);
  EXPECT_EQ(hl.channelIndex(pt21), 21);
  EXPECT_EQ(hl.channelIndex(pt22), 22);
  EXPECT_EQ(hl.channelIndex(pt23), 23);

  EXPECT_EQ(hl.channelIndex(pt6 + Point(0.0, 0.0, -7.0)), 6);
  EXPECT_EQ(hl.channelIndex(pt7 + Point(0.0, 0.0, -7.0)), 7);
  EXPECT_EQ(hl.channelIndex(pt8 + Point(0.0, 0.0, -7.0)), 8);
  EXPECT_EQ(hl.channelIndex(pt9 + Point(0.0, 0.0, -7.0)), 9);
  EXPECT_EQ(hl.channelIndex(pt10 + Point(0.0, 0.0, -7.0)), 10);
  EXPECT_EQ(hl.channelIndex(pt11 + Point(0.0, 0.0, -7.0)), 11);
  EXPECT_EQ(hl.channelIndex(pt12 + Point(0.0, 0.0, -7.0)), 12);
  EXPECT_EQ(hl.channelIndex(pt13 + Point(0.0, 0.0, -7.0)), 13);
  EXPECT_EQ(hl.channelIndex(pt14 + Point(0.0, 0.0, -7.0)), 14);
  EXPECT_EQ(hl.channelIndex(pt15 + Point(0.0, 0.0, -7.0)), 15);
  EXPECT_EQ(hl.channelIndex(pt16 + Point(0.0, 0.0, -7.0)), 16);
  EXPECT_EQ(hl.channelIndex(pt17 + Point(0.0, 0.0, -7.0)), 17);
  EXPECT_EQ(hl.channelIndex(pt18 + Point(0.0, 0.0, -7.0)), 18);
  EXPECT_EQ(hl.channelIndex(pt19 + Point(0.0, 0.0, -7.0)), 19);
  EXPECT_EQ(hl.channelIndex(pt20 + Point(0.0, 0.0, -7.0)), 20);
  EXPECT_EQ(hl.channelIndex(pt21 + Point(0.0, 0.0, -7.0)), 21);
  EXPECT_EQ(hl.channelIndex(pt22 + Point(0.0, 0.0, -7.0)), 22);
  EXPECT_EQ(hl.channelIndex(pt23 + Point(0.0, 0.0, -7.0)), 23);

  Point pt24(0.31, 1.44, 0.0);
  Point pt25(-0.38, 1.61, 0.0);
  Point pt26(-1.17, 1.52, 0.0);
  Point pt27(-1.78, 0.38, 0.0);
  Point pt28(-1.91, -0.42, 0.0);
  Point pt29(-1.39, -1.24, 0.0);
  Point pt30(-0.46, -1.74, 0.0);
  Point pt31(0.18, -1.79, 0.0);
  Point pt32(1.24, -1.17, 0.0);
  Point pt33(1.75, -0.57, 0.0);
  Point pt34(1.51, 0.37, 0.0);
  Point pt35(1.16, 1.42, 0.0);

  EXPECT_EQ(hl.channelIndex(pt24), 24);
  EXPECT_EQ(hl.channelIndex(pt25), 25);
  EXPECT_EQ(hl.channelIndex(pt26), 26);
  EXPECT_EQ(hl.channelIndex(pt27), 27);
  EXPECT_EQ(hl.channelIndex(pt28), 28);
  EXPECT_EQ(hl.channelIndex(pt29), 29);
  EXPECT_EQ(hl.channelIndex(pt30), 30);
  EXPECT_EQ(hl.channelIndex(pt31), 31);
  EXPECT_EQ(hl.channelIndex(pt32), 32);
  EXPECT_EQ(hl.channelIndex(pt33), 33);
  EXPECT_EQ(hl.channelIndex(pt34), 34);
  EXPECT_EQ(hl.channelIndex(pt35), 35);

  EXPECT_EQ(hl.channelIndex(pt24 + Point(0.0, 0.0, -0.1)), 24);
  EXPECT_EQ(hl.channelIndex(pt25 + Point(0.0, 0.0, -0.1)), 25);
  EXPECT_EQ(hl.channelIndex(pt26 + Point(0.0, 0.0, -0.1)), 26);
  EXPECT_EQ(hl.channelIndex(pt27 + Point(0.0, 0.0, -0.1)), 27);
  EXPECT_EQ(hl.channelIndex(pt28 + Point(0.0, 0.0, -0.1)), 28);
  EXPECT_EQ(hl.channelIndex(pt29 + Point(0.0, 0.0, -0.1)), 29);
  EXPECT_EQ(hl.channelIndex(pt30 + Point(0.0, 0.0, -0.1)), 30);
  EXPECT_EQ(hl.channelIndex(pt31 + Point(0.0, 0.0, -0.1)), 31);
  EXPECT_EQ(hl.channelIndex(pt32 + Point(0.0, 0.0, -0.1)), 32);
  EXPECT_EQ(hl.channelIndex(pt33 + Point(0.0, 0.0, -0.1)), 33);
  EXPECT_EQ(hl.channelIndex(pt34 + Point(0.0, 0.0, -0.1)), 34);
  EXPECT_EQ(hl.channelIndex(pt35 + Point(0.0, 0.0, -0.1)), 35);

  Point pt36(1.05, 1.75, 0.0);
  Point pt37(-1.02, 1.72, 0.0);
  Point pt38(-2.03, -0.05, 0.0);
  Point pt39(-1.01, -1.59, 0.0);
  Point pt40(0.89, -1.79, 0.0);
  Point pt41(1.98, 0.12, 0.0);

  EXPECT_EQ(hl.channelIndex(pt36), 36);
  EXPECT_EQ(hl.channelIndex(pt37), 37);
  EXPECT_EQ(hl.channelIndex(pt38), 38);
  EXPECT_EQ(hl.channelIndex(pt39), 39);
  EXPECT_EQ(hl.channelIndex(pt40), 40);
  EXPECT_EQ(hl.channelIndex(pt41), 41);

  EXPECT_EQ(hl.channelIndex(pt36 + Point(0.0, 0.0, 1.2)), 36);
  EXPECT_EQ(hl.channelIndex(pt37 + Point(0.0, 0.0, 1.2)), 37);
  EXPECT_EQ(hl.channelIndex(pt38 + Point(0.0, 0.0, 1.2)), 38);
  EXPECT_EQ(hl.channelIndex(pt39 + Point(0.0, 0.0, 1.2)), 39);
  EXPECT_EQ(hl.channelIndex(pt40 + Point(0.0, 0.0, 1.2)), 40);
  EXPECT_EQ(hl.channelIndex(pt41 + Point(0.0, 0.0, 1.2)), 41);
}

TEST_F(HexagonalLatticeTest, gaps1)
{
  HexagonalLatticeUtility hl(4.0, 0.8, 0.6, 0.05, 50.0, 1, 2);
  const auto & gi = hl.gapIndices();
  const auto & lg = hl.localToGlobalGaps();

  int i = 0;
  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, -1);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, -2);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, -3);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, -4);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, -5);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, -6);

  i = 0;
  EXPECT_EQ(lg[i][0], 5);
  EXPECT_EQ(lg[i++][1], 0);

  EXPECT_EQ(lg[i][0], 0);
  EXPECT_EQ(lg[i++][1], 1);

  EXPECT_EQ(lg[i][0], 1);
  EXPECT_EQ(lg[i++][1], 2);

  EXPECT_EQ(lg[i][0], 2);
  EXPECT_EQ(lg[i++][1], 3);

  EXPECT_EQ(lg[i][0], 3);
  EXPECT_EQ(lg[i++][1], 4);

  EXPECT_EQ(lg[i][0], 4);
  EXPECT_EQ(lg[i++][1], 5);
}

TEST_F(HexagonalLatticeTest, gaps2)
{
  HexagonalLatticeUtility hl(4.0, 0.8, 0.6, 0.05, 50.0, 2, 2);
  const auto & gi = hl.gapIndices();
  const auto & lg = hl.localToGlobalGaps();

  int i = 0;
  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 1);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 2);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 3);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 4);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 5);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 6);

  EXPECT_EQ(gi[i].first, 1);
  EXPECT_EQ(gi[i++].second, 2);

  EXPECT_EQ(gi[i].first, 1);
  EXPECT_EQ(gi[i++].second, 6);

  EXPECT_EQ(gi[i].first, 2);
  EXPECT_EQ(gi[i++].second, 3);

  EXPECT_EQ(gi[i].first, 3);
  EXPECT_EQ(gi[i++].second, 4);

  EXPECT_EQ(gi[i].first, 4);
  EXPECT_EQ(gi[i++].second, 5);

  EXPECT_EQ(gi[i].first, 5);
  EXPECT_EQ(gi[i++].second, 6);

  EXPECT_EQ(gi[i].first, 1);
  EXPECT_EQ(gi[i++].second, -1);

  EXPECT_EQ(gi[i].first, 2);
  EXPECT_EQ(gi[i++].second, -1);

  EXPECT_EQ(gi[i].first, 2);
  EXPECT_EQ(gi[i++].second, -2);

  EXPECT_EQ(gi[i].first, 3);
  EXPECT_EQ(gi[i++].second, -2);

  EXPECT_EQ(gi[i].first, 3);
  EXPECT_EQ(gi[i++].second, -3);

  EXPECT_EQ(gi[i].first, 4);
  EXPECT_EQ(gi[i++].second, -3);

  EXPECT_EQ(gi[i].first, 4);
  EXPECT_EQ(gi[i++].second, -4);

  EXPECT_EQ(gi[i].first, 5);
  EXPECT_EQ(gi[i++].second, -4);

  EXPECT_EQ(gi[i].first, 5);
  EXPECT_EQ(gi[i++].second, -5);

  EXPECT_EQ(gi[i].first, 6);
  EXPECT_EQ(gi[i++].second, -5);

  EXPECT_EQ(gi[i].first, 6);
  EXPECT_EQ(gi[i++].second, -6);

  EXPECT_EQ(gi[i].first, 1);
  EXPECT_EQ(gi[i++].second, -6);

  i = 0;
  EXPECT_EQ(lg[i][0], 0);
  EXPECT_EQ(lg[i][1], 6);
  EXPECT_EQ(lg[i++][2], 1);

  EXPECT_EQ(lg[i][0], 1);
  EXPECT_EQ(lg[i][1], 8);
  EXPECT_EQ(lg[i++][2], 2);

  EXPECT_EQ(lg[i][0], 2);
  EXPECT_EQ(lg[i][1], 9);
  EXPECT_EQ(lg[i++][2], 3);

  EXPECT_EQ(lg[i][0], 3);
  EXPECT_EQ(lg[i][1], 10);
  EXPECT_EQ(lg[i++][2], 4);

  EXPECT_EQ(lg[i][0], 4);
  EXPECT_EQ(lg[i][1], 11);
  EXPECT_EQ(lg[i++][2], 5);

  EXPECT_EQ(lg[i][0], 5);
  EXPECT_EQ(lg[i][1], 7);
  EXPECT_EQ(lg[i++][2], 0);

  EXPECT_EQ(lg[i][0], 6);
  EXPECT_EQ(lg[i][1], 13);
  EXPECT_EQ(lg[i++][2], 12);

  EXPECT_EQ(lg[i][0], 8);
  EXPECT_EQ(lg[i][1], 15);
  EXPECT_EQ(lg[i++][2], 14);

  EXPECT_EQ(lg[i][0], 9);
  EXPECT_EQ(lg[i][1], 17);
  EXPECT_EQ(lg[i++][2], 16);

  EXPECT_EQ(lg[i][0], 10);
  EXPECT_EQ(lg[i][1], 19);
  EXPECT_EQ(lg[i++][2], 18);

  EXPECT_EQ(lg[i][0], 11);
  EXPECT_EQ(lg[i][1], 21);
  EXPECT_EQ(lg[i++][2], 20);

  EXPECT_EQ(lg[i][0], 7);
  EXPECT_EQ(lg[i][1], 23);
  EXPECT_EQ(lg[i++][2], 22);

  EXPECT_EQ(lg[i][0], 23);
  EXPECT_EQ(lg[i++][1], 12);

  EXPECT_EQ(lg[i][0], 13);
  EXPECT_EQ(lg[i++][1], 14);

  EXPECT_EQ(lg[i][0], 15);
  EXPECT_EQ(lg[i++][1], 16);

  EXPECT_EQ(lg[i][0], 17);
  EXPECT_EQ(lg[i++][1], 18);

  EXPECT_EQ(lg[i][0], 19);
  EXPECT_EQ(lg[i++][1], 20);

  EXPECT_EQ(lg[i][0], 21);
  EXPECT_EQ(lg[i++][1], 22);
}

TEST_F(HexagonalLatticeTest, gaps3)
{
  HexagonalLatticeUtility hl(4.0, 0.8, 0.6, 0.05, 50.0, 3, 2);
  const auto & gi = hl.gapIndices();
  const auto & lg = hl.localToGlobalGaps();

  int i = 0;
  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 1);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 2);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 3);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 4);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 5);

  EXPECT_EQ(gi[i].first, 0);
  EXPECT_EQ(gi[i++].second, 6);

  EXPECT_EQ(gi[i].first, 1);
  EXPECT_EQ(gi[i++].second, 2);

  EXPECT_EQ(gi[i].first, 1);
  EXPECT_EQ(gi[i++].second, 6);

  EXPECT_EQ(gi[i].first, 1);
  EXPECT_EQ(gi[i++].second, 7);

  EXPECT_EQ(gi[i].first, 1);
  EXPECT_EQ(gi[i++].second, 8);

  EXPECT_EQ(gi[i].first, 1);
  EXPECT_EQ(gi[i++].second, 18);

  EXPECT_EQ(gi[i].first, 2);
  EXPECT_EQ(gi[i++].second, 3);

  EXPECT_EQ(gi[i].first, 2);
  EXPECT_EQ(gi[i++].second, 8);

  EXPECT_EQ(gi[i].first, 2);
  EXPECT_EQ(gi[i++].second, 9);

  EXPECT_EQ(gi[i].first, 2);
  EXPECT_EQ(gi[i++].second, 10);

  EXPECT_EQ(gi[i].first, 3);
  EXPECT_EQ(gi[i++].second, 4);

  EXPECT_EQ(gi[i].first, 3);
  EXPECT_EQ(gi[i++].second, 10);

  EXPECT_EQ(gi[i].first, 3);
  EXPECT_EQ(gi[i++].second, 11);

  EXPECT_EQ(gi[i].first, 3);
  EXPECT_EQ(gi[i++].second, 12);

  EXPECT_EQ(gi[i].first, 4);
  EXPECT_EQ(gi[i++].second, 5);

  EXPECT_EQ(gi[i].first, 4);
  EXPECT_EQ(gi[i++].second, 12);

  EXPECT_EQ(gi[i].first, 4);
  EXPECT_EQ(gi[i++].second, 13);

  EXPECT_EQ(gi[i].first, 4);
  EXPECT_EQ(gi[i++].second, 14);

  EXPECT_EQ(gi[i].first, 5);
  EXPECT_EQ(gi[i++].second, 6);

  EXPECT_EQ(gi[i].first, 5);
  EXPECT_EQ(gi[i++].second, 14);

  EXPECT_EQ(gi[i].first, 5);
  EXPECT_EQ(gi[i++].second, 15);

  EXPECT_EQ(gi[i].first, 5);
  EXPECT_EQ(gi[i++].second, 16);

  EXPECT_EQ(gi[i].first, 6);
  EXPECT_EQ(gi[i++].second, 16);

  EXPECT_EQ(gi[i].first, 6);
  EXPECT_EQ(gi[i++].second, 17);

  EXPECT_EQ(gi[i].first, 6);
  EXPECT_EQ(gi[i++].second, 18);

  EXPECT_EQ(gi[i].first, 7);
  EXPECT_EQ(gi[i++].second, 8);

  EXPECT_EQ(gi[i].first, 7);
  EXPECT_EQ(gi[i++].second, 18);

  EXPECT_EQ(gi[i].first, 8);
  EXPECT_EQ(gi[i++].second, 9);

  EXPECT_EQ(gi[i].first, 9);
  EXPECT_EQ(gi[i++].second, 10);

  EXPECT_EQ(gi[i].first, 10);
  EXPECT_EQ(gi[i++].second, 11);

  EXPECT_EQ(gi[i].first, 11);
  EXPECT_EQ(gi[i++].second, 12);

  EXPECT_EQ(gi[i].first, 12);
  EXPECT_EQ(gi[i++].second, 13);

  EXPECT_EQ(gi[i].first, 13);
  EXPECT_EQ(gi[i++].second, 14);

  EXPECT_EQ(gi[i].first, 14);
  EXPECT_EQ(gi[i++].second, 15);

  EXPECT_EQ(gi[i].first, 15);
  EXPECT_EQ(gi[i++].second, 16);

  EXPECT_EQ(gi[i].first, 16);
  EXPECT_EQ(gi[i++].second, 17);

  EXPECT_EQ(gi[i].first, 17);
  EXPECT_EQ(gi[i++].second, 18);

  EXPECT_EQ(gi[i].first, 7);
  EXPECT_EQ(gi[i++].second, -1);

  EXPECT_EQ(gi[i].first, 8);
  EXPECT_EQ(gi[i++].second, -1);

  EXPECT_EQ(gi[i].first, 9);
  EXPECT_EQ(gi[i++].second, -1);

  EXPECT_EQ(gi[i].first, 9);
  EXPECT_EQ(gi[i++].second, -2);

  EXPECT_EQ(gi[i].first, 10);
  EXPECT_EQ(gi[i++].second, -2);

  EXPECT_EQ(gi[i].first, 11);
  EXPECT_EQ(gi[i++].second, -2);

  EXPECT_EQ(gi[i].first, 11);
  EXPECT_EQ(gi[i++].second, -3);

  EXPECT_EQ(gi[i].first, 12);
  EXPECT_EQ(gi[i++].second, -3);

  EXPECT_EQ(gi[i].first, 13);
  EXPECT_EQ(gi[i++].second, -3);

  EXPECT_EQ(gi[i].first, 13);
  EXPECT_EQ(gi[i++].second, -4);

  EXPECT_EQ(gi[i].first, 14);
  EXPECT_EQ(gi[i++].second, -4);

  EXPECT_EQ(gi[i].first, 15);
  EXPECT_EQ(gi[i++].second, -4);

  EXPECT_EQ(gi[i].first, 15);
  EXPECT_EQ(gi[i++].second, -5);

  EXPECT_EQ(gi[i].first, 16);
  EXPECT_EQ(gi[i++].second, -5);

  EXPECT_EQ(gi[i].first, 17);
  EXPECT_EQ(gi[i++].second, -5);

  EXPECT_EQ(gi[i].first, 17);
  EXPECT_EQ(gi[i++].second, -6);

  EXPECT_EQ(gi[i].first, 18);
  EXPECT_EQ(gi[i++].second, -6);

  EXPECT_EQ(gi[i].first, 7);
  EXPECT_EQ(gi[i++].second, -6);

  i = 0;
  EXPECT_EQ(lg[i][0], 0);
  EXPECT_EQ(lg[i][1], 6);
  EXPECT_EQ(lg[i++][2], 1);

  EXPECT_EQ(lg[i][0], 1);
  EXPECT_EQ(lg[i][1], 11);
  EXPECT_EQ(lg[i++][2], 2);

  EXPECT_EQ(lg[i][0], 2);
  EXPECT_EQ(lg[i][1], 15);
  EXPECT_EQ(lg[i++][2], 3);

  EXPECT_EQ(lg[i][0], 3);
  EXPECT_EQ(lg[i][1], 19);
  EXPECT_EQ(lg[i++][2], 4);

  EXPECT_EQ(lg[i][0], 4);
  EXPECT_EQ(lg[i][1], 23);
  EXPECT_EQ(lg[i++][2], 5);

  EXPECT_EQ(lg[i][0], 5);
  EXPECT_EQ(lg[i][1], 7);
  EXPECT_EQ(lg[i++][2], 0);

  EXPECT_EQ(lg[i][0], 8);
  EXPECT_EQ(lg[i][1], 30);
  EXPECT_EQ(lg[i++][2], 9);

  EXPECT_EQ(lg[i][0], 12);
  EXPECT_EQ(lg[i][1], 6);
  EXPECT_EQ(lg[i++][2], 9);

  EXPECT_EQ(lg[i][0], 12);
  EXPECT_EQ(lg[i][1], 32);
  EXPECT_EQ(lg[i++][2], 13);

  EXPECT_EQ(lg[i][0], 13);
  EXPECT_EQ(lg[i][1], 33);
  EXPECT_EQ(lg[i++][2], 14);

  EXPECT_EQ(lg[i][0], 16);
  EXPECT_EQ(lg[i][1], 11);
  EXPECT_EQ(lg[i++][2], 14);

  EXPECT_EQ(lg[i][0], 16);
  EXPECT_EQ(lg[i][1], 34);
  EXPECT_EQ(lg[i++][2], 17);

  EXPECT_EQ(lg[i][0], 17);
  EXPECT_EQ(lg[i][1], 35);
  EXPECT_EQ(lg[i++][2], 18);

  EXPECT_EQ(lg[i][0], 20);
  EXPECT_EQ(lg[i][1], 15);
  EXPECT_EQ(lg[i++][2], 18);

  EXPECT_EQ(lg[i][0], 20);
  EXPECT_EQ(lg[i][1], 36);
  EXPECT_EQ(lg[i++][2], 21);

  EXPECT_EQ(lg[i][0], 21);
  EXPECT_EQ(lg[i][1], 37);
  EXPECT_EQ(lg[i++][2], 22);

  EXPECT_EQ(lg[i][0], 24);
  EXPECT_EQ(lg[i][1], 19);
  EXPECT_EQ(lg[i++][2], 22);

  EXPECT_EQ(lg[i][0], 24);
  EXPECT_EQ(lg[i][1], 38);
  EXPECT_EQ(lg[i++][2], 25);

  EXPECT_EQ(lg[i][0], 25);
  EXPECT_EQ(lg[i][1], 39);
  EXPECT_EQ(lg[i++][2], 26);

  EXPECT_EQ(lg[i][0], 27);
  EXPECT_EQ(lg[i][1], 23);
  EXPECT_EQ(lg[i++][2], 26);

  EXPECT_EQ(lg[i][0], 27);
  EXPECT_EQ(lg[i][1], 40);
  EXPECT_EQ(lg[i++][2], 28);

  EXPECT_EQ(lg[i][0], 28);
  EXPECT_EQ(lg[i][1], 41);
  EXPECT_EQ(lg[i++][2], 29);

  EXPECT_EQ(lg[i][0], 10);
  EXPECT_EQ(lg[i][1], 7);
  EXPECT_EQ(lg[i++][2], 29);

  EXPECT_EQ(lg[i][0], 10);
  EXPECT_EQ(lg[i][1], 31);
  EXPECT_EQ(lg[i++][2], 8);

  // edge channels
  EXPECT_EQ(lg[i][0], 30);
  EXPECT_EQ(lg[i][1], 43);
  EXPECT_EQ(lg[i++][2], 42);

  EXPECT_EQ(lg[i][0], 32);
  EXPECT_EQ(lg[i][1], 44);
  EXPECT_EQ(lg[i++][2], 43);

  EXPECT_EQ(lg[i][0], 33);
  EXPECT_EQ(lg[i][1], 46);
  EXPECT_EQ(lg[i++][2], 45);

  EXPECT_EQ(lg[i][0], 34);
  EXPECT_EQ(lg[i][1], 47);
  EXPECT_EQ(lg[i++][2], 46);

  EXPECT_EQ(lg[i][0], 35);
  EXPECT_EQ(lg[i][1], 49);
  EXPECT_EQ(lg[i++][2], 48);

  EXPECT_EQ(lg[i][0], 36);
  EXPECT_EQ(lg[i][1], 50);
  EXPECT_EQ(lg[i++][2], 49);

  EXPECT_EQ(lg[i][0], 37);
  EXPECT_EQ(lg[i][1], 52);
  EXPECT_EQ(lg[i++][2], 51);

  EXPECT_EQ(lg[i][0], 38);
  EXPECT_EQ(lg[i][1], 53);
  EXPECT_EQ(lg[i++][2], 52);

  EXPECT_EQ(lg[i][0], 39);
  EXPECT_EQ(lg[i][1], 55);
  EXPECT_EQ(lg[i++][2], 54);

  EXPECT_EQ(lg[i][0], 40);
  EXPECT_EQ(lg[i][1], 56);
  EXPECT_EQ(lg[i++][2], 55);

  EXPECT_EQ(lg[i][0], 41);
  EXPECT_EQ(lg[i][1], 58);
  EXPECT_EQ(lg[i++][2], 57);

  EXPECT_EQ(lg[i][0], 31);
  EXPECT_EQ(lg[i][1], 59);
  EXPECT_EQ(lg[i++][2], 58);

  // corner channels
  EXPECT_EQ(lg[i][0], 59);
  EXPECT_EQ(lg[i++][1], 42);

  EXPECT_EQ(lg[i][0], 44);
  EXPECT_EQ(lg[i++][1], 45);

  EXPECT_EQ(lg[i][0], 47);
  EXPECT_EQ(lg[i++][1], 48);

  EXPECT_EQ(lg[i][0], 50);
  EXPECT_EQ(lg[i++][1], 51);

  EXPECT_EQ(lg[i][0], 53);
  EXPECT_EQ(lg[i++][1], 54);

  EXPECT_EQ(lg[i][0], 56);
  EXPECT_EQ(lg[i++][1], 57);
}

TEST_F(HexagonalLatticeTest, line_distance)
{
  HexagonalLatticeUtility hl(4.0, 0.8, 0.6, 0.05, 50.0, 3, 2);

  // horizontal line
  Point p1(4.0, 5.0, 0.0);
  Point l1(1.0, 3.0, 0.0);
  Point l2(5.0, 3.0, 0.0);
  EXPECT_DOUBLE_EQ(hl.distanceFromLine(p1, l1, l2), 2.0);

  // vertical line
  Point l4(1.0, 5.0, 0.0);
  Point l3(1.0, 3.0, 0.0);
  Point p2(3.0, 4.0, 0.0);
  EXPECT_DOUBLE_EQ(hl.distanceFromLine(p2, l3, l4), 2.0);

  // angled line
  Point p3(2.0, 2.0, 0.0);
  Point l5(1.0, 2.0, 0.0);
  Point l6(2.0, 3.0, 0.0);
  EXPECT_DOUBLE_EQ(hl.distanceFromLine(p3, l5, l6), std::sqrt(2.0) / 2.0);
}

TEST_F(HexagonalLatticeTest, closest_gap)
{
  HexagonalLatticeUtility hl(4.0, 0.8, 0.6, 0.05, 50.0, 3, 2);

  Point pt1(0.23, 0.27, 0.0);
  EXPECT_EQ(hl.gapIndex(pt1), 0);

  Point pt2(-0.5, 0.29, 0.0);
  EXPECT_EQ(hl.gapIndex(pt2), 11);

  Point pt3(1.14, 0.275, 0.0);
  EXPECT_EQ(hl.gapIndex(pt3), 29);

  Point pt4(-0.77, 1.015, 0.0);
  EXPECT_EQ(hl.gapIndex(pt4), 13);

  Point pt5(-0.84, 0.445, 0.0);
  EXPECT_EQ(hl.gapIndex(pt5), 16);

  Point pt6(-0.47, 1.55, 0.0);
  EXPECT_EQ(hl.gapIndex(pt6), 32);

  Point pt7(-0.069, 1.94, 0.0);
  EXPECT_EQ(hl.gapIndex(pt7), 43);

  Point pt8(-1.22, 1.79, 0.0);
  EXPECT_EQ(hl.gapIndex(pt8), 45);

  Point pt9(-1.41, 1.17, 0.0);
  EXPECT_EQ(hl.gapIndex(pt9), 46);

  Point pt10(-0.26, -1.61, 0.0);
  EXPECT_EQ(hl.gapIndex(pt10), 37);

  Point pt11(2.23, 0.03, 0.0);
  EXPECT_EQ(hl.gapIndex(pt11), 57);

  Point pt12(-1.77, -0.70, 0.0);
  EXPECT_EQ(hl.gapIndex(pt12), 49);
}
