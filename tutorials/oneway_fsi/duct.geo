// mesh refinement params
Geometry.CopyMeshingMethod = 1;

Ny =  10; ry = 1.15; // pts in y, growth rate/bias
Nz =  10; rz = 1.15;

Nx1 = 14; // x layers near inlet
Nx2 = 10; // x layers near obstacle
Nx3 = 32; // x layers beyond obstacle

//================================================================
//================================================================

// based on benchmark specs

// points, constants
Lx = 0.1; // length along x

Point(1) = {0.0,0.0, 0.0};
Point(2) = {0.0,0.0,-0.2};
Point(3) = {0.0,0.2,-0.2};
Point(4) = {0.0,0.2, 0.0};

// curves

Line(1) = {1,2};
Line(2) = {2,3};
Line(3) = {3,4};
Line(4) = {4,1};


// creating surface

Line Loop(1) = {1,2,3,4}; Plane Surface(1) = {1};

Recombine Surface "*";
Transfinite Surface "*";

bottom_left[] = Translate {0., 0., -0.2} { Duplicata{ Surface{1}; } };

// meshing curves

Transfinite Line {-2,4,-7} = Ny Using Progression ry;
Transfinite Line {-1,3,6,-8} = Nz Using Progression rz;

top_surfs[] = Rotate {{1,0,0}, {0,0.2,-0.2}, Pi} { Duplicata{ Surface{1,5};} };


// Extrude first block
Extrude {0.4,0,0} // TODO: parameterise
{
  Surface{1,bottom_left[],top_surfs[]};
  Layers{ {Nx1,Nx2}, {0.75,1} };
  Recombine;
}

// Extrude 3 blocks over the solid block
Extrude {0.1,0,0}// TODO: parameterise
{
  Surface{60,82,104};
  Layers{Nx2};
  Recombine;
}

// Copy the surface from the front to the back of the block
block_back[] = Translate {0.1, 0., 0.} { Duplicata{ Surface{38}; } };

// Extrude 4 blocks over the solid block
Extrude {1.0,0,0}// TODO: parameterise
{
  Surface{126,148,170,171};
  Layers{ {Nx2,Nx3}, {0.1,1} };
  Recombine;
}

Coherence;

Physical Surface("inlet",1) = {1,5,9,14};
Physical Surface("outlet",2) = {241,219,263,197};

Physical Surface("mv",3) = {38,165,125,171};

Physical Surface("wall",4) = {91,69,157,135,228,206,51,81,117,147,188,218,47,25,113,184,250};
Physical Surface("sym",5) = {95,37,161,232,262};

Physical Volume("fluid") = {1:11};

Recombine Volume "*";
