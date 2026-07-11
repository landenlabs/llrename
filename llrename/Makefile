CXX = clang++
CXXFLAGS = -std=c++17 -I../llcommon

# define the C++ source files
SRCS = llrename.cpp dirscan.cpp widefiles.cpp ../llcommon/directory.cpp ../llcommon/parseutil.cpp ../llcommon/signals.cpp

OBJS = $(SRCS:.cpp=.o)

# define the executable file
MAIN = llrename

all: $(MAIN)


$(MAIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(MAIN) $(OBJS) # $(LFLAGS) $(LIBS)


clean:
	rm -rf *.o* ../llcommon/*.o $(MAIN)


#depend: $(SRCS)
#    makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
