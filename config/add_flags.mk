ifeq ($(ENABLE_NEK), yes)
  libmesh_CXXFLAGS    += -DENABLE_NEK_COUPLING
endif

ifeq ($(ENABLE_OPENMC), yes)
  libmesh_CXXFLAGS    += -DENABLE_OPENMC_COUPLING
endif

ifeq ($(ENABLE_DAGMC), yes)
  libmesh_CXXFLAGS    += -DENABLE_DAGMC

  # this flag is used in OpenMC
  libmesh_CXXFLAGS    += -DOPENMC_DAGMC_ENABLED
endif

libmesh_CXXFLAGS += -DOPENMC_LIBMESH_ENABLED

ifneq ($(GRIFFIN_CONTENT),)
 libmesh_CXXFLAGS    += -DENABLE_GRIFFIN_COUPLING

 APPLICATION_DIR    := $(GRIFFIN_DIR)/isoxml
 APPLICATION_NAME   := isoxml
 include            $(FRAMEWORK_DIR)/app.mk
 #ADDITIONAL_DEPEND_LIBS += $(CURDIR)/isoxml/lib/libisoxml-$(METHOD).la

 APPLICATION_DIR    := $(GRIFFIN_DIR)/xs_generation
 APPLICATION_NAME   := mcc3
 include            $(APPLICATION_DIR)/mcc3.dep
 include            $(FRAMEWORK_DIR)/app.mk

 APPLICATION_DIR    := $(GRIFFIN_DIR)/radiation_transport
 APPLICATION_NAME   := radiation_transport
 include            $(FRAMEWORK_DIR)/app.mk

 APPLICATION_DIR     := $(GRIFFIN_DIR)
 APPLICATION_NAME    := griffin
 include             $(FRAMEWORK_DIR)/app.mk
endif

ifneq ($(BISON_CONTENT),)
  libmesh_CXXFLAGS    += -DENABLE_BISON_COUPLING
  APPLICATION_DIR     := $(BISON_DIR)
  APPLICATION_NAME    := bison
  include             $(FRAMEWORK_DIR)/app.mk
  include            $(IAPWS95_DIR)/libSBTL.mk
endif

ifneq ($(SAM_CONTENT),)
  libmesh_CXXFLAGS    += -DENABLE_SAM_COUPLING
  APPLICATION_DIR     := $(SAM_DIR)
  APPLICATION_NAME    := sam
  SOLID_MECHANICS     := yes
  include             $(FRAMEWORK_DIR)/app.mk
endif

ifneq ($(SOCKEYE_CONTENT),)
  libmesh_CXXFLAGS    += -DENABLE_SOCKEYE_COUPLING
  APPLICATION_DIR     := $(SOCKEYE_DIR)
  APPLICATION_NAME    := sockeye
  include             $(FRAMEWORK_DIR)/app.mk
endif

ifneq ($(SODIUM_CONTENT),)
  libmesh_CXXFLAGS    += -DENABLE_SODIUM
  APPLICATION_DIR     := $(SODIUM_DIR)
  APPLICATION_NAME    := sodium
  include             $(FRAMEWORK_DIR)/app.mk
  include             $(SODIUM_DIR)/libSodiumProperties.mk
endif

ifneq ($(POTASSIUM_CONTENT),)
  libmesh_CXXFLAGS    += -DENABLE_POTASSIUM
  APPLICATION_DIR     := $(POTASSIUM_DIR)
  APPLICATION_NAME    := potassium
  include             $(FRAMEWORK_DIR)/app.mk
  include             $(POTASSIUM_DIR)/libPotassiumProperties.mk
endif

ifneq ($(IAPWS95_CONTENT),)
  libmesh_CXXFLAGS    += -DENABLE_IAPWS95
  APPLICATION_DIR     := $(IAPWS95_DIR)
  APPLICATION_NAME    := iapws95
  include             $(FRAMEWORK_DIR)/app.mk
  include             $(IAPWS95_DIR)/libSBTL.mk
endif
