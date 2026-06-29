$(XDG_BUILDDIR)/Makefile: build_moab build_embree | $(XDG_DIR)/CMakeLists.txt
	mkdir -p $(XDG_BUILDDIR)
	cd $(XDG_BUILDDIR) && \
	cmake -L \
	-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	-DCMAKE_C_COMPILER="$(LIBMESH_CC_LIST)" \
	-DCMAKE_CXX_COMPILER="$(LIBMESH_CXX_LIST)" \
	-DCMAKE_Fortran_COMPILER="$(LIBMESH_F90_LIST)" \
	-DCMAKE_PREFIX_PATH=$(LIBMESH_DIR) \
	-DCMAKE_INSTALL_PREFIX=$(XDG_INSTALL_DIR) \
	-DEMBREE_DIR=$(EMBREE_INSTALL_DIR) \
	-DMOAB_DIR=$(MOAB_INSTALL_DIR) \
	-DCMAKE_INSTALL_LIBDIR=lib \
	-DCMAKE_INSTALL_MESSAGE=LAZY \
	-DXDG_ENABLE_MOAB=$(ENABLE_DAGMC) \
	-DXDG_ENABLE_LIBMESH=ON \
	-DXDG_LINK_MPI=ON \
	-DXDG_ENABLE_EMBREE=ON \
	-DXDG_BUILD_TESTS=OFF \
	-DXDG_BUILD_TOOLS=OFF \
	$(XDG_DIR)

build_xdg: | $(XDG_BUILDDIR)/Makefile
	make VERBOSE=1 -C $(XDG_BUILDDIR) install

cleanall_xdg: | $(XDG_BUILDDIR)/Makefile
	make -C $(XDG_BUILDDIR) clean

clobber_xdg:
	rm -rf $(XDG_LIB) $(XDG_BUILDDIR) $(XDG_INSTALL_DIR)

cleanall: cleanall_xdg

clobberall: clobber_xdg

.PHONY: build_xdg cleanall_xdg clobber_xdg
