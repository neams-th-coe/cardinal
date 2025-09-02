MOAB_BUILDDIR := $(CARDINAL_DIR)/build/moab
MOAB_INSTALL_DIR := $(CONTRIB_INSTALL_DIR)

# By default, build moab
BUILD_MOAB ?= yes

ifeq ($(BUILD_MOAB),yes)

$(MOAB_BUILDDIR)/Makefile: $(MOAB_DIR)/CMakeLists.txt
	mkdir -p $(MOAB_BUILDDIR)
	cd $(MOAB_BUILDDIR) && \
	cmake -L \
	-DENABLE_MPI=ON \
	-DENABLE_HDF5=ON \
	-DENABLE_TESTING=OFF \
	-DENABLE_BLASLAPACK=OFF \
	-DEIGEN3_DIR=$(EIGEN3_DIR) \
	-DHDF5_DIR=$(HDF5_ROOT) \
	-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	-DCMAKE_C_COMPILER="$(LIBMESH_CC_LIST)" \
	-DCMAKE_CXX_COMPILER="$(LIBMESH_CXX_LIST)" \
	-DCMAKE_Fortran_COMPILER="$(LIBMESH_F90_LIST)" \
	-DCMAKE_PREFIX_PATH=$(LIBMESH_DIR) \
	-DCMAKE_INSTALL_PREFIX=$(MOAB_INSTALL_DIR) \
	-DCMAKE_INSTALL_RPATH="$(HDF5_ROOT)/lib;$(MOAB_INSTALL_DIR)/lib" \
	-DCMAKE_INSTALL_LIBDIR=lib \
	-DCMAKE_INSTALL_MESSAGE=LAZY \
	$(MOAB_DIR)

build_moab: | $(MOAB_BUILDDIR)/Makefile
	make VERBOSE=1 -C $(MOAB_BUILDDIR) install

cleanall_moab: | $(MOAB_BUILDDIR)/Makefile
	make -C $(MOAB_BUILDDIR) uninstall clean

clobber_moab:
	rm -rf $(MOAB_LIB) $(MOAB_BUILDDIR) $(MOAB_INSTALL_DIR)

else # BUILD_MOAB=no

MOAB_INSTALL_DIR = $(MOAB_DIR)
build_moab:
	@echo "Using pre-built moab from $(MOAB_INSTALL_DIR)"

cleanall_moab:
	@echo "Not cleaning pre-built moab"

clobber_moab:
	@echo "Not clobbering pre-built moab"

endif # BUILD_MOAB

cleanall: cleanall_moab

clobberall: clobber_moab

.PHONY: build_moab cleanall_moab clobber_moab
