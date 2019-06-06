

# Set the environmental variable $MACOSX_DEPLOYMENT_TARGET to
# "10.9" in Apple OS X to compile for OS X 10.9 and later (for example,
# you can compile for OS X 10.9 computers even if you are using the 10.10
# version of the operating system).
ENV =
OS := $(shell uname -s)
ifeq ($(OS),Darwin)
   # Minimum MacOS Version for C++11 is 10.9:
   ENV = MACOSX_DEPLOYMENT_TARGET=10.9
endif

BINDIR    = bin
OBJDIR    = obj
SRCDIR    = src
TOOLDIR   = tools
INCDIR    = include
LIBDIR    = lib
LIBFILE   = humidi
AR        = ar
RANLIB    = ranlib

PREFLAGS  = -g $(CFLAGS) $(DEFINES) -I$(INCDIR) 
PREFLAGS += -O3 -Wall
PREFLAGS += -std=c++11

POSTFLAGS =

LIBFLAGS  = -L$(LIBDIR) -l$(LIBFILE)

ifeq ($(CXX),)
   COMPILER  = LANG=C $(ENV) g++ 
else 
   COMPILER  = LANG=C $(ENV) $(CXX) 
endif
COMPILER  = LANG=C $(ENV) g++ 

# setting up the directory paths to search for dependency files
vpath %.h   $(INCDIR):$(SRCDIR)
vpath %.cpp $(SRCDIR):$(INCDIR)
vpath %.o   $(OBJDIR)

# generating a list of the object files
OBJS = $(notdir $(patsubst %.cpp,%.o,$(wildcard $(SRCDIR)/*.cpp)))

# targets which don't actually refer to files
.PHONY: src include tools obj bin tests lib


all: library midi2score

midi2score:
	@echo [CC] $@
	$(COMPILER) $(PREFLAGS) tools/midi2score.cpp -o $(BINDIR)/$@ $(LIBDIR)/$(LIBFILE).a

library: makedirs $(OBJS)
	@-rm -f $(LIBDIR)/$(LIBFILE).a
	@$(AR) r $(LIBDIR)/$(LIBFILE).a $(addprefix $(OBJDIR)/, $(OBJS))
	@$(RANLIB) $(LIBDIR)/$(LIBFILE).a

makedirs:
	@-mkdir -p $(OBJDIR)
	@-mkdir -p $(LIBDIR)
	@-mkdir -p $(BINDIR)

%.o : %.cpp
	@echo [CC] $@
	@$(COMPILER) -c $(PREFLAGS) -o $(OBJDIR)/$(notdir $@) $(POSTFLAGS) $<

%:
	@echo 'if [ "$<" == "" ]; then $(MAKE) -f Makefile.programs $@; fi' | bash -s




update: update-humlib update-midifile

update-humlib:
	wget https://raw.githubusercontent.com/craigsapp/humlib/master/include/humlib.h -O include/humlib.h
	wget https://raw.githubusercontent.com/craigsapp/humlib/master/src/humlib.cpp -O src/humlib.cpp
	wget https://raw.githubusercontent.com/craigsapp/humlib/master/include/pugiconfig.hpp -O include/pugiconfig.hpp
	wget https://raw.githubusercontent.com/craigsapp/humlib/master/include/pugixml.hpp -O include/pugixml.hpp
	wget https://raw.githubusercontent.com/craigsapp/humlib/master/src/pugixml.cpp -O src/pugixml.cpp


update-midifile:
	wget https://raw.githubusercontent.com/craigsapp/midifile/master/include/MidiFile.h -O include/MidiFile.h
	wget https://raw.githubusercontent.com/craigsapp/midifile/master/include/MidiEventList.h -O include/MidiEventList.h
	wget https://raw.githubusercontent.com/craigsapp/midifile/master/include/MidiEvent.h -O include/MidiEvent.h
	wget https://raw.githubusercontent.com/craigsapp/midifile/master/include/MidiMessage.h -O include/MidiMessage.h
	wget https://raw.githubusercontent.com/craigsapp/midifile/master/include/Binasc.h -O include/Binasc.h
	wget https://raw.githubusercontent.com/craigsapp/midifile/master/src/MidiFile.cpp -O src/MidiFile.cpp
	wget https://raw.githubusercontent.com/craigsapp/midifile/master/src/MidiEventList.cpp -O src/MidiEventList.cpp
	wget https://raw.githubusercontent.com/craigsapp/midifile/master/src/MidiEvent.cpp -O src/MidiEvent.cpp
	wget https://raw.githubusercontent.com/craigsapp/midifile/master/src/MidiMessage.cpp -O src/MidiMessage.cpp
	wget https://raw.githubusercontent.com/craigsapp/midifile/master/src/Binasc.cpp -O src/Binasc.cpp
	wget https://raw.githubusercontent.com/pianoroll/midiroll/master/src/MidiRoll.cpp -O src/MidiRoll.cpp
	wget https://raw.githubusercontent.com/pianoroll/midiroll/master/include/MidiRoll.h -O include/MidiRoll.h
