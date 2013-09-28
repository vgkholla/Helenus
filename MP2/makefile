PROGRAM = P2P

INCLUDEDIRS = \
	-Iheaders/ \
        -I/home/aloktiagi/boost-trunk/

LIBS = -lpthread 

STD_LIBS = -L/home/aloktiagi/boost-trunk/bin.v2/libs/serialization/build/gcc-4.7/release/link-static/threading-multi -lboost_serialization

CXXSOURCES = \
             ./Main.cxx \
             ./ConnectionHandler.cxx
CXXOBJECTS = $(CXXSOURCES:.cpp=.o)  # expands to list of object files
CXXFLAGS = $(INCLUDEDIRS)
CXX = g++ -fpermissive -std=gnu++11 -g

LDFLAGS = $(LIBDIRS) $(LIBS) $(STD_LIBS)

all: $(PROGRAM)

$(PROGRAM): $(CXXOBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(CXXOBJECTS) $(LDFLAGS)

#clean:
#	$(RM) -f $(CXXOBJECTS) $(PROGRAM)
