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
	-DCMAKE_INSTALL_LIBDIR=lib \
	-DCMAKE_INSTALL_MESSAGE=LAZY \
	-DXDG_ENABLE_MOAB=OFF \
	-DXDG_ENABLE_LIBMESH=ON \
	-DXDG_LINK_MPI=ON \
	-DXDG_ENABLE_EMBREE=ON \
	-DXDG_BUILD_TESTS=OFF \
	-DXDG_BUILD_TOOLS=OFF \
	$(XDG_DIR)

build_xdg: | $(XDG_BUILDDIR)/Makefile
	# Fix an issue where different version FMT libraries collide.
	# This is very crude and should be fixed in OpenMC at some point.
	rm -rf $(CONTRIB_INSTALL_DIR)/lib/cmake/fmt \
				 $(CONTRIB_INSTALL_DIR)/lib/pkgconfig/fmt.pc \
				 $(CONTRIB_INSTALL_DIR)/lib/libfmt.a
	make VERBOSE=1 -C $(XDG_BUILDDIR) install

cleanall_xdg: | $(XDG_BUILDDIR)/Makefile
	make -C $(XDG_BUILDDIR) clean

clobber_xdg:
	rm -rf $(XDG_LIB) $(XDG_BUILDDIR) $(XDG_INSTALL_DIR)

cleanall: cleanall_xdg

clobberall: clobber_xdg

.PHONY: build_xdg cleanall_xdg clobber_xdg
