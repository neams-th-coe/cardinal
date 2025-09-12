EMBREE_BUILDDIR := $(CARDINAL_DIR)/build/embree
EMBREE_INSTALL_DIR := $(CONTRIB_INSTALL_DIR)

# By default, build embree
BUILD_EMBREE ?= yes

ifeq ($(BUILD_EMBREE),yes)

$(EMBREE_BUILDDIR)/Makefile: $(EMBREE_DIR)/CMakeLists.txt
	mkdir -p $(EMBREE_BUILDDIR)
	cd $(EMBREE_BUILDDIR) && \
	cmake -L \
	-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	-DCMAKE_C_COMPILER="$(LIBMESH_CC_LIST)" \
	-DCMAKE_CXX_COMPILER="$(LIBMESH_CXX_LIST)" \
	-DCMAKE_Fortran_COMPILER="$(LIBMESH_F90_LIST)" \
	-DCMAKE_PREFIX_PATH=$(LIBMESH_DIR) \
	-DCMAKE_INSTALL_PREFIX=$(EMBREE_INSTALL_DIR) \
	-DCMAKE_INSTALL_LIBDIR=lib \
	-DCMAKE_INSTALL_MESSAGE=LAZY \
	-DEMBREE_TUTORIALS=OFF \
	-DEMBREE_TUTORIALS_GLFW=OFF \
	-DEMBREE_TASKING_SYSTEM=INTERNAL \
	-DEMBREE_IGNORE_CMAKE_CXX_FLAGS=OFF \
	$(EMBREE_DIR)

build_embree: | $(EMBREE_BUILDDIR)/Makefile
	make VERBOSE=1 -C $(EMBREE_BUILDDIR) install

cleanall_embree: | $(EMBREE_BUILDDIR)/Makefile
	make -C $(EMBREE_BUILDDIR) clean

clobber_embree:
	rm -rf $(EMBREE_LIB) $(EMBREE_BUILDDIR) $(EMBREE_INSTALL_DIR)

else # BUILD_EMBREE=no

EMBREE_INSTALL_DIR = $(EMBREE_DIR)
build_embree:
	@echo "Using pre-built embree from $(EMBREE_INSTALL_DIR)"

cleanall_embree:
	@echo "Not cleaning pre-built embree"

clobber_embree:
	@echo "Not clobbering pre-built embree"

endif # BUILD_EMBREE

cleanall: cleanall_embree

clobberall: clobber_embree

.PHONY: build_embree cleanall_embree clobber_embree
