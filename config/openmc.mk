OPENMC_BUILDDIR := $(CARDINAL_DIR)/build/openmc
OPENMC_INSTALL_DIR := $(CONTRIB_INSTALL_DIR)

# By default, build openmc

BUILD_OPENMC ?= yes

ifeq ($(BUILD_OPENMC),yes)

$(OPENMC_BUILDDIR)/Makefile: build_dagmc | $(OPENMC_DIR)/CMakeLists.txt
	mkdir -p $(OPENMC_BUILDDIR)
	cd $(OPENMC_BUILDDIR) && \
	cmake -L \
	-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	-DOPENMC_USE_LIBMESH=ON \
	-DOPENMC_USE_MPI=ON \
	-DOPENMC_USE_DAGMC=$(ENABLE_DAGMC) \
	-DOPENMC_FORCE_VENDORED_LIBS=ON \
	-DDAGMC_DIR=$(DAGMC_DIR) \
	-DCMAKE_C_COMPILER="$(LIBMESH_CC_LIST)" \
	-DCMAKE_CXX_COMPILER="$(LIBMESH_CXX_LIST)" \
	-DCMAKE_Fortran_COMPILER="$(LIBMESH_F90_LIST)" \
	-DCMAKE_PREFIX_PATH=$(LIBMESH_DIR) \
	-DCMAKE_INSTALL_PREFIX=$(OPENMC_INSTALL_DIR) \
	-DCMAKE_INSTALL_LIBDIR=$(OPENMC_LIBDIR) \
	-DCMAKE_CXX_STANDARD=17 \
	-DCMAKE_CXX_FLAGS="-Wno-error=reorder" \
	-DCMAKE_INSTALL_MESSAGE=LAZY \
	$(OPENMC_DIR)

build_openmc: | $(OPENMC_BUILDDIR)/Makefile
	make VERBOSE=1 -C $(OPENMC_BUILDDIR) install

cleanall_openmc: | $(OPENMC_BUILDDIR)/Makefile
	make -C $(OPENMC_BUILDDIR) uninstall clean

clobber_openmc:
	rm -rf $(OPENMC_LIB) $(OPENMC_BUILDDIR) $(OPENMC_INSTALL_DIR)

else # BUILD_OPENMC=no

OPENMC_INSTALL_DIR = $(OPENMC_DIR)
build_openmc:
	@echo "Using pre-built openmc from $(OPENMC_INSTALL_DIR)"

cleanall_openmc:
	@echo "Not cleaning pre-built openmc"

clobber_openmc:
	@echo "Not clobbering pre-built openmc"

endif # BUILD_OPENMC

OPENMC_INCLUDES := -I$(OPENMC_INSTALL_DIR)/include
ADDITIONAL_CPPFLAGS += $(HDF5_INCLUDES) $(OPENMC_INCLUDES)
OPENMC_LIBDIR := $(OPENMC_INSTALL_DIR)/lib
OPENMC_LIB := $(OPENMC_LIBDIR)/libopenmc.so

OPENMC_ADDITIONAL_LIBS := -L$(OPENMC_LIBDIR) -lopenmc -lhdf5_hl
OPENMC_EXTERNAL_FLAGS := -L$(OPENMC_LIBDIR) -L$(HDF5_LIBDIR) -lopenmc
ifeq ($(ENABLE_DAGMC), ON)
  OPENMC_ADDITIONAL_LIBS += -ldagmc -lMOAB
  OPENMC_EXTERNAL_FLAGS += -ldagmc -lMOAB
  ifeq ($(ENABLE_DOUBLE_DOWN), ON)
    OPENMC_ADDITIONAL_LIBS += -lembree4 -ldd
    OPENMC_EXTERNAL_FLAGS += -lembree4 -ldd
  endif
endif
OPENMC_ADDITIONAL_LIBS += $(CC_LINKER_SLFLAG)$(OPENMC_LIBDIR)
OPENMC_EXTERNAL_FLAGS += $(CC_LINKER_SLFLAG)$(OPENMC_LIBDIR) $(CC_LINKER_SLFLAG)$(HDF5_LIBDIR)

cleanall: cleanall_openmc

clobberall: clobber_openmc

.PHONY: build_openmc cleanall_openmc clobber_openmc
