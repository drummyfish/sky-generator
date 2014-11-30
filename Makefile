# Procedural texture generator project
# Miloslav Ciz, 2012

CXX=c++
CXXFLAGS=-pedantic -Wall -std=c++11 -g -O2 -MMD # -pg

SRCDIR=src
OBJFILES=$(SRCDIR)/main.o $(SRCDIR)/colorbuffer.o $(SRCDIR)/lodepng.o $(SRCDIR)/perlin.o $(SRCDIR)/raytracing.o


UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
BIN=skygen
else
BIN=skygen.exe
endif


.PHONY:all clean

all: skygen

$(BIN): $(OBJFILES)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f $(SRCDIR)/*.o $(SRCDIR)/*.d skygen

-include $(OBJFILES:.o=.d)
