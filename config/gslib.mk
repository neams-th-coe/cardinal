# Info we want from petscvariables:
#   * CC: C compiler; used to detect MPI
#   * CC_FLAGS: Used to set GS_CFLAGS

GS_OBJDIR := $(CURDIR)/obj_gs
GS_LIBDIR := $(CURDIR)/lib
GS_INCLUDE_DIR := $(CURDIR)/include/gs

# ==================================================================
# Config info
# ==================================================================

# CC_FLAGS from petscvariables
GS_CFLAGS := $(CC_FLAGS) -I$(GS_INCLUDE_DIR)

GS_HEADER_MACROS := GLOBAL_LONG_LONG
GS_CPPFLAGS := -DGLOBAL_LONG_LONG -DUSE_USR_EXIT -DPREFIX=gslib_ -DFPREFIX=fgslib_

# If using MPI
ifneq (,$(findstring mpi,$(CC)))
GS_HEADER_MACROS += MPI
GS_CPPFLAGS += -DMPI
endif

# Discover Fortran name-mangling
HAVE_UNDERSCORE := $(shell cat $$PETSC_DIR/include/petscconf.h | sed -n 's/^\#define[[:blank:]]\{1,\}PETSC_HAVE_FORTRAN_UNDERSCORE[[:blank:]]\{1,\}\([[:digit:]]\{1,\}\)/\1/p')
ifeq (1,$(HAVE_UNDERSCORE))
GS_HEADER_MACROS += UNDERSCORE
GS_CPPFLAGS += -DUNDERSCORE
endif

# If using debugging
ifneq (,$(findstring -g, $(GS_CFLAGS)))
GS_CPPFLAGS += -DGSLIB_DEBUG
endif

# ==================================================================
# Objects
# ==================================================================

GS_CORE := \
	comm \
	crystal \
	fail \
	gs \
	gs_local \
	sarray_sort \
	sarray_transfer \
	sort \
	tensor

GS_XXT := sparse_cholesky xxt

GS_AMG := amg

GS_FWRAPPER := fcrs fcrystal findpts

GS_INTP := findpts_el_2 findpts_el_3 findpts_local lob_bnd obbox poly

GS_OBJ := $(addprefix $(GS_OBJDIR)/, $(addsuffix .o, $(GS_CORE) $(GS_XXT) $(GS_AMG) $(GS_FWRAPPER) $(GS_INTP)))
GS_LIB := $(GS_LIBDIR)/libgs.a
GS_CONFIG_H := $(GS_INCLUDE_DIR)/config.h

# ==================================================================
# Targets
# ==================================================================

$(GS_LIB): $(GS_OBJ) | $(GS_LIBDIR)
	$(AR) $(AR_FLAGS) $@ $?
	$(RANLIB) $@

$(GS_OBJ): $(GS_OBJDIR)/%.o : %.c $(GS_CONFIG_H) | $(GS_CONFIG_H) $(GS_OBJDIR)
	$(CC) $(GS_CPPFLAGS) $(GS_CFLAGS) -c $< -o $@

$(GS_CONFIG_H): $(PETSC_DIR)/lib/petsc/conf/petscvariables $(PETSC_DIR)/include/petscconf.h | $(GS_INCLUDE_DIR)
	@rm -rf $@
	@for var in $(GS_HEADER_MACROS); do \
	echo "#ifndef $${var}\n#define $${var}\n#endif\n" >> $@; \
	done
	@echo "#ifndef PREFIX\n#define PREFIX gslib_\n#endif\n" >> $@
	@echo "#ifndef FPREFIX\n#define FPREFIX fgslib_\n#endif\n" >> $@

$(GS_OBJDIR) $(GS_LIBDIR) $(GS_INCLUDE_DIR):
	@mkdir -p $@

clean_gs:
	@rm -rf $(GS_OBJDIR) $(GS_LIB) $(GS_INCLUDE_H)

libgs: $(GS_LIB)

.PHONY: libgs clean_gs




