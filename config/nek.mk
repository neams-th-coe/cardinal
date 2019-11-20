# ===========================================================================
# Compiler flags
# ===========================================================================

NEK_OBJDIR := $(CURDIR)/obj_nek
NEK_LIBDIR := $(CURDIR)/lib

NEK_PPLIST := MPI TIMER VENDOR_BLAS

HAVE_UNDERSCORE := $(shell cat $$PETSC_DIR/$(PETSC_ARCH)/include/petscconf.h | sed -n 's/^\#define[[:blank:]]\{1,\}PETSC_HAVE_FORTRAN_UNDERSCORE[[:blank:]]\{1,\}\([[:digit:]]\{1,\}\)/\1/p')
ifeq (1,$(HAVE_UNDERSCORE))
NEK_PPLIST += UNDERSCORE
endif

# CC_FLAGS and FC_FLAGS from petscvariables
CFLAGS     := $(CFLAGS) $(CC_FLAGS)
CPPFLAGS   := $(CPPFLAGS) $(patsubst %,-D%,$(NEK_PPLIST)) -I$(NEK_CASEDIR) -I$(NEK_DIR)/core $(FFLAGS) $(PETSC_CC_INCLUDES)
FFLAGS     := $(FFLAGS) -std=legacy -fdefault-real-8 -fdefault-double-8 -cpp -I$(NEK_CASEDIR) -I$(NEK_DIR)/core $(FC_FLAGS) $(PETSC_FC_INCLUDES)
F_CPPFLAGS := $(patsubst %,$(FC_DEFINE_FLAG)%,$(NEK_PPLIST))
F_LDFLAGS  := $(LDFLAGS) $(FC_LINKER_FLAGS) -L$(NEK_LIBDIR) -L.
F_LIBS     := -lgs $(LIBS) $(BLASLAPACK_LIB)

# ===========================================================================
# Objects
# ===========================================================================

NEK_C_CORE := byte chelpers dictionary finiparser iniparser nek_comm

NEK_F_CORE := bdry byte_mpi calcz coef conduct \
 connect1 connect2 convect cvode_driver \
 drive1 drive2 dssum edgec eigsolv fast3d fasts \
 gauss genbox genxyz gfdm_op gfdm_par gfdm_solve gfldr \
 gmres hmholtz hpf hsmg ic induct intp_usr \
 makeq makeq_aux map2 math mvmesh mxm_wrapper navier0 \
 navier1 navier2 navier3 navier4 navier5 navier6 navier7 \
 navier8 nek_in_situ papi perturb plan4 plan5 \
 planx postpro prepost reader_par reader_re2 reader_rea \
 speclib ssolv subs1 subs2 vprops

# Always use Nek mxm (TODO: support hardware-specific MXM?)
NEK_MXM := mxm_std

# Placeholder for unsupported feature
NEK_CMT_CORE :=

# Non-core files
NEK_USR := nek_moose

# Always use MPI
NEK_COMM_MPI := comm_mpi

# Always use singlemesh (TODO: Option for multimesh)
NEK_NEKNEKO := singlmesh

# Placeholder for unsupported feature
NEK_VIST :=

# Always use vendor blas.  Just a placeholder
NEK_BLAS :=

NEK_F_OBJ := $(addprefix $(NEK_OBJDIR)/, $(addsuffix .o, $(NEK_F_CORE) $(NEK_MXM) $(NEK_CMT_CORE) $(NEK_USR) $(NEK_COMM_MPI) $(NEK_NEKNEKO) $(NEK_VISIT) $(NEK_BLAS) $(NEK_CASENAME)))

NEK_C_OBJ := $(addprefix $(NEK_OBJDIR)/, $(addsuffix .o, $(NEK_C_CORE)))

NEK_DRIVE_OBJ := $(NEK_OBJDIR)/drive.o

NEK_EXEC := nek5000_$(NEK_CASENAME)

NEK_LIB := $(NEK_LIBDIR)/libnek5000_$(NEK_CASENAME).a

SESSION_NAME := $(CURDIR)/SESSION.NAME

# ===========================================================================
# Rules/recipes
# ===========================================================================

$(NEK_EXEC) : $(NEK_DRIVE_OBJ) $(NEK_C_OBJ) $(NEK_F_OBJ)  | $(SESSION_NAME)
	$(FC) $(F_CPPFLAGS) $(FFLAGS) -o $@ $^ $(F_LDFLAGS) $(F_LIBS)

$(NEK_C_OBJ) : $(NEK_OBJDIR)/%.o : %.c | $(NEK_OBJDIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $< $(LDFLAGS) $(LIBS)

$(NEK_F_OBJ) $(NEK_DRIVE_OBJ) : $(NEK_OBJDIR)/%.o : %.f SIZE | $(NEK_OBJDIR)
	$(FC) $(F_CPPFLAGS) $(FFLAGS) -c -o $@ $< $(F_LDFLAGS) $(F_LIBS)

$(NEK_CASEDIR)/$(NEK_CASENAME).f: $(NEK_CASENAME).usr
	@echo "********************* MAKING USR *********************************"
	@cd $(NEK_CASEDIR) && CASENAME="$(NEK_CASENAME)" PPS="$(PPS)" $(NEK_DIR)/core/mkuserfile

$(NEK_OBJDIR)/$(NEK_CASENAME).o: $(NEK_CASEDIR)/$(NEK_CASENAME).f SIZE | $(NEK_OBJDIR)
	$(FC) $(F_CPPFLAGS) $(FFLAGS) -c -o $@ $< $(F_LDFLAGS) $(F_LIBS)

$(NEK_OBJDIR) $(NEK_LIBDIR):
	@mkdir -p $@

$(SESSION_NAME):
	@echo "$(NEK_CASENAME)" > $@
	@echo "$(NEK_CASEDIR)" >> $@

clean_nek:
	@rm -rf $(NEK_OBJDIR) libnek5000_$(NEK_CASENAME).a nek5000_$(NEK_CASENAME)

.PHONY: clean_nek
