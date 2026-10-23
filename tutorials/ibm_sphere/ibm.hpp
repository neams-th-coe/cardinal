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

// Compute the distance between two points
dfloat edgeLength(const Point & a, const Point & b)
{
  const dfloat dx = b.x() - a.x();
  const dfloat dy = b.y() - a.y();
  const dfloat dz = b.z() - a.z();
  return std::sqrt(dx * dx + dy * dy + dz * dz);
}

// Function to compute the area of a triangle given its vertices (used to compute the area of the traingulated surface, STL surface, and weight of Lagrangian and integration points)
dfloat computeTriangleArea(const Point& v1, const Point& v2, const Point& v3)
{
  const dfloat ax = v2.x() - v1.x();
  const dfloat ay = v2.y() - v1.y();
  const dfloat az = v2.z() - v1.z();
  const dfloat bx = v3.x() - v1.x();
  const dfloat by = v3.y() - v1.y();
  const dfloat bz = v3.z() - v1.z();

  const dfloat crossX = ay * bz - az * by;
  const dfloat crossY = az * bx - ax * bz;
  const dfloat crossZ = ax * by - ay * bx;

  return 0.5 * std::sqrt(crossX * crossX + crossY * crossY + crossZ * crossZ);
}

// Function to compute the total surface area of the STL file
dfloat computeSTLSurfaceArea(const std::vector<Vertex>& vertices)
{
  dfloat totalArea = 0.0;

  // Iterate over the triangles in the STL file
  for (size_t i = 0; i < vertices.size(); i += 3)
  {
    // Extract the three vertices of the triangle
    const Vertex& v1 = vertices[i];
    const Vertex& v2 = vertices[i + 1];
    const Vertex& v3 = vertices[i + 2];

    // Compute the area of the triangle
    dfloat area = computeTriangleArea(Point(v1.x, v1.y, v1.z),
                                      Point(v2.x, v2.y, v2.z),
                                      Point(v3.x, v3.y, v3.z));

    // Accumulate the total area
    totalArea += area;
  }

  return totalArea;
}

// Integrate a function over the STL surface; shown using an example function.
// Pass in globalIntegrationPoints, then no global communication is needed
dfloat integrate_function_over_STL(std::vector<IntegrationPoint> & integration_points)
{
  // Surface quadrature diagnostic on this rank
  dfloat integratedValue = 0.0;

  for (const auto& ip : integration_points)
  {
    // Calculate the function value at the integration point; arbitrary function
    dfloat fValue = (std::pow(ip.position.x(), 2) + std::pow(ip.position.y(), 2) + std::pow(ip.position.z(), 2)) *  std::exp(-2 * (std::pow(ip.position.x(), 2) + std::pow(ip.position.y(), 2) + std::pow(ip.position.z(), 2)));

    // Multiply by the weight of the integration point and accumulate
    integratedValue += fValue * ip.weight;
  }

  return integratedValue;
}

bool try_parse_float(const std::string& token, dfloat& value)
{
  try
  {
    size_t pos;
    value = std::stof(token, &pos);
    return pos == token.size();
  } catch (...) {
      return false;
  }
}

// Parse ASCII STL vertices from a file, and store the vertices and face normals
bool parse_stl(const std::string& filename, std::vector<Vertex>& vertices, std::vector<FacetNormal>& normals)
{
  std::ifstream file(filename);
  if (!file.is_open())
  {
    std::cerr << "Error opening file: " << filename << std::endl;
    return false;
  }

  std::string line;
  std::string ignore;

  while (std::getline(file, line))
  {
    std::istringstream iss(line);
    iss >> ignore;

    if (ignore == "vertex")
    {
      // Parse vertex coordinates
      Vertex vertex;
      for (int i = 0; i < 3; ++i)
      {
        std::string token;
        iss >> token;
        if (!try_parse_float(token, i == 0 ? vertex.x : (i == 1 ? vertex.y : vertex.z)))
        {
          std::cerr << "Error parsing vertex coordinates." << std::endl;
          return false;
        }
      }
      vertices.push_back(vertex);
    }
    else if (ignore == "facet")
    {
      // Parse facet normal
      FacetNormal normal;
      iss >> ignore >> normal.nx >> normal.ny >> normal.nz;
      normals.push_back(normal);
    }
  }

  if (vertices.size() % 3 != 0)
  {
    std::cerr << "STL does not contain a valid set of coordinates!" << std::endl;
    return false;
  }

  return true;
}

std::vector<std::array<dfloat, 3>> convertPointsToArrays(const std::vector<Point>& points)
{
  std::vector<std::array<dfloat, 3>> arrays;
  for (const auto& point : points)
    arrays.push_back({point.x(), point.y(), point.z()});
  return arrays;
}
