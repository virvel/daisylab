# Project Name
TARGET = daisylab

# Sources
CPP_SOURCES = daisylab.cpp dsp-headers/dsp/wavetable.cpp dsp-headers/dsp/interpolation.cpp

# Library Locations
LIBDAISY_DIR = ../DaisyExamples/libDaisy/
DAISYSP_DIR = ../DaisyExamples/DaisySP/

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile
