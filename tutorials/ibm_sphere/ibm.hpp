void read_stl_file();

struct Vertex {
    dfloat x, y, z;
};

struct FacetNormal {
    dfloat nx, ny, nz;
};

struct Point {
    dfloat xValue = 0.0;
    dfloat yValue = 0.0;
    dfloat zValue = 0.0;

    Point() = default;
    Point(dfloat x, dfloat y, dfloat z) : xValue(x), yValue(y), zValue(z) {}

    dfloat x() const { return xValue; }
    dfloat y() const { return yValue; }
    dfloat z() const { return zValue; }
};

// A structure to hold an integration point and its weight
struct IntegrationPoint {
    Point position;
    dfloat weight;
    std::array<dfloat, 3> normal;
};

// A structure to hold a Lagrange point, its weight, the characteristicArea, characteristicLength, the element number it cuts
struct LagrangePoint {
    Point position;
    dfloat weight;
    dfloat characteristicArea;
    dfloat characteristicLength;
    int elementNumber;
    std::array<dfloat, 3> normal;
};

// Define a structure to hold an iso-surface mesh (points and triangle connectivity)
struct IsoSurfaceMesh {
  std::vector<std::array<dfloat, 3>> points;
  std::vector<std::array<std::size_t, 3>> faces;
};
