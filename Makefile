# See README for instructions on how to use this Makefile

PRODUCTNAME = AttrbuteUtils

# The output objects dir
OBJDIR = ./out
BUILDDIR = ./build

OUTFILENAME = $(PRODUCTNAME).so
OUTFILEPATH = $(OBJDIR)/$(OUTFILENAME)

RESOURCESOPDIR = $(BUILDDIR)/Ops
RESOURCESNODEDIR = $(BUILDDIR)/Plugins
RESOURCESMACRODIR = $(BUILDDIR)/Macros

# Plugin sources and includes
PLUGIN_SRC = $(KATANA_HOME)/plugin_apis/src
SOURCES = $(shell find src -name \*.cpp)

# Directly suck in the FnAttribute/FnGeolibOp/FnGeolibUtil/pystring cpp files
SOURCES += $(shell find $(PLUGIN_SRC)/FnAttribute -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnGeolib/op -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnGeolib/util -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnGeolibServices/client -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/FnPluginManager -name \*.cpp)
SOURCES += $(shell find $(PLUGIN_SRC)/pystring -name \*.cpp)
SOURCES += $(PLUGIN_SRC)/FnPluginSystem/FnPlugin.cpp
SOURCES += $(shell find $(PLUGIN_SRC)/FnAsset -name \*.cpp)

INCLUDES = -I$(KATANA_HOME)/plugin_apis/include -Iinclude


CFLAGS=-ffloat-store
CXXFLAGS=-Wall -Wextra -Wshadow -Wconversion -Wcast-qual -fPIC -DPIC
CXXFLAGS=-fPIC -DPIC

# Initialize FnAsset API hosts for this Op
CXXFLAGS += -DFNGEOLIBOP_INIT_FNASSET_HOSTS

# Object files and flags
OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(SOURCES))

LIBPATH = -L$(KATANA_HOME)/bin/Geolib3/internal/CEL
LIBS = -lCEL


# Targets:
all: resources

$(OUTFILEPATH): $(OBJS)
	@echo "  Compiling $(PRODUCTNAME)"
	$(CXX) $(CXXFLAGS) $(OBJS) $(LIBPATH) $(LIBS) -shared -o $(OUTFILEPATH) -Wl,-soname,$(OUTFILENAME)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p `dirname $@`
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

resources: $(OUTFILEPATH)
	@mkdir -p $(RESOURCESOPDIR)
	@mkdir -p $(RESOURCESNODEDIR)
	cp $(OUTFILEPATH) $(RESOURCESOPDIR)
	cp nodes/*.py $(RESOURCESNODEDIR)

clean:
	@echo "  Cleaning $(PRODUCTNAME)"
	@rm -rf $(OBJDIR)
	@rm -rf $(BUILDDIR)

