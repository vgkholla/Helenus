PROGRAM = P2P

INCLUDEDIRS = \
	-Iheaders/

LIBS = -lpthread

CXXSOURCES = \
             ./Main.cxx \
             ./ConnectionHandler.cxx
CXXOBJECTS = $(CXXSOURCES:.cpp=.o)  # expands to list of object files
CXXFLAGS = $(INCLUDEDIRS)
CXX = g++ -fpermissive

LDFLAGS = $(LIBDIRS) $(LIBS)

all: $(PROGRAM)

$(PROGRAM): $(CXXOBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(CXXOBJECTS) $(LDFLAGS)

#clean:
#	$(RM) -f $(CXXOBJECTS) $(PROGRAM)
