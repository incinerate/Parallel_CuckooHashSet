#  CSE 375/475 Assignment #1
#  Fall 2017
#
#  Description: This makefile allows you to build the assignment by typing
#  'make'.  Note that typing "BITS=64 make" will build a 64-bit executable
#  instead of a 32-bit executable.
#
#  Note: It's worth understanding how this Makefile works
#

# This defaults bits to 32, but allows it to be overridden on the command
# line
BITS = 32

# Output directory
ODIR  = obj32
tmp  := $(shell mkdir -p $(ODIR))

# Basic compiler configuration and flags
CXX      = g++
CXXFLAGS = -lpthread -fgnu-tm
LDFLAGS	 = -m$(BITS)

# The basenames of the c++ files that this program uses
CXXFILES = test

# The executable we will build
TARGET = $(ODIR)/test

# Create the .o names from the CXXFILES
OFILES = $(patsubst %, $(ODIR)/%.o, $(CXXFILES))

# Create .d files to store dependency information, so that we don't need to
# clean every time before running make
DFILES = $(patsubst %.o, %.d, $(OFILES))

# Default rule builds the executable
all: $(TARGET)

# clean up everything by clobbering the output folder
clean:
	@echo cleaning up...
	@rm -rf $(ODIR)

# build an .o file from a .cc file
$(ODIR)/%: %.cc
	@echo [CXX] $< "-->" $@
	@$(CXX) $(CXXFLAGS) -o $@ $<

# Link rule for building the target from .o files
#$(TARGET): $(OFILES)
#	@echo [LD] $^ "-->" $@
#	@$(CXX) -o $@ $^

# Remember that 'all' and 'clean' aren't real targets
.PHONY: all clean

# Pull in all dependencies
-include $(DFILES)
