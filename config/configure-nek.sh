#!/bin/bash
#-------------------------------------------------------------------------------
# Nek5000 config file
#-------------------------------------------------------------------------------

# source path 
#SOURCE_ROOT="$HOME/Nek5000" 

# Fortran/C compiler
#FC="mpif77"
#CC="mpicc"

# pre-processor list (set to "?" to get a list of available symbols)
#PPLIST="?" 

#-------------------------------------------------------------------------------
# WHAT FOLLOWS ARE OPTIONAL SETTINGS
#-------------------------------------------------------------------------------

# optional compiler flags
#FFLAGS=""
#CFLAGS=""

# optional linking flags
#USR_LFLAGS+=" -L/usr/lib -lfoo"

# auxilliary files to compile
# NOTE: source files have to located in the same directory as makenek
#       a makefile_usr.inc has to be provided containing the build rules 
#USR="foo.o"

# MPI (default 1) 
#MPI=0

# profiling (default 1)
#PROFILING=0

# VisIt in situ (default 0)
#VISIT=1
#  Note: you can override the lib and include paths. VISIT_LIB and VISIT_INC
#  If VISIT_STOP is set the simulation will stop after first step and wait
#  for VisIt to connect.
#VISIT_INSTALL="/path/to/visit/current/linux-x86_64/"
#VISIT_STOP=true


###############################################################################
# DONT'T TOUCH WHAT FOLLOWS !!!
###############################################################################
set -e

# assign version tag
mver=17.0.4

# set defaults
: ${SOURCE_ROOT:="$HOME/Nek5000"}
: ${FC:="mpif77"}
: ${CC:="mpicc"}
: ${MPI:=1}
: ${PROFILING:=1}
: ${VISIT:=0}
: ${PPLIST:=""}
: ${CASEDIR:=`pwd`}

# overwrite source path with optional 2nd argument
if [ -d $2 ] && [ $# -eq 2 ]; then
  SOURCE_ROOT="$2"
  echo "change source code directory to: ", $SOURCE_ROOT
fi

# create makefile
function make_3rd_party() {
  if [ $NOCOMPILE -ne 0 ]; then
    return 0
  fi
  echo "build 3rd-party software ..."

  cd $SOURCE_ROOT/3rd_party/gslib
  ./install
  cd $CASEDIR 

  if [ $CVODE -ne 0 ]; then
    cd $SOURCE_ROOT/3rd_party/cvode
    ./install
  fi
  cd $CASEDIR

  if [ $HYPRE -ne 0 ]; then
    cd $SOURCE_ROOT/3rd_party/hypre
    ./install
  fi
  cd $CASEDIR
}

function clean_3rd_party() {
  echo "cleaning 3rd-party software ..."
  cd $SOURCE_ROOT/3rd_party/gslib; ./install clean 2>/dev/null
  cd $CASEDIR
  cd $SOURCE_ROOT/3rd_party/cvode; ./install clean 2>/dev/null
  cd $CASEDIR
  cd $SOURCE_ROOT/3rd_party/hypre; ./install clean 2>/dev/null
  cd $CASEDIR
}

# This include file is used by the makenek script
# to automatically create a makefile for Nek5000 
# ------------------------------------------------

echo "makenek - automatic build tool for Nek5000"

if [ "$PPLIST" == "?" ]; then
  echo "available pre-processor symbols:" 
  echo "  NOMPIIO     deactivate MPI-IO support"
  echo "  BGQ         use BGQ optimized mxm"
  echo "  XSMM        use libxsmm for mxm"
  echo "  CVODE       compile with CVODE support for scalars"
  echo "  HYPRE       compile with HYPRE support"
  echo "  VENDOR_BLAS use VENDOR BLAS/LAPACK"
  exit 1
fi

# first do some checks ...
if [ "$1" == "-h" ]; then
  echo ""
  echo "usage: makenek [.usr filename | clean] < -nocompile >"
  echo ""
  exit 1
fi

if [ "$mver" != "17.0.4" ]; then
  echo "FATAL ERROR: Your makenek is incompatible!"
  echo "Please upgrade to the version shipped with your Nek source."
  exit 1
fi

if [ ! "$SOURCE_ROOT_GSLIB" ]; then
  SOURCE_ROOT_GSLIB="$SOURCE_ROOT/3rd_party/gslib"
  export SOURCE_ROOT_GSLIB
fi

if [[ $1 = "clean" ]]; then
  while true; do
    read -p "do you want to clean all 3rd party software? [N]" yn
    case $yn in
      [Yy]* )
        clean_3rd_party; 
        make clean;
        break ;;
      * ) 
        make clean;
        break ;;
    esac
  done
  rm -rf compiler.out
  rm -rf makefile
  rm -rf .state
  exit 0
fi

NOCOMPILE=0
if [[ "$2" == "-nocompile" ]]; then
  NOCOMPILE=1
fi 

if [ ! -f "$CASEDIR/$CASENAME.usr" ]; then
  echo "FATAL ERROR: Cannot find $CASEDIR/$CASENAME.usr!"
  exit 1
fi

if [ -f $CASEDIR/SIZEu ]; then
  if [ -f $CASEDIR/SIZE ]; then
    echo "FATAL ERROR: found SIZEu and SIZE!"
    exit 1
  fi
  NEK_WARN="WARNING: SIZEu will be deprecated in a future release. Please rename to SIZE!"
  mv -v $CASEDIR/SIZEu $CASEDIR/SIZE
fi

if [ ! -f $CASEDIR/SIZE ]; then
  echo "FATAL ERROR: Cannot find SIZE!"
  exit 1
fi

if [ ! -f $CARDINAL_DIR/config/Makefile.nek.in ]; then
  echo "FATAL ERROR: Cannot find $CARDINAL_DIR/config/Makefile.nek.in"
  exit 1
fi

# test if FC compiler exists
if ! which `echo $FC | awk '{print $1}'` 1>/dev/null ; then
  echo "FATAL ERROR: Cannot find `echo $FC | awk '{print $1}'`!"
  exit 1
fi


# test if C compiler exists
if ! which `echo $CC | awk '{print $1}'` 1>/dev/null ; then 
  echo "FATAL ERROR: Cannot find `echo $CC | awk '{print $1}'`!"
  exit 1
fi

# trying to figure which compiler the wrapper is using 
FCok=0

FCcomp_=`$FC -showme 2>/dev/null | head -1 2>/dev/null 1>.tmp || true`
FCcomp=`cat .tmp | awk '{print $1}' | awk -F/ '{print $NF}' || true`
if [ -f "`which $FCcomp 2>/dev/null`" ]; then
  FCok=1
fi

if [ $FCok -eq 0 ]; then
  FCcomp_=`$FC -show 2>/dev/null | head -1 2>/dev/null 1>.tmp || true`
  FCcomp=`cat .tmp | awk '{print $1}' | awk -F/ '{print $NF}' || true`
  if [ -f "`which $FCcomp 2>/dev/null`" ]; then
    FCok=1
  fi
fi

if [ $FCok -eq 0 ]; then
  FCcomp_=`$FC -craype-verbose 2>/dev/null 1>.tmp || true`
  FCcomp=`cat .tmp | awk '{print $1}' | awk -F/ '{print $NF}' || true`
  if [ -f "`which $FCcomp 2>/dev/null`" ]; then
    FCok=1
  fi
fi

if [ $FCok -eq 0 ]; then
  FCcomp=`echo $FC | awk '{print $1}'`
  if [ -f "`which $FCcomp 2>/dev/null`" ]; then
    FCok=1
  fi
fi

\rm -f .tmp
if [ $FCok -eq 0 ]; then
  FCcomp="unknown"
fi

# assign FC compiler specific flags
case $FCcomp in
  *pgf*)        FFLAGS+=" -r8 -Mpreprocess"
    ;;
  *gfortran*)   FFLAGS+=" -std=legacy -fdefault-real-8 -fdefault-double-8 -cpp"
    ;;
  *ifort*)      FFLAGS+=" -r8 -fpconstant -fpp"
    ;;
  *xlf*)        FFLAGS+=" -qfixed -qrealsize=8 -qdpc=e -qsuffix=cpp=f -qsuppress=cmpmsg"
    PPPO="-WF,"
    ;;
  *)  echo "ERROR: Cannot find a supported compiler!"
    echo ""
    NOCOMPILE=1
    read;;
esac

# initial clean-up
rm -f $CASENAME.f nek5000 2>/dev/null
rm -f ./obj/subuser.o 2>/dev/null

# Check if the compiler adds an underscore to external functions
UNDERSCORE=0
cat > test_underscore.f << _ACEOF
      subroutine underscore_test
        call test
      end
_ACEOF
$FCcomp -c test_underscore.f 2>&1 >/dev/null 
if nm test_underscore.o | grep -q test_ ; then
  UNDERSCORE=1
fi
\rm test_underscore.* 2>/dev/null

# Check if ld option is available
cat > _test.f << _ACEOF
      program test
    end
_ACEOF
$FCcomp -Wl,--allow-multiple-definition -o _test _test.f >/dev/null 2>&1 || true
\rm _test.f 2>/dev/null
if [ -f _test ]; then
  LDFLAGS+=" -Wl,--allow-multiple-definition"
  \rm _test 2>/dev/null
fi

# set preprocessor symbols
if [ $MPI -eq 0 ]; then
  PPLIST="${PPLIST} NOMPIIO"
else
  MPI=1
  PPLIST="${PPLIST} MPI"
fi

if [ $UNDERSCORE -ne 0 ]; then
  PPLIST="${PPLIST} UNDERSCORE"
fi 

if [ $PROFILING -ne 0 ]; then
  if [ $MPI -ne 0 ]; then
    PPLIST="${PPLIST} TIMER"
  else
    PPLIST="${PPLIST} TIMER"
  fi
fi

PPLIST="${PPLIST}"

# add user defined preprocessor symbols
CMT=0
MPIIO=1
for i in $PPLIST
do
  if [ "$i" == "CMTNEK" ]; then
    CMT=1
  fi
  if [ "$i" == "NOMPIIO" ]; then
    MPIIO=0
  fi
done

if echo $PPLIST | grep -q 'CVODE' ; then 
  CVODE=1
  if [ ! "$LIBPATH_CVODE" ]; then
    LIBPATH_CVODE=$SOURCE_ROOT/3rd_party/cvode/lib 
  fi
  if [ $MPI -eq 1 ]; then
    USR_LFLAGS+=" -L$LIBPATH_CVODE -lsundials_fcvode -lsundials_cvode"
    USR_LFLAGS+=" -lsundials_fnvecparallel -lsundials_nvecparallel" 
  else
    USR_LFLAGS+=" -L$LIBPATH_CVODE -lsundials_fcvode -lsundials_cvode"
    USR_LFLAGS+=" -lsundials_fnvecserial -lsundials_nvecserial" 
  fi
else
  CVODE=0
fi

if echo $PPLIST | grep -q 'HYPRE' ; then 
  HYPRE=1
else
  HYPRE=0
fi

MXM_USER="mxm_std.o"
if echo $PPLIST | grep -q 'BGQ' ; then 
  MXM_USER+=" mxm_bgq.o" 
fi

BLAS="blas.o dsygv.o"
if echo $PPLIST | grep -q 'VENDOR_BLAS' ; then 
  BLAS=" "
fi

# gslib build options
GSLIB_PREFIX="gslib_"
GSLIB_FPREFIX="fgslib_"
GSLIB_OPT+=" MPI=$MPI MPIIO=$MPIIO"
GSLIB_OPT+=" ADDUS=$UNDERSCORE USREXIT=1 BLAS=2"
GSLIB_OPT+=" PREFIX=$GSLIB_PREFIX FPREFIX=$GSLIB_FPREFIX"
GSLIB_IFLAGS="-DPREFIX=$GSLIB_PREFIX -DFPREFIX=$GSLIB_FPREFIX -DGLOBAL_LONG_LONG"
GSLIB_IFLAGS+=" -I$SOURCE_ROOT_GSLIB/gslib/src"
USR_LFLAGS+=" -L$SOURCE_ROOT_GSLIB/lib -lgs"

# tweak makefile template 
echo "generating makefile ..."
rm -rf $CASEDIR/Makefile 2>/dev/null

sed \
  -e "s|@FC@|$FC|" \
  -e "s|@CC@|$CC|" \
  -e "s|@FFLAGS@|$FFLAGS|" \
  -e "s|@CFLAGS@|$CFLAGS|" \
  -e "s|@LDFLAGS@|$LDFLAGS|" \
  -e "s|@PPPO@|$PPPO|" \
  -e "s|@PPS@|$PPLIST|" \
  -e "s|@MXM@|$MXM_USER|" \
  -e "s|@BLAS@|$BLAS|" \
  -e "s|@MPI@|$MPI|" \
  -e "s|@CVODE@|$CVODE|" \
  -e "s|@CMT@|$CMT|" \
  -e "s|@GSLIB_IFLAGS@|$GSLIB_IFLAGS|" \
  -e "s|@VISIT@|$VISIT|" \
  -e "s|@USR@|$USR|" \
  -e "s|@USR_LFLAGS@|$USR_LFLAGS|" \
  -e "s|@S@|${SOURCE_ROOT}/core|" \
  -e "s|@SCMT@|${SOURCE_ROOT}/core/cmt|" \
  $CARDINAL_DIR/config/Makefile.nek.in >.makefile

if [ $VISIT -ne 0 ]; then
  echo ""
  echo "Activate VisIt Sim Mode"

  if [ -z "$VISIT_INC" ]; then
    VISIT_INC=${VISIT_INSTALL}/libsim/V2/include
  fi
  echo "  VISIT_INC = ($VISIT_INC)"
  if [ -z "$VISIT_LIB" ]; then
    VISIT_LIB=${VISIT_INSTALL}/libsim/V2/lib
  fi
  echo "  VISIT_LIB = ($VISIT_LIB)"
  if [ -n "$VISIT_STOP" ]; then
    VISIT_STOP="VISIT_STOP"
    echo "  VISIT_STOP is defined. Nek5000 will wait for VisIt to Connect."
  fi

  sed -e "s/@VISIT@/${VISIT}/" \
    -e "s|@VISIT_IFLAGS@|-I${VISIT_INC}|" \
    -e "s|@VISIT_LFLAGS@|${VISIT_LIB} -lsimV2 -lsimV2f -ldl -lstdc++|" \
    -e "s/@PPS@/${PPLIST} VISIT ${VISIT_STOP}/" \
    .makefile > .makefile.tmp

  mv .makefile.tmp .makefile
  echo ""
else
  sed -e "s|@VISIT_IFLAGS@||" \
    -e "s|@VISIT_LFLAGS@||" \
    .makefile > .makefile.tmp
  mv .makefile.tmp .makefile
fi

if [ $HYPRE -ne 0 ]; then
  PATH_HYPRE=$SOURCE_ROOT/3rd_party/hypre
  USR_LFLAGS+=" -L$PATH_HYPRE/lib -lHYPRE"  
  HYPRE_INC="$PATH_HYPRE/include"
  sed -e "s|@HYPRE_IFLAGS@|-I${HYPRE_INC}|" \
    -e "s:@USR_LFLAGS@:$USR_LFLAGS:" \
    .makefile > .makefile.tmp

  mv .makefile.tmp .makefile
else
  sed -e "s|@HYPRE_IFLAGS@||" .makefile > .makefile.tmp
  mv .makefile.tmp .makefile
fi

if [ "$USR" != "" ]; then
  echo "###########################################################" >> $CASEDIR/Makefile
  echo "include makefile_usr.inc" >> .makefile
fi

if [ -f .makefile ]; then
  sed -e "1i\\
  ### makefile automatically created by makenek `date +"%m/%d/%Y %T"` ###" \
    -e "s|@CASEDIR@|${CASEDIR}|" \
    -e "s|@CASENAME@|${CASENAME}|" .makefile > $CASEDIR/Makefile
else
  echo "ERROR: Nek Makefile could not be created!"
  exit 1 
fi
\rm .makefile 2>/dev/null

# tweak leagacy SIZE file
if ! cat $CASEDIR/SIZE | grep -q 'SIZE.inc' ; then
  if ! cat $CASEDIR/SIZE | grep -qi 'optlevel' ; then
    echo >>$CASEDIR/SIZE
    echo 'c automatically added by makenek' >>$CASEDIR/SIZE
    echo '      integer optlevel,loglevel' >>$CASEDIR/SIZE
    echo '      common /lolevels/ optlevel,loglevel' >>$CASEDIR/SIZE
  fi
  if ! cat $CASEDIR/SIZE | grep -qi 'lxo' ; then
    echo >>$CASEDIR/SIZE
    echo 'c automatically added by makenek' >>$CASEDIR/SIZE
    echo '      parameter(lxo   = lx1) ! max output grid size (lxo>=lx1)' >>$CASEDIR/SIZE
  fi
  if ! cat $CASEDIR/SIZE | grep -qi 'ax1' ; then
    echo >>$CASEDIR/SIZE
    echo 'c automatically added by makenek' >>$CASEDIR/SIZE
    echo '      integer ax1,ay1,az1,ax2,ay2,az2' >> $CASEDIR/SIZE
    echo '      parameter (ax1=lx1,ay1=ly1,az1=lz1,ax2=lx2,ay2=ly2,az2=lz2) ! running averages' >> $CASEDIR/SIZE 
  fi
  if ! cat $CASEDIR/SIZE | grep -qi 'lys=lxs' ; then
    cat $CASEDIR/SIZE | grep -iv lxs > $CASEDIR/SIZE.x; mv $CASEDIR/SIZE.x $CASEDIR/SIZE  # Clean existing $CASEDIR/SIZE file of old version
    echo >>$CASEDIR/SIZE
    echo 'c automatically added by makenek' >>$CASEDIR/SIZE
    echo '      parameter (lxs=1,lys=lxs,lzs=(lxs-1)*(ldim-2)+1) !New Pressure Preconditioner' >> $CASEDIR/SIZE

  fi
  if ! cat $CASEDIR/SIZE | grep -qi 'lcvx1' ; then
    echo >>$CASEDIR/SIZE
    echo 'c automatically added by makenek' >>$CASEDIR/SIZE
    echo '      integer lcvx1,lcvy1,lcvz1,lcvelt' >> $CASEDIR/SIZE
    echo '      parameter (lcvx1=1,lcvy1=1,lcvz1=1,lcvelt=1) ! cvode arrays' >> $CASEDIR/SIZE
  fi
  if ! cat $CASEDIR/SIZE | grep -qi 'lfdm' ; then
    echo >>$CASEDIR/SIZE
    echo 'c automatically added by makenek' >>$CASEDIR/SIZE
    echo '      parameter (lfdm=0)  ! == 1 for fast diagonalization method' >> $CASEDIR/SIZE
  fi
  if ! cat $CASEDIR/SIZE | grep -qi 'nsessmax' ; then
    echo >>$CASEDIR/SIZE
    echo 'c automatically added by makenek' >>$CASEDIR/SIZE
    echo '      parameter (nsessmax=1)  ! max sessions to NEKNEK' >> $CASEDIR/SIZE
  fi
  if ! cat $CASEDIR/SIZE | grep -qi 'nmaxl_nn' ; then
    echo >>$CASEDIR/SIZE
    echo 'c automatically added by makenek' >>$CASEDIR/SIZE
    echo '      parameter (nmaxl_nn=' >> $CASEDIR/SIZE
    echo '     $          min(1+(nsessmax-1)*2*ldim*lxz*lelt,2*ldim*lxz*lelt))' >>$CASEDIR/SIZE
  fi
  if ! cat $CASEDIR/SIZE | grep -qi 'nfldmax_nn' ; then
    echo >>$CASEDIR/SIZE
    echo 'c automatically added by makenek' >>$CASEDIR/SIZE
    echo '      parameter (nfldmax_nn=' >> $CASEDIR/SIZE
    echo '     $          min(1+(nsessmax-1)*(ldim+1+ldimt),ldim+1+ldimt))' >>$CASEDIR/SIZE
  fi
  if ! cat $CASEDIR/SIZE | grep -qi 'nio' ; then
    echo >>$CASEDIR/SIZE
    echo 'c automatically added by makenek' >>$CASEDIR/SIZE
    echo '      common/IOFLAG/nio  ! for logfile verbosity control' >> $CASEDIR/SIZE
  fi

  if ! cat $CASEDIR/SIZE | grep -qi 'toteq' ; then
    if [ $CMT -ne 0 ]; then
      echo >>$CASEDIR/SIZE
      echo 'c automatically added by makenek' >>$CASEDIR/SIZE
      echo '      integer toteq' >> $CASEDIR/SIZE
      echo '      parameter(toteq = 5  ) ! Number of conserved variables '  >>$CASEDIR/SIZE
    fi
  fi
fi

# tweak all SIZE file versions
if ! cat $CASEDIR/SIZE | grep -qi 'ldimt_proj' ; then
  echo >>$CASEDIR/SIZE
  echo 'c automatically added by makenek' >>$CASEDIR/SIZE
  echo '      integer ldimt_proj' >>$CASEDIR/SIZE
  echo '      parameter(ldimt_proj = 1) ! max auxiliary fields residual projection ' >>$CASEDIR/SIZE
fi

export FC
export CC
export NEK_WARN 
export MPI
export MPIO
export FFLAGS
export CFLAGS
export UNDERSCORE
export GSLIB_OPT


if [ $NOCOMPILE -ne 0 ]; then
  exit 0
fi

declare -a varlist=(
"SOURCE_ROOT"
"FC"
"FFLAGS"
"CC"
"CFLAGS"
"PPLIST"
)
if [ -f .state ]; then
  for i in "${varlist[@]}"
  do
    if ! cat .state | grep -q "$i=${!i}" ; then
      echo "configuration change detected, forcing full rebuild"
      rm -rf .state
      while true; do
        read -p "do you want to rebuild all 3rd party software? [N]" yn
        case $yn in
          [Yy]* )
            clean_3rd_party;
            break ;;
          * )
            break ;;
        esac
      done  
      make clean;
      break
    fi
  done ;
fi

rm -rf .state
for i in "${varlist[@]}"
do
  echo "$i=${!i}" >> .state
done ;

exit 0
