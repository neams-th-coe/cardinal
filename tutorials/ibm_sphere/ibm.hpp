void read_stl_file();

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
