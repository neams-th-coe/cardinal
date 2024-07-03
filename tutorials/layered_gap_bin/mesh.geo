Geometry.CopyMeshingMethod = 1;
Coherence;
Coherence Mesh;

meshDim=3;
padHeight = 0.1016;
ductHeight = 5*padHeight; //full height = 4.7752
flatToFlat = 0.1571;
pitch = 0.16142;
IAGap = pitch-flatToFlat;
ductRadius = (0.5*flatToFlat)*2/Sqrt(3);
pad_thickness = 1.5e-3;
pad_gap_thickness = (IAGap - 2.0*pad_thickness);
boundary_layer_thickness = 0.0001;

Narms  = 6; // number of nodes along each half arm
Nbl    = 2; // no. of points in boundary layers
Ngap   = 2; // no. of points across half the load pad gap
Nouter = 2; // no. of points in the outer region i.e. slices north of the load pad in 2d slice
Ninner = 2;
Dh = 8.7587931254065887E-003;

Lz1 = 10*Dh;
Lz2 = padHeight;
Lz3 = 5*padHeight;

//Nz11 = 4; Nz12 = 3; lz11 = 0.60; lz12 = 1.0;Nz2 =  8; lz21 = 1;
//Nz31 = 8; Nz32 = 20; lz31 = 0.2; lz32 = 1.0;

Nz11 = 2; Nz12 = 2; lz11 = 0.60; lz12 = 1.0; Nz2 =  6; lz21 = 1;
Nz31 = 4; Nz32 = 10; lz31 = 0.2; lz32 = 1.0;

trans_layer_growth = 1.3;

displ = 0.0*pad_gap_thickness;

//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////

blt = boundary_layer_thickness;
blt_lp = blt;

If (pad_gap_thickness - displ < 4* blt)   // i.e. we cannot allocate 2 BLs of thickness BL in thinnest part of load pad
  blt_lp = 0.15*(pad_gap_thickness - displ);
EndIf

theta1 = Pi/3.0; theta2 = Pi/6.0;
r1  = 0.5 * flatToFlat;

// creating innermost duct - RHS vertex (angle = 0)
Point(1) = { ductRadius * Cos(0*theta1), ductRadius * Sin(0*theta1), 0};
Point(2) = { r1 * Cos(1.0 *theta2) , r1 * Sin(1.0 *theta2) , 0};
Point(3) = { r1 * Cos(-1.0*theta2) , r1 * Sin(-1.0*theta2) , 0};

r2 = r1 + IAGap;
dr2 = ductRadius + IAGap;

// outer duct curves
Line(1) = {1,2}; Line(2) = {1,3};
Translate {IAGap*Cos(theta2) ,  IAGap*Sin(theta2) , 0} {  Duplicata { Line{1}; } }
Translate {IAGap*Cos(-theta2) ,  IAGap*Sin(-theta2) , 0} {  Duplicata { Line{2}; } }

xyz[] = Point{4};
Rotate {{0, 0, 1}, {xyz[0],xyz[1],xyz[2]},-2*theta1} {  Duplicata{ Curve{3};}}
xyz[] = Point{6};
Rotate {{0, 0, 1}, {xyz[0],xyz[1],xyz[2]}, 2*theta1} {  Duplicata{ Curve{4};}}

// load pad inner wall
r1 = ( (0.5 * flatToFlat) + pad_thickness)/Sin(theta1);
r2 = (0.5*flatToFlat) + pad_thickness;
p1 = newp; Point(p1) = { r1 * Cos(0*theta1) , r1 * Sin(0*theta1) , 0};
p2 = newp; Point(p2) = { r2 * Cos( 1*theta2), r2 * Sin( 1*theta2), 0};
p3 = newp; Point(p3) = { r2 * Cos(-1*theta2), r2 * Sin(-1*theta2), 0};

l1 = newl; Line(l1) = {p2,p1};
l2 = newl; Line(l2) = {p1,p3};

// load pad outer wall
r1 = pad_thickness; // ductRadius + pad_thickness + pad_gap_thickness;
r2 = (0.5*flatToFlat) + pad_thickness + pad_gap_thickness;
r3 = (( (0.5 * flatToFlat) + pad_thickness)/Sin(theta1)) - ductRadius;
p4 = Translate {-r3*Cos(theta1),-r3*Sin(theta1), 0} { Duplicata { Point{4}; }}; //mid north
p5 = newp; Point(p5) = { r2 * Cos( 1*theta2), r2 * Sin( 1*theta2), 0}; // extreme north
p6 = newp; Point(p6) = { r2 * Cos(-1*theta2), r2 * Sin(-1*theta2), 0};  // extreme south
p7 = Translate {-r3*Cos(theta1), r3*Sin(theta1), 0} { Duplicata { Point{6}; }};// mid south
p8 = Translate {0,-r1, 0} { Duplicata { Point{9}; }}; // arm north
p9 = Translate {0, r1, 0} { Duplicata { Point{11}; }}; // arm south

l3 = newl; Line(l3) = {p5,p4};
l4 = newl; Line(l4) = {p4,p8};
l5 = newl; Line(l5) = {p9,p7};
l6 = newl; Line(l6) = {p7,p6};

// rotate to create the entire 2D cross section at once
// note that after displacement of duct, load pad in +y direction, there is no longer symmetry
// so we are reflecting first, displacing later
Rotate {{0, 0, 1}, {0, 0, 0}, 1*theta1} {
  Duplicata { Curve{5}; Curve{6}; Curve{4}; Curve{8}; Curve{12}; Curve{2}; Curve{7}; Curve{9}; Curve{3}; Curve{1}; Curve{11}; Curve{10}; }}
Rotate {{0, 0, 1}, {0, 0, 0}, 2*theta1} {
  Duplicata { Curve{5}; Curve{6}; Curve{4}; Curve{8}; Curve{12}; Curve{2}; Curve{7}; Curve{9}; Curve{3}; Curve{1}; Curve{11}; Curve{10}; }}
Rotate {{0, 0, 1}, {0, 0, 0}, 3*theta1} {
  Duplicata { Curve{5}; Curve{6}; Curve{4}; Curve{8}; Curve{12}; Curve{2}; Curve{7}; Curve{9}; Curve{3}; Curve{1}; Curve{11}; Curve{10}; }}
Rotate {{0, 0, 1}, {0, 0, 0}, 4*theta1} {
  Duplicata { Curve{5}; Curve{6}; Curve{4}; Curve{8}; Curve{12}; Curve{2}; Curve{7}; Curve{9}; Curve{3}; Curve{1}; Curve{11}; Curve{10}; }}
Rotate {{0, 0, 1}, {0, 0, 0}, 5*theta1} {
  Duplicata { Curve{5}; Curve{6}; Curve{4}; Curve{8}; Curve{12}; Curve{2}; Curve{7}; Curve{9}; Curve{3}; Curve{1}; Curve{11}; Curve{10}; }}

// the goal is to study the variation in the pressure drop with respect to height and the azimuthal direction
// to ultimately develop a momentum source based on hu and fanning 13 nuc eng and design
// that can be used to model the impact of duct movement with structural components without directly meshing
// the load pads in an fsi simulation, which can create mesh quality issues when approaching contact.
Translate {0, displ, 0} {
  Curve{1}; Point{2}; Curve{18}; Point{41}; Curve{22}; Point{58}; Curve{30}; Curve{34}; Point{87}; Point{104}; Curve{42}; Point{133}; Curve{46}; Point{150}; Curve{54}; Point{179}; Curve{58}; Point{196}; Curve{66}; Curve{70}; Point{225}; Point{3}; Curve{2}; Point{1}; Curve{16}; Curve{19}; Point{33}; Curve{28}; Point{45}; Point{79}; Curve{31}; Point{91}; Curve{40}; Point{125}; Curve{43}; Curve{52}; Point{137}; Point{171}; Curve{55}; Point{183}; Curve{64}; Point{217}; Curve{67}; Point{14}; Curve{8}; Point{12}; Curve{7}; Point{13};
}

xyz1[] = Point{12}; xyz2[] = Point{15}; xyz3[] = Point{18};
midpt1 = newp; Point(midpt1) = { (xyz1[0] + xyz2[0] + xyz3[0])/3.0, (xyz1[1] + xyz2[1] + xyz3[1])/3.0, 0.0};

xyz1[] = Point{50}; xyz2[] = Point{33}; xyz3[] = Point{37};
midpt2 = newp; Point(midpt2) = { (xyz1[0] + xyz2[0] + xyz3[0])/3.0, (xyz1[1] + xyz2[1] + xyz3[1])/3.0, 0.0};

xyz1[] = Point{96}; xyz2[] = Point{83}; xyz3[] = Point{79};
midpt3 = newp; Point(midpt3) = { (xyz1[0] + xyz2[0] + xyz3[0])/3.0, (xyz1[1] + xyz2[1] + xyz3[1])/3.0, 0.0};

xyz1[] = Point{129}; xyz2[] = Point{125}; xyz3[] = Point{142};
midpt4 = newp; Point(midpt4) = { (xyz1[0] + xyz2[0] + xyz3[0])/3.0, (xyz1[1] + xyz2[1] + xyz3[1])/3.0, 0.0};

xyz1[] = Point{175}; xyz2[] = Point{171}; xyz3[] = Point{188};
midpt5 = newp; Point(midpt5) = { (xyz1[0] + xyz2[0] + xyz3[0])/3.0, (xyz1[1] + xyz2[1] + xyz3[1])/3.0, 0.0};

xyz1[] = Point{217}; xyz2[] = Point{221}; xyz3[] = Point{234};
midpt6 = newp; Point(midpt6) = { (xyz1[0] + xyz2[0] + xyz3[0])/3.0, (xyz1[1] + xyz2[1] + xyz3[1])/3.0, 0.0};

// create load pad boundary layers

// E junction
Translate { blt*Cos(0)     , -blt*Sin(0)     , 0} {  Duplicata { Point{12}; }}
Translate {-blt*Cos(theta1),  blt*Sin(theta1), 0} {  Duplicata { Point{18}; }}
Translate {-blt*Cos(theta1), -blt*Sin(theta1), 0} {  Duplicata { Point{15}; }}

// E arm
Translate { 0, -blt, 0} {  Duplicata { Point{19}; }}
Translate { 0,  blt, 0} {  Duplicata { Point{20}; }}

// NE arm
Translate { blt*Cos(theta2),  blt*Sin(theta2), 0} {  Duplicata { Point{13}; }}
Translate {-blt*Cos(theta2), -blt*Sin(theta2), 0} {  Duplicata { Point{16}; }}

//SE mid arm
Translate { blt*Cos(theta2), -blt*Sin(theta2), 0} {  Duplicata { Point{14}; }}
Translate {-blt*Cos(theta2),  blt*Sin(theta2), 0} {  Duplicata { Point{17}; }}

// SE junction
Translate { blt*Cos(theta1), -blt*Sin(theta1), 0} {  Duplicata { Point{217}; }}
Translate {-blt*Cos(     0),  blt*Sin(     0), 0} {  Duplicata { Point{234}; }}
Translate { blt*Cos(theta1),  blt*Sin(theta1), 0} {  Duplicata { Point{221}; }}

// SE arm
Translate {-blt*Cos(theta2), -blt*Sin(theta2), 0} {  Duplicata { Point{250}; }}
Translate { blt*Cos(theta2),  blt*Sin(theta2), 0} {  Duplicata { Point{245}; }}

//S mid-arm
Translate { 0, -blt, 0} {  Duplicata { Point{183}; }}
Translate { 0,  blt, 0} {  Duplicata { Point{187}; }}

// SW junction
Translate {-blt*Cos(theta1), -blt*Sin(theta1), 0} {  Duplicata { Point{171}; }}
Translate { blt*Cos(     0),  blt*Sin(     0), 0} {  Duplicata { Point{175}; }}
Translate {-blt*Cos(theta1),  blt*Sin(theta1), 0} {  Duplicata { Point{188}; }}

// SW arms
Translate { blt*Cos(theta2), -blt*Sin(theta2), 0} {  Duplicata { Point{199}; }}
Translate {-blt*Cos(theta2),  blt*Sin(theta2), 0} {  Duplicata { Point{204}; }}

// SW mid-arm
Translate { blt*Cos(theta2),  blt*Sin(theta2), 0} {  Duplicata { Point{141}; }}
Translate {-blt*Cos(theta2), -blt*Sin(theta2), 0} {  Duplicata { Point{137}; }}

// W sector
Translate {-blt            ,  0              , 0} {  Duplicata { Point{125}; }}
Translate { blt*Cos(theta1), -blt*Sin(theta1), 0} {  Duplicata { Point{129}; }}
Translate { blt*Cos(theta1),  blt*Sin(theta1), 0} {  Duplicata { Point{142}; }}

// W arm
Translate { 0, -blt, 0} {  Duplicata { Point{153}; }}
Translate { 0,  blt, 0} {  Duplicata { Point{158}; }}

// NW mid-arm
Translate { blt*Cos(theta2), -blt*Sin(theta2), 0} {  Duplicata { Point{95}; }}
Translate {-blt*Cos(theta2),  blt*Sin(theta2), 0} {  Duplicata { Point{91}; }}

// NW junction
Translate {-blt_lp*Cos(theta1), blt_lp*Sin(theta1), 0} {  Duplicata { Point{79}; }}
Translate {-blt_lp*Cos(theta1),-blt_lp*Sin(theta1), 0} {  Duplicata { Point{83}; }}
Translate { blt, 0, 0} {  Duplicata { Point{96}; }}

// NW arm
Translate {-blt*Cos(theta2), -blt*Sin(theta2), 0} {  Duplicata { Point{107}; }}
Translate { blt*Cos(theta2),  blt*Sin(theta2), 0} {  Duplicata { Point{112}; }}

// N mid-arm
Translate { 0, -blt_lp, 0} {  Duplicata { Point{49}; }}
Translate { 0,  blt_lp, 0} {  Duplicata { Point{45}; }}

// NE junction
Translate { blt_lp*Cos(theta1),-blt_lp*Sin(theta1), 0} {  Duplicata { Point{50}; }}
Translate { blt_lp*Cos(theta1), blt_lp*Sin(theta1), 0} {  Duplicata { Point{33}; }}
Translate {-blt_lp, 0.0, 0} {  Duplicata { Point{37}; }}

// NE arm
Translate { blt*Cos(theta2),-blt*Sin(theta2), 0} {  Duplicata { Point{66}; }}
Translate {-blt*Cos(theta2), blt*Sin(theta2), 0} {  Duplicata { Point{61}; }}

//mid points (starting E, going counter-clockwise)
xyz1[] = Point{19}; xyz2[] = Point{20};
mplp1 = newp; Point(mplp1) = { 0.5*(xyz1[0]+xyz2[0]), 0.5*(xyz1[1]+xyz2[1]), 0 };

xyz1[] = Point{262}; xyz2[] = Point{263};
mplp2 = newp; Point(mplp2) = { 0.5*(xyz1[0]+xyz2[0]), 0.5*(xyz1[1]+xyz2[1]), 0 };

xyz1[] = Point{66}; xyz2[] = Point{61};
mplp3 = newp; Point(mplp3) = { 0.5*(xyz1[0]+xyz2[0]), 0.5*(xyz1[1]+xyz2[1]), 0 };

// switching to thinnest boundary layer at the North
xyz1[] = Point{292}; xyz2[] = Point{293}; xyz3[] = Point{296};
mplp4 = newp; Point(mplp4) = { 0.5*(xyz1[0]+xyz2[0]), 0.5*(xyz1[1]+xyz2[1]), 0 };

// correction for NE,NW midpoint
xyz1[] = Point{252}; xyz2[] = Point{302}; dy = xyz2[1] - xyz1[1];
Translate { 0, dy, 0} { Point{252}; }

xyz1[] = Point{253}; xyz2[] = Point{302}; dy = xyz2[1] - xyz1[1];
Translate { 0, dy, 0} { Point{253}; }

//////

xyz1[] = Point{291}; xyz2[] = Point{290};
mplp4 = newp; Point(mplp4) = { 0.5*(xyz1[0]+xyz2[0]), 0.5*(xyz1[1]+xyz2[1]), 0 };

xyz1[] = Point{285}; xyz2[] = Point{286};
mplp5 = newp; Point(mplp5) = { 0.5*(xyz1[0]+xyz2[0]), 0.5*(xyz1[1]+xyz2[1]), 0 };

xyz1[] = Point{283}; xyz2[] = Point{284};
mplp6 = newp; Point(mplp6) = { 0.5*(xyz1[0]+xyz2[0]), 0.5*(xyz1[1]+xyz2[1]), 0 };

xyz1[] = Point{279}; xyz2[] = Point{278};
mplp7 = newp; Point(mplp7) = { 0.5*(xyz1[0]+xyz2[0]), 0.5*(xyz1[1]+xyz2[1]), 0 };

xyz1[] = Point{276}; xyz2[] = Point{277};
mplp8 = newp; Point(mplp8) = { 0.5*(xyz1[0]+xyz2[0]), 0.5*(xyz1[1]+xyz2[1]), 0 };

xyz1[] = Point{271}; xyz2[] = Point{272};
mplp9 = newp; Point(mplp9) = { 0.5*(xyz1[0]+xyz2[0]), 0.5*(xyz1[1]+xyz2[1]), 0 };

xyz1[] = Point{270}; xyz2[] = Point{269};
mplp10 = newp; Point(mplp10) = { 0.5*(xyz1[0]+xyz2[0]), 0.5*(xyz1[1]+xyz2[1]), 0 };

xyz1[] = Point{264}; xyz2[] = Point{265};
mplp11 = newp; Point(mplp11) = { 0.5*(xyz1[0]+xyz2[0]), 0.5*(xyz1[1]+xyz2[1]), 0 };

// boundary layers outside load pad, starting on the E arms, going counter-clockwise

tlg = trans_layer_growth;
//E arm
Translate { 0,  blt    , 0} {  Duplicata { Point{11}; }}
Translate { 0, -tlg*blt, 0} {  Duplicata { Point{20}; }}
Translate { 0,  tlg*blt, 0} {  Duplicata { Point{19}; }}
Translate { 0, -blt    , 0} {  Duplicata { Point{ 9}; }}

// E junction
Translate { -blt*Cos(theta1), -blt*Sin(theta1), 0} {  Duplicata { Point{4}; }}
Translate { -blt*Cos(theta1),  blt*Sin(theta1), 0} {  Duplicata { Point{6}; }}
Translate {  blt    , 0, 0} {  Duplicata { Point{1}; }}

Translate { -tlg*blt, 0, 0} {  Duplicata { Point{12}; }}
Translate {  tlg*blt*Cos(theta1),  tlg*blt*Sin(theta1), 0} {  Duplicata { Point{15}; }}
Translate {  tlg*blt*Cos(theta1), -tlg*blt*Sin(theta1), 0} {  Duplicata { Point{18}; }}

//NE arm mid-arm
Translate {  blt*Cos(theta2),  blt*Sin(theta2), 0} {  Duplicata { Point{2}; }}
Translate { -blt*Cos(theta2), -blt*Sin(theta2), 0} {  Duplicata { Point{5}; }}

Translate {  tlg*blt*Cos(theta2),  tlg*blt*Sin(theta2), 0} {  Duplicata { Point{16}; }}
Translate { -tlg*blt*Cos(theta2), -tlg*blt*Sin(theta2), 0} {  Duplicata { Point{13}; }}

//NE junction
Translate {  blt*Cos(theta1),  blt*Sin(theta1), 0} {  Duplicata { Point{41}; }}
Translate {  blt*Cos(theta1), -blt*Sin(theta1), 0} {  Duplicata { Point{21}; }}
Translate { -blt, 0, 0} {  Duplicata { Point{25}; }}

Translate { -tlg*blt*Cos(theta1), -tlg*blt*Sin(theta1), 0} {  Duplicata { Point{33}; }}
Translate { -tlg*blt*Cos(theta1),  tlg*blt*Sin(theta1), 0} {  Duplicata { Point{50}; }}
Translate {  tlg*blt,  0, 0} {  Duplicata { Point{37}; }}

// NE arm
Translate {  blt*Cos(theta2), -blt*Sin(theta2), 0} {  Duplicata { Point{22}; }}
Translate { -blt*Cos(theta2),  blt*Sin(theta2), 0} {  Duplicata { Point{26}; }}

Translate {  -tlg*blt*Cos(theta2), tlg*blt*Sin(theta2), 0} {  Duplicata { Point{66}; }}
Translate {   tlg*blt*Cos(theta2),-tlg*blt*Sin(theta2), 0} {  Duplicata { Point{61}; }}

// N mid arm
Translate { 0, -blt, 0} {  Duplicata { Point{54}; }}
Translate { 0,  blt, 0} {  Duplicata { Point{58}; }}

Translate { 0,  tlg*blt, 0} {  Duplicata { Point{49}; }}
Translate { 0, -tlg*blt, 0} {  Duplicata { Point{45}; }}

//NW junction
Translate { -blt*Cos(theta1), -blt*Sin(theta1), 0} {  Duplicata { Point{71}; }}
Translate { -blt*Cos(theta1),  blt*Sin(theta1), 0} {  Duplicata { Point{87}; }}
Translate {  blt, 0, 0} {  Duplicata { Point{67}; }}

Translate {  tlg*blt*Cos(theta1),  tlg*blt*Sin(theta1), 0} {  Duplicata { Point{83}; }}
Translate {  tlg*blt*Cos(theta1), -tlg*blt*Sin(theta1), 0} {  Duplicata { Point{79}; }}
Translate { -tlg*blt,  0, 0} {  Duplicata { Point{96}; }}

// NW corner
Translate {  blt*Cos(theta2),  blt*Sin(theta2), 0} {  Duplicata { Point{68}; }}
Translate { -blt*Cos(theta2), -blt*Sin(theta2), 0} {  Duplicata { Point{72}; }}

Translate {  tlg*blt*Cos(theta2),  tlg*blt*Sin(theta2), 0} {  Duplicata { Point{107}; }}
Translate { -tlg*blt*Cos(theta2), -tlg*blt*Sin(theta2), 0} {  Duplicata { Point{112}; }}

//NW midsection
Translate {  blt*Cos(theta2), -blt*Sin(theta2), 0} {  Duplicata { Point{100}; }}
Translate { -blt*Cos(theta2),  blt*Sin(theta2), 0} {  Duplicata { Point{104}; }}

Translate {  tlg*blt*Cos(theta2), -tlg*blt*Sin(theta2), 0} {  Duplicata { Point{91}; }}
Translate { -tlg*blt*Cos(theta2),  tlg*blt*Sin(theta2), 0} {  Duplicata { Point{95}; }}

//W junction
Translate {  blt*Cos(theta1), -blt*Sin(theta1), 0} {  Duplicata { Point{117}; }}
Translate {  blt*Cos(theta1),  blt*Sin(theta1), 0} {  Duplicata { Point{113}; }}
Translate { -blt, 0, 0} {  Duplicata { Point{133}; }}

Translate { -tlg*blt*Cos(theta1),  tlg*blt*Sin(theta1), 0} {  Duplicata { Point{129}; }}
Translate { -tlg*blt*Cos(theta1), -tlg*blt*Sin(theta1), 0} {  Duplicata { Point{142}; }}
Translate { tlg*blt, 0, 0} {  Duplicata { Point{125}; }}

// W arm
Translate { 0, -blt    , 0} {  Duplicata { Point{118}; }}
Translate { 0,  blt    , 0} {  Duplicata { Point{114}; }}
Translate { 0,  tlg*blt, 0} {  Duplicata { Point{153}; }}
Translate { 0, -tlg*blt, 0} {  Duplicata { Point{158}; }}

// SW mid arm
Translate {  blt*Cos(theta2),  blt*Sin(theta2), 0} {  Duplicata { Point{146}; }}
Translate { -blt*Cos(theta2), -blt*Sin(theta2), 0} {  Duplicata { Point{150}; }}

Translate {  tlg*blt*Cos(theta2),  tlg*blt*Sin(theta2), 0} {  Duplicata { Point{137}; }}
Translate { -tlg*blt*Cos(theta2), -tlg*blt*Sin(theta2), 0} {  Duplicata { Point{141}; }}

// SW junction
Translate { -blt*Cos(theta1),  blt*Sin(theta1), 0} {  Duplicata { Point{159}; }}
Translate { -blt*Cos(theta1), -blt*Sin(theta1), 0} {  Duplicata { Point{179}; }}
Translate {  blt, 0, 0} {  Duplicata { Point{163}; }}

Translate {  tlg*blt*Cos(theta1),  tlg*blt*Sin(theta1), 0} {  Duplicata { Point{171}; }}
Translate {  tlg*blt*Cos(theta1), -tlg*blt*Sin(theta1), 0} {  Duplicata { Point{188}; }}
Translate { -tlg*blt,  0, 0} {  Duplicata { Point{175}; }}

// SW arm
Translate { -blt*Cos(theta2),  blt*Sin(theta2), 0} {  Duplicata { Point{160}; }}
Translate {  blt*Cos(theta2), -blt*Sin(theta2), 0} {  Duplicata { Point{164}; }}

Translate {  -tlg*blt*Cos(theta2), tlg*blt*Sin(theta2), 0} {  Duplicata { Point{199}; }}
Translate {   tlg*blt*Cos(theta2),-tlg*blt*Sin(theta2), 0} {  Duplicata { Point{204}; }}

// S mid-arm
Translate { 0, -blt, 0} {  Duplicata { Point{196}; }}
Translate { 0,  blt, 0} {  Duplicata { Point{192}; }}

Translate { 0,  blt, 0} {  Duplicata { Point{183}; }}
Translate { 0, -blt, 0} {  Duplicata { Point{187}; }}

// SE junction
Translate {  blt*Cos(theta1), -blt*Sin(theta1), 0} {  Duplicata { Point{225}; }}
Translate {  blt*Cos(theta1),  blt*Sin(theta1), 0} {  Duplicata { Point{209}; }}
Translate { -blt, 0, 0} {  Duplicata { Point{205}; }}

Translate { -tlg*blt*Cos(theta1),  tlg*blt*Sin(theta1), 0} {  Duplicata { Point{217}; }}
Translate { -tlg*blt*Cos(theta1), -tlg*blt*Sin(theta1), 0} {  Duplicata { Point{221}; }}
Translate {  tlg*blt,  0, 0} {  Duplicata { Point{234}; }}

// SE arm
Translate {  blt*Cos(theta2),  blt*Sin(theta2), 0} {  Duplicata { Point{210}; }}
Translate { -blt*Cos(theta2), -blt*Sin(theta2), 0} {  Duplicata { Point{206}; }}

Translate {  -tlg*blt*Cos(theta2),-tlg*blt*Sin(theta2), 0} {  Duplicata { Point{245}; }}
Translate {   tlg*blt*Cos(theta2), tlg*blt*Sin(theta2), 0} {  Duplicata { Point{250}; }}

// SE mid arm
Translate { -blt*Cos(theta2),  blt*Sin(theta2), 0} {  Duplicata { Point{7}; }}
Translate {  blt*Cos(theta2), -blt*Sin(theta2), 0} {  Duplicata { Point{3}; }}

Translate { -tlg*blt*Cos(theta2),  tlg*blt*Sin(theta2), 0} {  Duplicata { Point{14}; }}
Translate {  tlg*blt*Cos(theta2), -tlg*blt*Sin(theta2), 0} {  Duplicata { Point{17}; }}


// stitching together transverse curves

// Load pad boundary layers, starting with NE junction, going counter-clockwise
Line(73) = {298, 296};
Line(74) = {296, 37};
Line(75) = {61, 298};
Line(76) = {66, 297};
Line(77) = {297, 294};
Line(78) = {294, 50};
Line(79) = {33, 295};
Line(80) = {294, 252};
Line(81) = {295, 252};
Line(82) = {296, 252};
Line(83) = {252, 301};
Line(84) = {252, 302};
Line(85) = {294, 292};
Line(86) = {292, 49};
Line(87) = {293, 45};
Line(88) = {293, 295};
Line(89) = {293, 302};
Line(90) = {302, 292};
Line(91) = {293, 287};
Line(92) = {287, 79};
Line(93) = {83, 288};
Line(94) = {288, 292};
Line(95) = {302, 253};
Line(96) = {253, 288};
Line(97) = {253, 287};
Line(98) = {96, 289};
Line(99) = {289, 291};
Line(100) = {291, 112};
Line(101) = {290, 107};
Line(102) = {290, 288};
Line(103) = {253, 289};
Line(104) = {253, 303};
Line(105) = {287, 286};
Line(106) = {286, 91};
Line(107) = {95, 285};
Line(108) = {285, 289};
Line(109) = {253, 304};
Line(110) = {304, 285};
Line(111) = {304, 286};
Line(112) = {286, 280};
Line(113) = {280, 125};
Line(114) = {254, 280};
Line(115) = {254, 304};
Line(116) = {285, 281};
Line(117) = {281, 129};
Line(118) = {254, 281};
Line(119) = {282, 254};
Line(120) = {142, 282};
Line(121) = {282, 284};
Line(122) = {284, 158};
Line(123) = {153, 283};
Line(124) = {283, 281};
Line(125) = {254, 305};
Line(126) = {284, 305};
Line(127) = {305, 283};
Line(128) = {282, 278};
Line(129) = {278, 141};
Line(130) = {278, 306};
Line(131) = {306, 279};
Line(132) = {279, 137};
Line(133) = {306, 254};
Line(134) = {280, 279};
Line(135) = {273, 171};
Line(136) = {273, 279};
Line(137) = {306, 255};
Line(138) = {274, 175};
Line(139) = {274, 278};
Line(140) = {274, 255};
Line(141) = {255, 273};
Line(142) = {255, 275};
Line(143) = {275, 188};
Line(144) = {274, 276};
Line(145) = {276, 199};
Line(146) = {277, 204};
Line(147) = {277, 275};
Line(148) = {276, 307};
Line(149) = {307, 277};
Line(150) = {307, 255};
Line(151) = {255, 308};
Line(152) = {275, 272};
Line(153) = {272, 187};
Line(154) = {271, 183};
Line(155) = {271, 273};
Line(156) = {272, 308};
Line(157) = {308, 271};
Line(158) = {271, 266};
Line(159) = {217, 266};
Line(160) = {268, 221};
Line(161) = {268, 272};
Line(162) = {308, 256};
Line(163) = {266, 256};
Line(164) = {256, 268};
Line(165) = {256, 267};
Line(166) = {268, 270};
Line(167) = {270, 245};
Line(168) = {270, 309};
Line(169) = {309, 269};
Line(170) = {269, 250};
Line(171) = {309, 256};
Line(172) = {267, 234};
Line(173) = {267, 269};
Line(174) = {266, 264};
Line(175) = {264, 14};
Line(176) = {267, 265};
Line(177) = {265, 17};
Line(178) = {264, 310};
Line(179) = {310, 265};
Line(180) = {310, 256};
Line(181) = {264, 257};
Line(182) = {257, 12};
Line(183) = {265, 258};
Line(184) = {258, 18};
Line(185) = {258, 251};
Line(186) = {251, 257};
Line(187) = {251, 310};
Line(188) = {251, 259};
Line(189) = {259, 15};
Line(190) = {259, 260};
Line(191) = {260, 19};
Line(192) = {260, 299};
Line(193) = {299, 261};
Line(194) = {261, 20};
Line(195) = {261, 258};
Line(196) = {251, 299};
Line(197) = {259, 263};
Line(198) = {263, 16};
Line(199) = {257, 262};
Line(200) = {262, 13};
Line(201) = {262, 300};
Line(202) = {300, 263};
Line(203) = {300, 251};
Line(204) = {262, 295};
Line(205) = {252, 300};
Line(206) = {263, 296};
Line(207) = {11, 311};
Line(208) = {311, 312};
Line(209) = {312, 20};
Line(210) = {19, 313};
Line(211) = {313, 314};
Line(212) = {314, 9};
Line(213) = {314, 315};
Line(214) = {315, 4};
Line(215) = {319, 315};
Line(216) = {319, 15};
Line(217) = {319, 313};
Line(218) = {312, 320};
Line(219) = {320, 18};
Line(220) = {316, 320};
Line(221) = {316, 6};
Line(222) = {316, 311};
Line(223) = {315, 322};
Line(224) = {322, 5};
Line(225) = {323, 322};
Line(226) = {16, 323};
Line(227) = {323, 319};
Line(228) = {318, 12};
Line(229) = {318, 317};
Line(230) = {317, 1};
Line(231) = {318, 324};
Line(232) = {324, 13};
Line(233) = {324, 321};
Line(234) = {321, 2};
Line(235) = {321, 317};
Line(236) = {322, 327};
Line(237) = {327, 25};
Line(238) = {37, 330};
Line(239) = {330, 327};
Line(240) = {330, 323};
Line(241) = {324, 328};
Line(242) = {328, 33};
Line(243) = {328, 325};
Line(244) = {325, 41};
Line(245) = {325, 321};
Line(246) = {327, 332};
Line(247) = {332, 26};
Line(248) = {332, 334};
Line(249) = {334, 61};
Line(250) = {298, 301};
Line(251) = {301, 297};
Line(252) = {66, 333};
Line(253) = {333, 331};
Line(254) = {331, 22};
Line(255) = {334, 330};
Line(256) = {50, 329};
Line(257) = {329, 326};
Line(258) = {326, 21};
Line(259) = {333, 329};
Line(260) = {326, 331};
Line(261) = {329, 337};
Line(262) = {337, 49};
Line(263) = {337, 335};
Line(264) = {335, 54};
Line(265) = {335, 326};
Line(266) = {328, 338};
Line(267) = {45, 338};
Line(268) = {338, 336};
Line(269) = {336, 58};
Line(270) = {336, 325};
Line(271) = {336, 340};
Line(272) = {340, 87};
Line(273) = {335, 339};
Line(274) = {339, 71};
Line(275) = {79, 343};
Line(276) = {343, 340};
Line(277) = {343, 338};
Line(278) = {83, 342};
Line(279) = {342, 337};
Line(280) = {339, 342};
Line(281) = {339, 346};
Line(282) = {346, 72};
Line(283) = {346, 347};
Line(284) = {347, 107};
Line(285) = {290, 303};
Line(286) = {303, 291};
Line(287) = {112, 348};
Line(288) = {345, 348};
Line(289) = {68, 345};
Line(290) = {342, 347};
Line(291) = {348, 344};
Line(292) = {344, 96};
Line(293) = {344, 341};
Line(294) = {341, 67};
Line(295) = {341, 345};
Line(296) = {344, 352};
Line(297) = {352, 95};
Line(298) = {352, 349};
Line(299) = {349, 100};
Line(300) = {349, 341};
Line(301) = {91, 351};
Line(302) = {351, 350};
Line(303) = {350, 104};
Line(304) = {351, 343};
Line(305) = {340, 350};
Line(306) = {350, 355};
Line(307) = {355, 133};
Line(308) = {125, 358};
Line(309) = {358, 351};
Line(310) = {352, 356};
Line(311) = {356, 129};
Line(312) = {356, 353};
Line(313) = {353, 117};
Line(314) = {353, 349};
Line(315) = {358, 355};
Line(316) = {358, 365};
Line(317) = {365, 137};
Line(318) = {355, 364};
Line(319) = {364, 150};
Line(320) = {364, 365};
Line(321) = {141, 366};
Line(322) = {366, 363};
Line(323) = {363, 146};
Line(324) = {353, 359};
Line(325) = {359, 118};
Line(326) = {359, 361};
Line(327) = {361, 153};
Line(328) = {361, 356};
Line(329) = {158, 362};
Line(330) = {362, 360};
Line(331) = {114, 360};
Line(332) = {362, 357};
Line(333) = {357, 142};
Line(334) = {357, 354};
Line(335) = {354, 113};
Line(336) = {354, 360};
Line(337) = {354, 363};
Line(338) = {366, 357};
Line(339) = {363, 369};
Line(340) = {369, 163};
Line(341) = {369, 372};
Line(342) = {372, 175};
Line(343) = {372, 366};
Line(344) = {365, 370};
Line(345) = {370, 171};
Line(346) = {370, 368};
Line(347) = {368, 179};
Line(348) = {368, 364};
Line(349) = {164, 374};
Line(350) = {374, 375};
Line(351) = {375, 199};
Line(352) = {204, 376};
Line(353) = {376, 373};
Line(354) = {373, 160};
Line(355) = {374, 369};
Line(356) = {372, 375};
Line(357) = {376, 371};
Line(358) = {371, 188};
Line(359) = {371, 367};
Line(360) = {367, 159};
Line(361) = {367, 373};
Line(362) = {368, 377};
Line(363) = {377, 196};
Line(364) = {377, 379};
Line(365) = {379, 183};
Line(366) = {187, 380};
Line(367) = {380, 378};
Line(368) = {378, 192};
Line(369) = {379, 370};
Line(370) = {371, 380};
Line(371) = {378, 367};
Line(372) = {377, 381};
Line(373) = {381, 225};
Line(374) = {384, 217};
Line(375) = {384, 381};
Line(376) = {379, 384};
Line(377) = {380, 385};
Line(378) = {385, 221};
Line(379) = {385, 382};
Line(380) = {382, 209};
Line(381) = {382, 378};
Line(382) = {382, 387};
Line(383) = {387, 210};
Line(384) = {387, 389};
Line(385) = {389, 245};
Line(386) = {389, 385};
Line(387) = {250, 390};
Line(388) = {390, 388};
Line(389) = {388, 206};
Line(390) = {390, 386};
Line(391) = {386, 234};
Line(392) = {386, 383};
Line(393) = {383, 205};
Line(394) = {388, 383};
Line(395) = {386, 394};
Line(396) = {394, 17};
Line(397) = {394, 391};
Line(398) = {391, 7};
Line(399) = {391, 383};
Line(400) = {384, 393};
Line(401) = {393, 14};
Line(402) = {392, 3};
Line(403) = {392, 381};
Line(404) = {392, 393};
Line(405) = {394, 320};
Line(406) = {316, 391};
Line(407) = {393, 318};
Line(408) = {317, 392};

///////////////////////////////////////////////////////
// Surfaces
///////////////////////////////////////////////////////

Curve Loop(1) = {5, -212, 213, 214};
Plane Surface(1) = {1};
Curve Loop(2) = {213, -215, 217, 211};
Plane Surface(2) = {2};
Curve Loop(3) = {217, -210, -10, -216};
Plane Surface(3) = {3};
Curve Loop(4) = {10, -191, -190, 189};
Plane Surface(4) = {4};
Curve Loop(5) = {190, 192, -196, 188};
Plane Surface(5) = {5};
Curve Loop(6) = {196, 193, 195, 185};
Plane Surface(6) = {6};
Curve Loop(7) = {195, 184, -11, -194};
Plane Surface(7) = {7};
Curve Loop(8) = {219, -11, -209, 218};
Plane Surface(8) = {8};
Curve Loop(9) = {218, -220, 222, 208};
Plane Surface(9) = {9};
Curve Loop(10) = {222, -207, -6, -221};
Plane Surface(10) = {10};
Curve Loop(11) = {223, 224, -3, -214};
Plane Surface(11) = {11};
Curve Loop(12) = {225, -223, -215, -227};
Plane Surface(12) = {12};
Curve Loop(13) = {227, 216, -9, 226};
Plane Surface(13) = {13};
Curve Loop(14) = {9, -189, 197, 198};
Plane Surface(14) = {14};
Curve Loop(15) = {188, 197, -202, 203};
Plane Surface(15) = {15};
Curve Loop(16) = {203, 186, 199, 201};
Plane Surface(16) = {16};
Curve Loop(17) = {199, 200, 7, -182};
Plane Surface(17) = {17};
Curve Loop(18) = {7, -228, 231, 232};
Plane Surface(18) = {18};
Curve Loop(19) = {229, -235, -233, -231};
Plane Surface(19) = {19};
Curve Loop(20) = {235, 230, 1, -234};
Plane Surface(20) = {20};
Curve Loop(21) = {15, -224, 236, 237};
Plane Surface(21) = {21};
Curve Loop(22) = {236, -239, 240, 225};
Plane Surface(22) = {22};
Curve Loop(23) = {240, -226, -17, 238};
Plane Surface(23) = {23};
Curve Loop(24) = {17, -198, 206, 74};
Plane Surface(24) = {24};
Curve Loop(25) = {202, 206, 82, 205};
Plane Surface(25) = {25};
Curve Loop(26) = {205, -201, 204, 81};
Plane Surface(26) = {26};
Curve Loop(27) = {204, -79, 16, -200};
Plane Surface(27) = {27};
Curve Loop(28) = {242, 16, -232, 241};
Plane Surface(28) = {28};
Curve Loop(29) = {233, -245, -243, -241};
Plane Surface(29) = {29};
Curve Loop(30) = {245, 234, -18, -244};
Plane Surface(30) = {30};
Curve Loop(31) = {14, -247, -246, 237};
Plane Surface(31) = {31};
Curve Loop(32) = {248, 255, 239, 246};
Plane Surface(32) = {32};
Curve Loop(33) = {255, -238, -23, -249};
Plane Surface(33) = {33};
Curve Loop(34) = {75, 73, 74, -23};
Plane Surface(34) = {34};
Curve Loop(35) = {82, 83, -250, 73};
Plane Surface(35) = {35};
Curve Loop(36) = {251, 77, 80, 83};
Plane Surface(36) = {36};
Curve Loop(37) = {77, 78, 24, 76};
Plane Surface(37) = {37};
Curve Loop(38) = {24, 252, 259, -256};
Plane Surface(38) = {38};
Curve Loop(39) = {257, 260, -253, 259};
Plane Surface(39) = {39};
Curve Loop(40) = {260, 254, -13, -258};
Plane Surface(40) = {40};
Curve Loop(41) = {21, -264, 265, 258};
Plane Surface(41) = {41};
Curve Loop(42) = {263, 265, -257, 261};
Plane Surface(42) = {42};
Curve Loop(43) = {261, 262, 20, 256};
Plane Surface(43) = {43};
Curve Loop(44) = {86, 20, -78, 85};
Plane Surface(44) = {44};
Curve Loop(45) = {80, 84, 90, -85};
Plane Surface(45) = {45};
Curve Loop(46) = {89, -84, -81, -88};
Plane Surface(46) = {46};
Curve Loop(47) = {88, -79, -19, -87};
Plane Surface(47) = {47};
Curve Loop(48) = {267, -266, 242, -19};
Plane Surface(48) = {48};
Curve Loop(49) = {266, 268, 270, -243};
Plane Surface(49) = {49};
Curve Loop(50) = {269, -22, -244, -270};
Plane Surface(50) = {50};
Curve Loop(51) = {271, 272, 30, -269};
Plane Surface(51) = {51};
Curve Loop(52) = {276, -271, -268, -277};
Plane Surface(52) = {52};
Curve Loop(53) = {277, -267, -28, 275};
Plane Surface(53) = {53};
Curve Loop(54) = {92, 28, -87, 91};
Plane Surface(54) = {54};
Curve Loop(55) = {91, -97, -95, -89};
Plane Surface(55) = {55};
Curve Loop(56) = {96, 94, -90, 95};
Plane Surface(56) = {56};
Curve Loop(57) = {94, 86, -29, 93};
Plane Surface(57) = {57};
Curve Loop(58) = {278, 279, 262, -29};
Plane Surface(58) = {58};
Curve Loop(59) = {279, 263, 273, 280};
Plane Surface(59) = {59};
Curve Loop(60) = {273, 274, 27, -264};
Plane Surface(60) = {60};
Curve Loop(61) = {26, -282, -281, 274};
Plane Surface(61) = {61};
Curve Loop(62) = {283, -290, -280, 281};
Plane Surface(62) = {62};
Curve Loop(63) = {290, 284, 35, 278};
Plane Surface(63) = {63};
Curve Loop(64) = {35, 93, -102, 101};
Plane Surface(64) = {64};
Curve Loop(65) = {102, -96, 104, -285};
Plane Surface(65) = {65};
Curve Loop(66) = {104, 286, -99, -103};
Plane Surface(66) = {66};
Curve Loop(67) = {99, 100, -36, 98};
Plane Surface(67) = {67};
Curve Loop(68) = {36, 287, 291, 292};
Plane Surface(68) = {68};
Curve Loop(69) = {293, 295, 288, 291};
Plane Surface(69) = {69};
Curve Loop(70) = {295, -289, -25, -294};
Plane Surface(70) = {70};
Curve Loop(71) = {34, -303, -305, 272};
Plane Surface(71) = {71};
Curve Loop(72) = {302, -305, -276, -304};
Plane Surface(72) = {72};
Curve Loop(73) = {304, -275, -31, 301};
Plane Surface(73) = {73};
Curve Loop(74) = {31, -92, 105, 106};
Plane Surface(74) = {74};
Curve Loop(75) = {105, -111, -109, 97};
Plane Surface(75) = {75};
Curve Loop(76) = {110, 108, -103, 109};
Plane Surface(76) = {76};
Curve Loop(77) = {108, -98, -32, 107};
Plane Surface(77) = {77};
Curve Loop(78) = {297, 32, -292, 296};
Plane Surface(78) = {78};
Curve Loop(79) = {296, 298, 300, -293};
Plane Surface(79) = {79};
Curve Loop(80) = {300, 294, 33, -299};
Plane Surface(80) = {80};
Curve Loop(81) = {42, -303, 306, 307};
Plane Surface(81) = {81};
Curve Loop(82) = {315, -306, -302, -309};
Plane Surface(82) = {82};
Curve Loop(83) = {309, -301, -40, 308};
Plane Surface(83) = {83};
Curve Loop(84) = {40, -106, 112, 113};
Plane Surface(84) = {84};
Curve Loop(85) = {112, -114, 115, 111};
Plane Surface(85) = {85};
Curve Loop(86) = {115, 110, 116, -118};
Plane Surface(86) = {86};
Curve Loop(87) = {116, 117, 41, 107};
Plane Surface(87) = {87};
Curve Loop(88) = {41, -297, 310, 311};
Plane Surface(88) = {88};
Curve Loop(89) = {310, 312, 314, -298};
Plane Surface(89) = {89};
Curve Loop(90) = {314, 299, -39, -313};
Plane Surface(90) = {90};
Curve Loop(91) = {313, 38, -325, -324};
Plane Surface(91) = {91};
Curve Loop(92) = {326, 328, 312, 324};
Plane Surface(92) = {92};
Curve Loop(93) = {328, 311, -47, -327};
Plane Surface(93) = {93};
Curve Loop(94) = {123, 124, 117, -47};
Plane Surface(94) = {94};
Curve Loop(95) = {124, -118, 125, 127};
Plane Surface(95) = {95};
Curve Loop(96) = {125, -126, -121, 119};
Plane Surface(96) = {96};
Curve Loop(97) = {120, 121, 122, -48};
Plane Surface(97) = {97};
Curve Loop(98) = {329, 332, 333, 48};
Plane Surface(98) = {98};
Curve Loop(99) = {334, 336, -330, 332};
Plane Surface(99) = {99};
Curve Loop(100) = {331, -336, 335, 37};
Plane Surface(100) = {100};
Curve Loop(101) = {45, -323, -337, 335};
Plane Surface(101) = {101};
Curve Loop(102) = {322, -337, -334, -338};
Plane Surface(102) = {102};
Curve Loop(103) = {338, 333, -44, 321};
Plane Surface(103) = {103};
Curve Loop(104) = {44, 120, 128, 129};
Plane Surface(104) = {104};
Curve Loop(105) = {119, -133, -130, -128};
Plane Surface(105) = {105};
Curve Loop(106) = {131, -134, -114, -133};
Plane Surface(106) = {106};
Curve Loop(107) = {134, 132, 43, -113};
Plane Surface(107) = {107};
Curve Loop(108) = {317, 43, 308, 316};
Plane Surface(108) = {108};
Curve Loop(109) = {316, -320, -318, -315};
Plane Surface(109) = {109};
Curve Loop(110) = {319, -46, -307, 318};
Plane Surface(110) = {110};
Curve Loop(111) = {51, -323, 339, 340};
Plane Surface(111) = {111};
Curve Loop(112) = {341, 343, 322, 339};
Plane Surface(112) = {112};
Curve Loop(113) = {343, -321, -53, -342};
Plane Surface(113) = {113};
Curve Loop(114) = {138, 53, -129, -139};
Plane Surface(114) = {114};
Curve Loop(115) = {139, 130, 137, -140};
Plane Surface(115) = {115};
Curve Loop(116) = {141, 136, -131, 137};
Plane Surface(116) = {116};
Curve Loop(117) = {136, 132, -52, -135};
Plane Surface(117) = {117};
Curve Loop(118) = {345, 52, -317, 344};
Plane Surface(118) = {118};
Curve Loop(119) = {344, 346, 348, 320};
Plane Surface(119) = {119};
Curve Loop(120) = {54, -319, -348, 347};
Plane Surface(120) = {120};
Curve Loop(121) = {50, 349, 355, 340};
Plane Surface(121) = {121};
Curve Loop(122) = {350, -356, -341, -355};
Plane Surface(122) = {122};
Curve Loop(123) = {356, 351, 59, -342};
Plane Surface(123) = {123};
Curve Loop(124) = {59, -138, 144, 145};
Plane Surface(124) = {124};
Curve Loop(125) = {144, 148, 150, -140};
Plane Surface(125) = {125};
Curve Loop(126) = {149, 147, -142, -150};
Plane Surface(126) = {126};
Curve Loop(127) = {147, 143, 60, -146};
Plane Surface(127) = {127};
Curve Loop(128) = {60, 352, 357, 358};
Plane Surface(128) = {128};
Curve Loop(129) = {359, 361, -353, 357};
Plane Surface(129) = {129};
Curve Loop(130) = {361, 354, -49, -360};
Plane Surface(130) = {130};
Curve Loop(131) = {347, 58, -363, -362};
Plane Surface(131) = {131};
Curve Loop(132) = {364, 369, 346, 362};
Plane Surface(132) = {132};
Curve Loop(133) = {369, 345, -55, -365};
Plane Surface(133) = {133};
Curve Loop(134) = {154, 55, -135, -155};
Plane Surface(134) = {134};
Curve Loop(135) = {155, -141, 151, 157};
Plane Surface(135) = {135};
Curve Loop(136) = {156, -151, 142, 152};
Plane Surface(136) = {136};
Curve Loop(137) = {152, 153, 56, -143};
Plane Surface(137) = {137};
Curve Loop(138) = {366, -370, 358, -56};
Plane Surface(138) = {138};
Curve Loop(139) = {359, -371, -367, -370};
Plane Surface(139) = {139};
Curve Loop(140) = {371, 360, 57, -368};
Plane Surface(140) = {140};
Curve Loop(141) = {66, -363, 372, 373};
Plane Surface(141) = {141};
Curve Loop(142) = {372, -375, -376, -364};
Plane Surface(142) = {142};
Curve Loop(143) = {376, 374, 64, -365};
Plane Surface(143) = {143};
Curve Loop(144) = {64, -154, 158, -159};
Plane Surface(144) = {144};
Curve Loop(145) = {157, 158, 163, -162};
Plane Surface(145) = {145};
Curve Loop(146) = {162, 164, 161, 156};
Plane Surface(146) = {146};
Curve Loop(147) = {161, 153, -65, -160};
Plane Surface(147) = {147};
Curve Loop(148) = {65, 366, 377, 378};
Plane Surface(148) = {148};
Curve Loop(149) = {367, -381, -379, -377};
Plane Surface(149) = {149};
Curve Loop(150) = {381, 368, -63, -380};
Plane Surface(150) = {150};
Curve Loop(151) = {62, -383, -382, 380};
Plane Surface(151) = {151};
Curve Loop(152) = {382, 384, 386, 379};
Plane Surface(152) = {152};
Curve Loop(153) = {386, 378, -71, -385};
Plane Surface(153) = {153};
Curve Loop(154) = {167, 71, -160, 166};
Plane Surface(154) = {154};
Curve Loop(155) = {166, 168, 171, 164};
Plane Surface(155) = {155};
Curve Loop(156) = {171, 165, 173, -169};
Plane Surface(156) = {156};
Curve Loop(157) = {173, 170, -72, -172};
Plane Surface(157) = {157};
Curve Loop(158) = {387, 390, 391, 72};
Plane Surface(158) = {158};
Curve Loop(159) = {390, 392, -394, -388};
Plane Surface(159) = {159};
Curve Loop(160) = {394, 393, 61, -389};
Plane Surface(160) = {160};
Curve Loop(161) = {69, -398, 399, 393};
Plane Surface(161) = {161};
Curve Loop(162) = {397, 399, -392, 395};
Plane Surface(162) = {162};
Curve Loop(163) = {395, 396, 68, -391};
Plane Surface(163) = {163};
Curve Loop(164) = {68, -172, 176, 177};
Plane Surface(164) = {164};
Curve Loop(165) = {165, 176, -179, 180};
Plane Surface(165) = {165};
Curve Loop(166) = {180, -163, 174, 178};
Plane Surface(166) = {166};
Curve Loop(167) = {174, 175, 67, 159};
Plane Surface(167) = {167};
Curve Loop(168) = {67, -374, 400, 401};
Plane Surface(168) = {168};
Curve Loop(169) = {375, -403, 404, -400};
Plane Surface(169) = {169};
Curve Loop(170) = {403, 373, 70, -402};
Plane Surface(170) = {170};
Curve Loop(171) = {398, -4, -221, 406};
Plane Surface(171) = {171};
Curve Loop(172) = {406, -397, 405, -220};
Plane Surface(172) = {172};
Curve Loop(173) = {405, 219, 12, -396};
Plane Surface(173) = {173};
Curve Loop(174) = {12, -177, 183, 184};
Plane Surface(174) = {174};
Curve Loop(175) = {183, 185, 187, 179};
Plane Surface(175) = {175};
Curve Loop(176) = {187, -178, 181, -186};
Plane Surface(176) = {176};
Curve Loop(177) = {181, 182, 8, -175};
Plane Surface(177) = {177};
Curve Loop(178) = {8, -401, 407, 228};
Plane Surface(178) = {178};
Curve Loop(179) = {407, 229, 408, 404};
Plane Surface(179) = {179};
Curve Loop(180) = {408, 402, -2, -230};
Plane Surface(180) = {180};

Transfinite Surface "*";
Recombine Surface "*";

///////////////////////////////////////////////////////
// Transfinite Curves
///////////////////////////////////////////////////////

// longitudinal curves
Transfinite Curve {6, 222, 218, 11, 195, 196, 190, 10, 217, 213, 5, 4, 406, 405, 12, 183, 187, 181, 8, 407, 408, 2, 1, 235, 231, 7, 199, 203, 197, 9, 227, 223, 3, 18, 245, 241, 16, 204, 205, 206, 17, 240, 236, 15, 14, 246, 255, 23, 73, 83, 77, 24, 259, 260, 13, 22, 270, 266, 19, 88, 84, 85, 20, 261, 265, 21, 30, 271, 277, 28, 91, 95, 94, 29, 279, 273, 27, 25, 295, 291, 36, 99, 104, 102, 35, 290, 281, 26, 33, 300, 296, 32, 108, 109, 105, 31, 304, 305, 34, 42, 306, 309, 40, 112, 115, 116, 41, 310, 39, 314, 37, 336, 332, 48, 121, 125, 124, 47, 328, 324, 38, 45, 337, 338, 44, 128, 133, 134, 43, 316, 318, 46, 51, 339, 343, 139, 53, 137, 136, 52, 344, 348, 54, 362, 58, 369, 55, 155, 151, 152, 56, 370, 57, 371, 49, 361, 357, 60, 147, 150, 144, 59, 356, 355, 50, 63, 381, 377, 65, 161, 162, 158, 64, 376, 372, 66, 382, 62, 386, 71, 166, 171, 173, 72, 390, 394, 61, 69, 399, 395, 68, 176, 180, 174, 67, 400, 403, 70} = Narms Using Progression 1;

// transverse curves, inner loop (under load pad)
Transfinite Curve {229, 233, 243, 268, 276, 302, 315, 320, 346, 364, 375, 404} = Ninner Using Progression 1;

// transverse curves, outer loop (above load pad)
Transfinite Curve {211, 215, 225, 239, 248, 253, 257, 263, 280, 283, 288, 293, 298, 312, 326, 330, 334, 322, 341, 350, 353, 359, 367, 379, 384, 388, 392, 397, 220, 208} = Nouter Using Progression 1;

// transverse curves, load pad gap
Transfinite Curve {192, 193, 185, 188, 186, 201, 202, 81, 80, 82, 90, 89, 96, 103, 97, 250, 251, 286, 285, 110, 111, 118, 119, 114, 127, 126, 130, 131, 140, 141, 142, 148, 149, 157, 156, 163, 164, 165, 168, 169, 178, 179} = Ngap Using Progression 1;

// transverse curves, boundary layers
Transfinite Curve {207, 209, 194, 191, 210, 212, 221, 219, 184, 189, 216, 214, 182, 228, 230, 234, 232, 200, 198, 226, 224, 237, 238, 74, 79, 242, 244, 78, 256, 258, 247, 249, 75, 76, 252, 254, 269, 267, 87, 86, 262, 264, 272, 275, 92, 93, 278, 274, 98, 292, 294, 289, 287, 100, 101, 284, 282, 303, 301, 106, 107, 297, 299, 313, 311, 117, 113, 308, 307, 120, 333, 335, 331, 329, 122, 123, 327, 325, 323, 321, 129, 132, 317, 319, 340, 342, 138, 143, 358, 360, 135, 345, 347, 349, 351, 145, 146, 352, 354, 368, 366, 153, 154, 365, 363, 373, 374, 159, 160, 378, 380, 172, 391, 393, 383, 385, 167, 170, 387, 389, 402, 401, 175, 177, 396, 398} = Nbl Using Progression 1;

// Extrusion #1 - up to load pad base

ex1 = Extrude {0,0,Lz1} // up to load pad base
{
  Surface{:};
  Layers{ {Nz11,Nz12}, {lz11,lz12} };
  Recombine;
};

Translate {0, 0, padHeight} {
  Duplicata { Surface{430}; Surface{672}; Surface{650}; Surface{452}; Surface{474}; Surface{694}; Surface{584}; Surface{606}; Surface{628}; Surface{4170}; Surface{4192}; Surface{4214}; Surface{4324}; Surface{804}; Surface{826}; Surface{4346}; Surface{4368}; Surface{848}; Surface{1090}; Surface{870}; Surface{892}; Surface{1112}; Surface{1134}; Surface{914}; Surface{1024}; Surface{1046}; Surface{1068}; Surface{1508}; Surface{1486}; Surface{1464}; Surface{1354}; Surface{1332}; Surface{1310}; Surface{1288}; Surface{1266}; Surface{1244}; Surface{1948}; Surface{1926}; Surface{1904}; Surface{2146}; Surface{2168}; Surface{2124}; Surface{1992}; Surface{2014}; Surface{1970}; Surface{1530}; Surface{1552}; Surface{1574}; Surface{1684}; Surface{1794}; Surface{1772}; Surface{1706}; Surface{1750}; Surface{1728}; Surface{2366}; Surface{2388}; Surface{2410}; Surface{2432}; Surface{2344}; Surface{2454}; Surface{2212}; Surface{2190}; Surface{2234}; Surface{2828}; Surface{2806}; Surface{2784}; Surface{2652}; Surface{2586}; Surface{2564}; Surface{2674}; Surface{2630}; Surface{2608}; Surface{2872}; Surface{2850}; Surface{3070}; Surface{3092}; Surface{3114}; Surface{2894}; Surface{3026}; Surface{3048}; Surface{3004}; Surface{3312}; Surface{3290}; Surface{3334}; Surface{3466}; Surface{3224}; Surface{3444}; Surface{3246}; Surface{3268}; Surface{3488}; Surface{4148}; Surface{4126}; Surface{4104}; Surface{3532}; Surface{3510}; Surface{3554}; Surface{3686}; Surface{3664}; Surface{3708}; Surface{3730}; Surface{3752}; Surface{3774}; Surface{3928}; Surface{3906}; Surface{3884}; Surface{3994}; Surface{3972}; Surface{3950}; }
}

Extrude {0, 0, Lz2} {
  Surface{1200}; Surface{1178}; Surface{1156}; Surface{1222}; Surface{936}; Surface{958}; Surface{980}; Surface{1002}; Surface{1442}; Surface{1420}; Surface{1398}; Surface{1376}; Surface{1640}; Surface{1662}; Surface{1596}; Surface{1618}; Surface{2058}; Surface{2036}; Surface{2080}; Surface{2102}; Surface{1882}; Surface{1860}; Surface{1838}; Surface{1816}; Surface{2278}; Surface{2300}; Surface{2322}; Surface{2256}; Surface{2762}; Surface{2740}; Surface{2718}; Surface{2696}; Surface{2520}; Surface{2542}; Surface{2498}; Surface{2476}; Surface{2938}; Surface{2960}; Surface{2982}; Surface{2916}; Surface{3136}; Surface{3158}; Surface{3180}; Surface{3202}; Surface{3400}; Surface{3422}; Surface{3378}; Surface{3356}; Surface{3818}; Surface{3840}; Surface{3620}; Surface{3598}; Surface{4060}; Surface{4038}; Surface{3796}; Surface{3642}; Surface{3576}; Surface{4082}; Surface{4016}; Surface{3862}; Surface{496}; Surface{518}; Surface{540}; Surface{562}; Surface{4236}; Surface{4258}; Surface{4280}; Surface{760}; Surface{738}; Surface{716}; Surface{782}; Surface{4302};
  Layers{ {Nz2}, {lz21} };
  Recombine;
}

Extrude {0, 0, Lz3} {
  Surface{4474}; Surface{4459}; Surface{4464}; Surface{4469}; Surface{4484}; Surface{4479}; Surface{4971}; Surface{4949}; Surface{4927}; Surface{4993}; Surface{4544}; Surface{4539}; Surface{4534}; Surface{5037}; Surface{5015}; Surface{5059}; Surface{5081}; Surface{4489}; Surface{4494}; Surface{4499}; Surface{4504}; Surface{4509}; Surface{4514}; Surface{5103}; Surface{5125}; Surface{5147}; Surface{4519}; Surface{5169}; Surface{4524}; Surface{4529}; Surface{4579}; Surface{4599}; Surface{4594}; Surface{4589}; Surface{4604}; Surface{4584}; Surface{5301}; Surface{5235}; Surface{5257}; Surface{5279}; Surface{5323}; Surface{5389}; Surface{5191}; Surface{5411}; Surface{4614}; Surface{5433}; Surface{4609}; Surface{5213}; Surface{4619}; Surface{4624}; Surface{4634}; Surface{4629}; Surface{5345}; Surface{4574}; Surface{4559}; Surface{5367}; Surface{4554}; Surface{4564}; Surface{4569}; Surface{4549}; Surface{4699}; Surface{4714}; Surface{5609}; Surface{5587}; Surface{5565}; Surface{5543}; Surface{4694}; Surface{4689}; Surface{4684}; Surface{4719}; Surface{4724}; Surface{4704}; Surface{4709}; Surface{5653}; Surface{5631}; Surface{5675}; Surface{5477}; Surface{5455}; Surface{5521}; Surface{4679}; Surface{4669}; Surface{4674}; Surface{4654}; Surface{4639}; Surface{4659}; Surface{5499}; Surface{5697}; Surface{4664}; Surface{4649}; Surface{4644}; Surface{4804}; Surface{4809}; Surface{4814}; Surface{4789}; Surface{4799}; Surface{4794}; Surface{5873}; Surface{5917}; Surface{5895}; Surface{5851}; Surface{5829}; Surface{5807}; Surface{4749}; Surface{4744}; Surface{4739}; Surface{4729}; Surface{4734}; Surface{4754}; Surface{5785}; Surface{5719}; Surface{5741}; Surface{5763}; Surface{4769}; Surface{4784}; Surface{5961}; Surface{5939}; Surface{4759}; Surface{4764}; Surface{4779}; Surface{4774}; Surface{4859}; Surface{4864}; Surface{4869}; Surface{4849}; Surface{4854}; Surface{4874}; Surface{6115}; Surface{6137}; Surface{6027}; Surface{6049}; Surface{6159}; Surface{4834}; Surface{4844}; Surface{4839}; Surface{4824}; Surface{4819}; Surface{4829}; Surface{6181}; Surface{6071}; Surface{6093}; Surface{6203}; Surface{4894}; Surface{6005}; Surface{5983}; Surface{6225}; Surface{4889}; Surface{4884}; Surface{4879}; Surface{4904}; Surface{4899}; Surface{4419}; Surface{4404}; Surface{4414}; Surface{4409}; Surface{4424}; Surface{4399}; Surface{6313}; Surface{6335}; Surface{6357}; Surface{6379}; Surface{4429}; Surface{6489}; Surface{4444}; Surface{4449}; Surface{4454}; Surface{4439}; Surface{4434}; Surface{6401}; Surface{6467}; Surface{6423}; Surface{6269}; Surface{6291}; Surface{6247}; Surface{4389}; Surface{6445}; Surface{4394}; Surface{4374}; Surface{4384}; Surface{4369}; Surface{4379};
  Layers{ {Nz31,Nz32}, {lz31,lz32} };
  Recombine;
}

Physical Volume("fluid", 1) = {374, 375, 378, 379, 396, 397, 399, 398, 395, 400, 344, 343, 348, 349, 352, 353, 354, 355, 356, 357, 373, 376, 377, 380, 381, 382, 345, 383, 385, 346, 384, 347, 350, 386, 351, 368, 367, 366, 372, 371, 401, 402, 394, 393, 392, 391, 390, 389, 387, 388, 359, 360, 358, 361, 362, 363, 364, 365, 369, 370, 405, 403, 407, 410, 411, 412, 414, 413, 415, 416, 322, 313, 314, 315, 316, 317, 318, 320, 319, 321, 406, 404, 408, 409, 424, 423, 425, 426, 430, 431, 323, 324, 325, 326, 327, 328, 339, 340, 335, 341, 422, 427, 420, 428, 417, 429, 419, 418, 421, 432, 334, 329, 330, 338, 337, 333, 332, 331, 336, 342, 272, 267, 256, 257, 271, 255, 266, 270, 269, 268, 286, 283, 305, 306, 310, 288, 293, 289, 292, 311, 273, 274, 275, 276, 277, 278, 280, 279, 281, 282, 285, 284, 287, 290, 291, 295, 300, 299, 302, 303, 254, 253, 258, 259, 260, 261, 262, 263, 264, 265, 312, 307, 309, 308, 294, 296, 298, 297, 301, 304, 235, 229, 240, 230, 224, 223, 222, 221, 236, 231, 232, 237, 226, 225, 227, 228, 239, 234, 233, 238, 220, 217, 218, 219, 245, 246, 247, 252, 212, 211, 210, 209, 244, 243, 242, 241, 214, 213, 215, 216, 249, 250, 248, 251, 206, 205, 207, 208, 185, 186, 188, 187, 200, 199, 198, 197, 189, 190, 191, 192, 195, 196, 193, 194, 183, 182, 181, 184, 201, 202, 203, 204, 151, 152, 153, 154, 155, 157, 159, 158, 156, 160, 130, 129, 128, 127, 126, 125, 124, 123, 122, 121, 150, 149, 148, 147, 146, 145, 140, 144, 143, 139, 142, 138, 137, 141, 136, 135, 134, 133, 132, 131, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 111, 113, 112, 114, 115, 116, 117, 118, 119, 120, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 101, 102, 103, 104, 105, 106, 107, 109, 108, 110, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 100, 99, 98, 97, 96, 95, 94, 93, 92, 91, 15, 14, 16, 13, 20, 12, 18, 19, 17, 11, 81, 86, 85, 87, 88, 82, 83, 84, 89, 90, 30, 24, 22, 23, 29, 21, 25, 28, 27, 26, 71, 72, 77, 78, 79, 73, 76, 74, 75, 80, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 70, 68, 69, 67, 66, 65, 64, 63, 62, 61};

Physical Surface("inlet", 3) = {129, 130, 140, 139, 138, 137, 127, 128, 136, 126, 125, 115, 124, 123, 113, 114, 112, 122, 121, 111, 135, 116, 134, 133, 118, 117, 119, 132, 131, 120, 141, 170, 142, 169, 168, 143, 144, 167, 145, 166, 165, 146, 150, 149, 148, 147, 154, 153, 152, 151, 155, 156, 157, 158, 159, 162, 163, 164, 161, 160, 15, 16, 176, 175, 6, 5, 3, 4, 14, 13, 17, 18, 178, 177, 179, 19, 20, 180, 2, 12, 1, 11, 7, 174, 8, 9, 10, 172, 173, 171, 22, 32, 31, 21, 23, 33, 34, 24, 35, 25, 26, 36, 45, 46, 47, 27, 28, 48, 49, 29, 30, 50, 39, 38, 37, 43, 44, 42, 40, 41, 59, 60, 61, 62, 63, 58, 57, 65, 64, 56, 55, 66, 76, 75, 54, 74, 53, 73, 72, 52, 51, 71, 67, 68, 78, 77, 79, 69, 70, 80, 109, 82, 83, 81, 110, 108, 84, 107, 106, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 105, 104, 103, 98, 97, 99, 102, 101, 100};
//+
Physical Surface("outlet", 4) = {7589, 7611, 7633, 7567, 7479, 7523, 7545, 7501, 7435, 7457, 7413, 7391, 7369, 7347, 7325, 7259, 7193, 7215, 7237, 7171, 7281, 7303, 7655, 7677, 7699, 7721, 7743, 7765, 7787, 7809, 8293, 8271, 8249, 8227, 8205, 8183, 8381, 8359, 8337, 8469, 8447, 8315, 8425, 8403, 8161, 8139, 8117, 8095, 8073, 8051, 8029, 7831, 7853, 7897, 7875, 7919, 7941, 7963, 7985, 8007, 9063, 9129, 9085, 9107, 9019, 8997, 8975, 8953, 9041, 8931, 8909, 8711, 8689, 8667, 8645, 8579, 8601, 8623, 8733, 8755, 8821, 8887, 8865, 8843, 8799, 8777, 8557, 8491, 8535, 8513, 9459, 9481, 9503, 9525, 9547, 9569, 9591, 9613, 9789, 9767, 9723, 9745, 9701, 9679, 9635, 9657, 9283, 9261, 9195, 9173, 9151, 9217, 9239, 9305, 9327, 9349, 9371, 9415, 9393, 9437, 10427, 10405, 10295, 10317, 10251, 10273, 9921, 9943, 9833, 9877, 9855, 9811, 9899, 9965, 9987, 10009, 10053, 10031, 10075, 10097, 10119, 10141, 10163, 10207, 10185, 10229, 10339, 10383, 10361, 10449, 6929, 6951, 6973, 6907, 6885, 6863, 6995, 7017, 7039, 6841, 7061, 6687, 7105, 6709, 6731, 7083, 6753, 7149, 6775, 7127, 6665, 6797, 6819, 6643, 6621, 6599, 6577, 6511, 6533, 6555};

Physical Surface("inwall", 1) = {7210, 6942, 5226, 5098, 1499, 1525, 1552, 1574, 1530, 1486, 1464, 1508, 4599, 4509, 4514, 4604, 4594, 4504, 7224, 8280, 2177, 1957, 5508, 5288, 2819, 3035, 5758, 5538, 7998, 9072, 3497, 3281, 5952, 6146, 9424, 9098, 9476, 10092, 6484, 6176, 4143, 4363, 1063, 843, 10114, 6462, 5076, 6924, 1992, 2014, 1970, 2212, 2234, 2190, 2806, 2784, 2828, 3026, 3004, 3048, 3290, 3312, 3334, 4126, 3532, 3510, 4148, 4104, 3554, 4346, 826, 848, 4368, 804, 4324, 1046, 1068, 1024, 4494, 4499, 4489, 4444, 4439, 4454, 4449, 4429, 4434, 4834, 4824, 4839, 4819, 4829, 4844, 4774, 4784, 4779, 4759, 4769, 4764, 4689, 4694, 4684, 4669, 4674, 4679, 4579, 4589, 4584};

Physical Surface("outwall", 2) = {9740, 9754, 9846, 9872, 6322, 6308, 6190, 6220, 3923, 3937, 4161, 623, 10414, 10444, 6542, 6520, 4970, 5002, 6432, 6234, 417, 645, 857, 1077, 6770, 7136, 7606, 7620, 5420, 5208, 5160, 4988, 1737, 1723, 1297, 1283, 7804, 7782, 8464, 8438, 5696, 5494, 5340, 5362, 1943, 2163, 2383, 2401, 8786, 8830, 8016, 8050, 5652, 5596, 5776, 5794, 3057, 2837, 2617, 2607, 9160, 9146, 8530, 8508, 6106, 6132, 5912, 5868, 3717, 3703, 3483, 3263};

Physical Surface("outwall", 2) += {2564, 2586, 2608, 2674, 2652, 2630, 2872, 3092, 3070, 2850, 2894, 3114, 3224, 3246, 3268, 3488, 3466, 3444, 3752, 3686, 3664, 3774, 3730, 3708, 3906, 3972, 3994, 3884, 3928, 3950, 4192, 606, 584, 4214, 4170, 628, 672, 452, 430, 650, 694, 474, 892, 1112, 1090, 870, 1134, 914, 1266, 1332, 1354, 1244, 1288, 1310, 1706, 1772, 1794, 1684, 1728, 1750, 1926, 2146, 1904, 2124, 2168, 1948, 2432, 2366, 2344, 2454, 2388, 2410};

Physical Surface("outwall", 2) += {4744, 4729, 4734, 4739, 4749, 4754, 4704, 4699, 4719, 4724, 4714, 4709, 4789, 4804, 4794, 4799, 4814, 4809, 4869, 4849, 4854, 4874, 4864, 4859, 4879, 4904, 4899, 4884, 4889, 4894, 4404, 4409, 4414, 4419, 4424, 4399, 4374, 4384, 4369, 4379, 4394, 4389, 4474, 4469, 4459, 4464, 4484, 4479, 4524, 4539, 4534, 4529, 4519, 4544, 4619, 4624, 4634, 4629, 4609, 4614, 4554, 4564, 4549, 4569, 4574, 4559, 4639, 4654, 4649, 4644, 4659, 4664};

Physical Surface("periodic1", 5) = {9186, 9164, 9260, 9270, 9648, 9634, 9670, 9688, 9722, 9744, 5974, 6004, 6216, 6102, 3839, 3809, 3783, 3773, 3743, 3721, 3853, 3871, 3905, 3927};

Physical Surface("periodic2", 6) = {7624, 7554, 7470, 7500, 7456, 7404, 7712, 7690, 7738, 7800, 5432, 5410, 5380, 5358, 1759, 1785, 1815, 1837, 1851, 1873, 1895, 1921, 1939, 1741};

Physical Surface("periodic3", 7) = {8702, 8676, 8732, 8746, 8764, 8790, 8622, 8592, 8486, 8504, 5838, 5820, 5806, 5872, 3259, 3241, 3215, 3201, 3167, 3149, 3135, 3105, 3079, 3061};

Physical Surface("periodic4", 8) = {1099, 1081, 1133, 1143, 1173, 1187, 1221, 1235, 1261, 1279, 4958, 4944, 4914, 4992, 6524, 6498, 6630, 6620, 6660, 6674, 6708, 6722, 6748, 6766};

Physical Surface("periodic5", 9) = {2419, 2405, 2453, 2463, 2497, 2511, 2537, 2551, 2581, 2595, 5684, 5674, 5622, 5648, 8302, 8442, 8424, 8390, 8160, 8130, 8112, 8082, 8068, 8038};

Physical Surface("periodic6", 10) = {531, 509, 487, 465, 451, 421, 561, 579, 605, 619, 6282, 6260, 6238, 6312, 9868, 9832, 9916, 9942, 10264, 10242, 10286, 10308, 10404, 10418};
