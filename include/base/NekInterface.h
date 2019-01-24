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
  double pc_x[];
  double pc_y[];
  double pc_z[];  
  double pc_flag[];
} point_cloud_;

extern struct {
  double pc_t[];
  double pc_f[];
} data_cloud_;

// subroutine nek_init(intracomm)
void FORTRAN_CALL(nek_init)(const int&);  // Ron likes this better now
void FORTRAN_CALL(nek_init_step)();
void FORTRAN_CALL(nek_step)();
void FORTRAN_CALL(nek_finalize_step)();
void FORTRAN_CALL(nek_pointscloud)();
void FORTRAN_CALL(flux_reconstruction)();
}
}

#endif //CARDINAL_NEKINTERFACE_H
