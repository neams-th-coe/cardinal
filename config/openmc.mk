OPENMC_OBJDIR := $(CURDIR)/obj_openmc
OPENMC_LIBDIR := $(CURDIR)/lib
OPENMC_LIB := $(OPENMC_LIBDIR)/libopenmc.so

$(OPENMC_LIB):
	mkdir -p $(OPENMC_OBJDIR)
	mkdir -p $(LIBMESH_DIR)/include/contrib
	cd $(OPENMC_OBJDIR) && \
	PKG_CONFIG_PATH=$(LIBMESH_DIR)/lib/pkgconfig/ \
	cmake -Dlibmesh=ON -DCMAKE_EXE_LINKER_FLAGS="$(libmesh_LIBS)" -DCMAKE_SHARED_LINKER_FLAGS="$(libmesh_LIBS)" -DCMAKE_INSTALL_PREFIX=$(CURDIR) $(OPENMC_DIR)
	make VERBOSE=1 -C $(OPENMC_OBJDIR) install

clean_openmc:
	make -C $(OPENMC_OBJDIR) clean

libopenmc: $(OPENMC_LIB)

.PHONY: libopenmc clean_openmc
