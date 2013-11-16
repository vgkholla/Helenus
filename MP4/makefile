PROGRAM = P2P

INCLUDEDIRS = \
	-Iheaders/ \

LIBS =-lpthread -lcrypto

STD_LIBS =-lboost_serialization

CXXSOURCES = \
             ./Main.cxx \
             ./ConnectionHandler.cxx
CXXOBJECTS = $(CXXSOURCES:.cpp=.o)  # expands to list of object files
CXXFLAGS = $(INCLUDEDIRS)
CXX = g++ -fpermissive -g

LDFLAGS = $(LIBDIRS) $(LIBS) $(STD_LIBS)

all: $(PROGRAM)

$(PROGRAM): $(CXXOBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $(CXXOBJECTS) $(LDFLAGS)

#clean:
#	$(RM) -f $(CXXOBJECTS) $(PROGRAM)
