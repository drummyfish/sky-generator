CXX=c++
CXXFLAGS=-pedantic -Wall -std=c++11 -g -O2 -MMD -fopenmp # -pg

SRCDIR=src
OBJFILES=$(SRCDIR)/main.o $(SRCDIR)/colorbuffer.o $(SRCDIR)/lodepng.o $(SRCDIR)/perlin.o $(SRCDIR)/raytracing.o $(SRCDIR)/skyrenderer.o

UNAME := $(shell uname)
ifeq ($(UNAME), Linux)
BIN=skygen
ANIMBIN=anim
else
BIN=skygen.exe
ANIMBIN=anim.exe
endif

.PHONY:all clean

all: $(BIN) $(ANIMBIN)

$(BIN): $(OBJFILES)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(ANIMBIN): $(SRCDIR)/anim.o $(SRCDIR)/colorbuffer.o $(SRCDIR)/lodepng.o $(SRCDIR)/perlin.o $(SRCDIR)/raytracing.o $(SRCDIR)/skyrenderer.o
	$(CXX) $(CXXFLAGS) -lSDL2 $^ -o $@

clean:
	rm -f $(SRCDIR)/*.o $(SRCDIR)/*.d $(BIN) $(ANIMBIN)

-include $(OBJFILES:.o=.d)
