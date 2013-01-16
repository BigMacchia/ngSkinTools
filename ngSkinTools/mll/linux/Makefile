# ==========================================================================
# Copyright 2010 Viktoras Makauskas. All rights reserved.
# ==========================================================================




MAYA_LOCATION_2011 = /usr/autodesk/maya2011-x64
MAYA_LOCATION_2009 = /usr/autodesk/maya2009-x64

MAYA_VERSION = 2011

MAYA_LOCATION = $(MAYA_LOCATION_2011)

ifeq ($(MAYA_VERSION),2009)
	MAYA_LOCATION = $(MAYA_LOCATION_2009)
endif 

CC412 = /usr/local/gcc42/bin/gcc42
C++412 = /usr/local/gcc42/bin/g++42

CC = $(CC412)
C++ = $(C++412)
#include $(MAYA_LOCATION)/devkit/plug-ins/buildconfig
include mac-buildconfig
CC = $(CC412)
C++ = $(C++412)



# output folder and plugin filename
OUTPUT_DIR = output/Release ($(MAYA_VERSION)) Linux64
OUTPUT_DIR_OBJ = $(OUTPUT_DIR)/obj
PLUGIN_FILENAME = ngSkinTools.so

# all cpp files
CPP_FILES := $(wildcard *.cpp)
OBJ_FILENAMES := $(patsubst %.cpp,%.o,$(CPP_FILES))


# compiles all cpp files into OUTPUT_DIR_OBJ
%.o: %.cpp

	@mkdir -p "$(OUTPUT_DIR_OBJ)"
	@$(C++) -c $(INCLUDES) $(C++FLAGS) -o "$(OUTPUT_DIR_OBJ)/$@" "$<"

# compiles plugin
$(PLUGIN_FILENAME): $(OBJ_FILENAMES)

	@$(LD) -o "$(OUTPUT_DIR)/$@" $(foreach f,$^,"$(OUTPUT_DIR_OBJ)/$(f)") $(LIBS) -lOpenMayaAnim -lOpenMayaUI -IPHLPAPI
	@echo "created plugin in $(OUTPUT_DIR)/$@"


#compiles all plugins: dummy target
plugins: $(PLUGIN_FILENAME) 
