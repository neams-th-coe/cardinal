NEKRS_BUILDDIR := $(CARDINAL_DIR)/build/nekrs
NEKRS_INSTALL_DIR := $(CONTRIB_INSTALL_DIR)

# By default, build nekrs
BUILD_NEKRS ?= yes

ifeq ($(BUILD_NEKRS),yes)

# These have the same names and meanings as in makenrs
NEKRS_CFLAGS := 
NEKRS_CXXFLAGS := 
NEKRS_FFLAGS := 
NEKRS_NEK5000_PPLIST := PARRSB DPROCMAP
NEKRS_LIBP_DEFINES := -DUSE_NULL_PROJECTION=1
USE_OCCA_MEM_BYTE_ALIGN := 64
OCCA_CXXFLAGS := -O2 -ftree-vectorize -funroll-loops -march=native -mtune=native

$(NEKRS_BUILDDIR)/Makefile: $(NEKRS_DIR)/CMakeLists.txt
	mkdir -p $(NEKRS_BUILDDIR)
	cd $(NEKRS_BUILDDIR) && \
	cmake -L -Wno-dev -Wfatal-errors \
	-DCMAKE_BUILD_TYPE="$(BUILD_TYPE)" \
	-DCMAKE_C_COMPILER="$(LIBMESH_CC_LIST)" \
	-DCMAKE_CXX_COMPILER="$(LIBMESH_CXX_LIST)" \
	-DCMAKE_Fortran_COMPILER="$(LIBMESH_F90_LIST)" \
	-DCMAKE_C_FLAGS="$(NEKRS_CFLAGS)" \
	-DCMAKE_CXX_FLAGS="$(NEKRS_CXXFLAGS)" \
	-DCMAKE_Fortran_FLAGS="$(NEKRS_FFLAGS)" \
	-DCMAKE_INSTALL_PREFIX="$(NEKRS_INSTALL_DIR)" \
	-DCMAKE_INSTALL_MESSAGE=LAZY \
	-DNEK5000_PPLIST="$(NEKRS_NEK5000_PPLIST)" \
	-DLIBP_DEFINES="$(NEKRS_LIBP_DEFINES)" \
	-DUSE_OCCA_MEM_BYTE_ALIGN="$(USE_OCCA_MEM_BYTE_ALIGN)" \
	-DOCCA_CXX="$(libmesh_CC)" \
	-DOCCA_CXXFLAGS="$(OCCA_CXXFLAGS)" \
	-DENABLE_CUDA="$(OCCA_CUDA_ENABLED)" \
	-DENABLE_OPENCL="$(OCCA_OPENCL_ENABLED)" \
	-DENABLE_HIP="$(OCCA_HIP_ENABLED)" \
	-DENABLE_AMGX="$(AMGX_ENABLED)" \
	$(NEKRS_DIR)

build_nekrs: | $(NEKRS_BUILDDIR)/Makefile
	make -C $(NEKRS_BUILDDIR) install

cleanall_nekrs: |  $(NEKRS_BUILDDIR)/Makefile
	make -C $(NEKRS_BUILDDIR) uninstall clean

clobber_nekrs:
	rm -rf $(NEKRS_LIB) $(NEKRS_BUILDDIR) $(NEKRS_INSTALL_DIR)

else # BUILD_NEKRS=no

NEKRS_INSTALL_DIR = $(NEKRS_DIR)
build_nekrs:
	@echo "Using pre-built nekrs from $(NEKRS_INSTALL_DIR)"

cleanall_nekrs:
	@echo "Not cleaning pre-built nekrs"

clobber_nekrs:
	@echo "Not clobbering pre-built nekrs"

endif # BUILD_NEKRS

NEKRS_INCLUDES := \
	-I$(NEKRS_INSTALL_DIR)/include \
	-I$(NEKRS_INSTALL_DIR)/include/bin \
	-I$(NEKRS_INSTALL_DIR)/include/lib \
	-I$(NEKRS_INSTALL_DIR)/include/core \
	-I$(NEKRS_INSTALL_DIR)/include/core/bdry \
	-I$(NEKRS_INSTALL_DIR)/include/core/ogs \
	-I$(NEKRS_INSTALL_DIR)/include/core/io \
	-I$(NEKRS_INSTALL_DIR)/include/core/linAlg \
	-I$(NEKRS_INSTALL_DIR)/include/mesh \
	-I$(NEKRS_INSTALL_DIR)/include/occa \
	-I$(NEKRS_INSTALL_DIR)/include/occa/core \
	-I$(NEKRS_INSTALL_DIR)/include/occa/c \
	-I$(NEKRS_INSTALL_DIR)/include/occa/loops \
	-I$(NEKRS_INSTALL_DIR)/include/occa/functional \
	-I$(NEKRS_INSTALL_DIR)/include/occa/dtype \
	-I$(NEKRS_INSTALL_DIR)/include/occa/types \
	-I$(NEKRS_INSTALL_DIR)/include/occa/defines \
	-I$(NEKRS_INSTALL_DIR)/include/occa/utils \
	-I$(NEKRS_INSTALL_DIR)/include/bench \
	-I$(NEKRS_INSTALL_DIR)/include/bdry \
	-I$(NEKRS_INSTALL_DIR)/include/elliptic \
	-I$(NEKRS_INSTALL_DIR)/include/plugins \
	-I$(NEKRS_INSTALL_DIR)/include/nekInterface \
	-I$(NEKRS_INSTALL_DIR)/include/nrs \
	-I$(NEKRS_INSTALL_DIR)/include/nrs/bdry \
	-I$(NEKRS_INSTALL_DIR)/include/nrs/cds \
	-I$(NEKRS_INSTALL_DIR)/include/nrs/cds/cvode \
	-I$(NEKRS_INSTALL_DIR)/include/nrs/neknek \
	-I$(NEKRS_INSTALL_DIR)/include/nrs/postProcessing \
	-I$(NEKRS_INSTALL_DIR)/include/pointInterpolation \
	-I$(NEKRS_INSTALL_DIR)/include/pointInterpolation/findpts/ \
	-I$(NEKRS_INSTALL_DIR)/include/udf \
	-I$(NEKRS_INSTALL_DIR)/include/utils \
	# Additional includes for nekrs <= 23.0
	-I$(NEKRS_INSTALL_DIR)/gatherScatter \
	-I$(NEKRS_INSTALL_DIR)/include/linAlg \
 	-I$(NEKRS_INSTALL_DIR)/include/libP/parAlmond

ADDITIONAL_CPPFLAGS += $(NEKRS_INCLUDES)

NEKRS_LIBDIR := $(NEKRS_INSTALL_DIR)/lib
NEKRS_LIB := $(NEKRS_LIBDIR)/libnekrs.so

NEKRS_ADDITIONAL_LIBS := -L$(NEKRS_LIBDIR) -lnekrs -locca $(CC_LINKER_SLFLAG)$(NEKRS_LIBDIR)
NEKRS_EXTERNAL_FLAGS := -L$(NEKRS_LIBDIR) -lnekrs $(CC_LINKER_SLFLAG)$(NEKRS_LIBDIR)

# cleanall and clobberall are from moose.mk
cleanall: cleanall_nekrs

clobberall: clobber_nekrs

.PHONY: build_nekrs cleanall_nekrs clobber_nekrs
