OPENMC_INSTALL_DIR := $(CONTRIB_INSTALL_DIR)
OPENMC_BUILDDIR := $(CARDINAL_DIR)/build/openmc
OPENMC_LIBDIR := $(CONTRIB_INSTALL_DIR)/lib
OPENMC_LIB := $(OPENMC_LIBDIR)/libopenmc.so

$(OPENMC_LIB):
	mkdir -p $(OPENMC_BUILDDIR)
	mkdir -p $(LIBMESH_DIR)/include/contrib
	cd $(OPENMC_BUILDDIR) && \
	cmake -L -Dlibmesh=ON -Doptimize=ON -DLIBMESH_DIR=$(LIBMESH_DIR) -DCMAKE_INSTALL_PREFIX=$(CONTRIB_INSTALL_DIR) $(OPENMC_DIR) && \
	cmake $(OPENMC_DIR) && make VERBOSE=1 -C $(OPENMC_BUILDDIR) install

clean_openmc:
	make -C $(OPENMC_BUILDDIR) clean

libopenmc: $(OPENMC_LIB)

.PHONY: libopenmc clean_openmc
