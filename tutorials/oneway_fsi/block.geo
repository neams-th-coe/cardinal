// mesh refinement params

Nx = 48; // uniform in x direction
Ny = 64; ry = 1.2; // pts in y, growth rate/bias
Nz = 64; rz = 1.2;

//================================================================
//================================================================

// based on benchmark specs

// points, constants
Lx = 0.1; // length along x

Point(1) = {0.4,0.0, 0.0};
Point(2) = {0.4,0.0,-0.2};
Point(3) = {0.4,0.2,-0.2};
Point(4) = {0.4,0.2, 0.0};

// curves

Line(1) = {1,2};
Line(2) = {2,3};
Line(3) = {3,4};
Line(4) = {4,1};

// meshing curves

Transfinite Line {-2,4} = Ny Using Progression ry;
Transfinite Line {-1,3} = Nz Using Progression rz;

// creating surface

Line Loop(1) = {1,2,3,4}; Plane Surface(1) = {1};

Recombine Surface "*";
Transfinite Surface "*";

// extrusion
Extrude{Lx,0.0,0.0}
{
  Surface{1};
  Layers{Nx};
  Recombine;
}

// separate sidesets for each surface - optional for debugging
// Physical Surface("inwall") = {1};
// Physical Surface("outwall") = {26};
// Physical Surface("top") = {21};
// Physical Surface("zwall") = {17};

// entire fluid/solid interface in one sideset
Physical Surface("interface") = {1,17,21,26};
// the surfaces below must always be in separate sidesets
Physical Surface("bottom") = {13};
Physical Surface("sym") = {25};

Physical Volume("solid") = {1};
Recombine Volume "*";

Coherence;

