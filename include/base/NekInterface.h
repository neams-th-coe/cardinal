#ifndef CARDINAL_NEKINTERFACE_H
#define CARDINAL_NEKINTERFACE_H

#include "Moose.h"

// String length for file paths in Nek5000
#define PATHLEN 132


namespace Nek5000
{
// Nek Fortran interface
extern "C" {

// CFILES common block
extern struct {
  char \
        reafle[PATHLEN], \
        fldfle[PATHLEN], \
        dmpfle[PATHLEN], \
        hisfle[PATHLEN], \
        schfle[PATHLEN], \
        orefle[PATHLEN], \
        nrefle[PATHLEN];
} cfiles_;

// DIMN common block
extern struct {
  int \
        nelv,  \
        nelt,  \
        nx1,   \
        ny1,   \
        nz1,   \
        nx2,   \
        ny2,   \
        nz2,   \
        nx3,   \
        ny3,   \
        nz3,   \
        ndim,  \
        nfield,\
        npert, \
        nid,   \
        nxd,   \
        nyd,   \
        nzd;
} dimn_;

// DIMN common block
extern struct {
  double flux_moose, temp_nek;
} test_passing_;

extern struct {
  double pc_x[];     // vertex X coordinate, in sequential order
} point_cloudx_;

extern struct {
  double pc_y[];     // vertex Y coordinate, in sequential order
} point_cloudy_;

extern struct {
  double pc_z[];     // vertex Z coordinate, in sequential order
} point_cloudz_;

extern struct {
  double pc_t[];     // temperature vertex data    
} point_cloudt_;

extern struct {
  double pc_f[];     // flux vertex data
} point_flux_;

extern struct {
  double pc_flag[];  // element number for each vertex      
} ref_element_;

extern struct {
  long int  nw_dbt;   // total number of elements (we are assuming QUAD4, so vertices are obtained as 4*nw_bdt 
} tot_surf_;

// subroutine nek_init(intracomm)
void FORTRAN_CALL(nek_init)(const int &);   // initilization, pass a communicator
void FORTRAN_CALL(nek_init_step)();         // call once before each time step
void FORTRAN_CALL(nek_step)();              // call for each picard iteration within the step
void FORTRAN_CALL(nek_finalize_step)();     // call at the end of step
void FORTRAN_CALL(nek_interpolation)();  // to be called before extracting data from common blocks
void FORTRAN_CALL(nek_pointscloud)();    // to be called before extracting point cloud locations from common blocks
void FORTRAN_CALL(flux_reconstruction)(); // to be called after loading flux data in common blocks
}
}

#endif //CARDINAL_NEKINTERFACE_H
