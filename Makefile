# This project requires C++14. Suitable compilers include 
# any of these macros on cs340.cs.uwindsor.ca:
CXX = g++-7.0.0-alpha20170319
#CXX = g++-6.3.0
#CXX = g++-5.4.0
#CXX = clang++-4.0

# The final program will be built with optimizations and no 
# debug information...
CXXOPTS = -g0 -O3 -march=native -flto -DNDEBUG

# Disable the previous macro and enable this one if no 
# optimizations and debug information is desired...
#CXXOPTS = -g -O0 -DDEBUG

# Regardless, every time a C++ file is built, ensure strict C++14
# compliance is set and all warnings are enabled...
CXXFLAGS = $(CXXOPTS) -Wall -Wextra -std=c++14 -pedantic

# The use of threads in C++ with GCC and clang require linking to
# the pthread library. Using Boost.ProgramOptions requires linking
# to its library as well.
CXXLDFLAGS = -lpthread -lboost_program_options

# Define an array macro of all source files...
SRCS = main.cxx types.cxx ga.cxx

# Define an array macro of all object files (based on SRCS)...
OBJS = $(SRCS:.cxx=.o)

# Define the final target as a macro...
EXE = ga

# Tell make that there is to be an implicit rule to generate
# a .o (object) target file from a .cxx (C++) source file...
.SUFFIXES:
.SUFFIXES: .cxx .o

# Define the .cxx => .o implicit rule.
# Notice this rule compiles the C++ program to a .o file.
# $@ is the name of the target file (e.g., main.o).
# $< is the name of the source file (e.g., main.cxx).
.cxx.o:
	$(CXX) -c $(CXXFLAGS) -o $@ $<

# Define the default rule.
# The default rule depends on all object files.
# The build step invokes the C++ compiler with all object
# files, CXXOPTS and CXXLDFLAGS to generate the EXE file.
all: $(OBJS)
	$(CXX) $(CXXOPTS) $(OBJS) -o $(EXE) $(CXXLDFLAGS)

# Define the clean rule to delete all intermediate object
# as well as the EXE files.
clean:
	rm -f $(EXE) $(OBJS)

tests: test-sequential test-parallel 
all-tests: test-sequential test-parallel test-parallel-smart

test-sequential: $(EXE)
	./$(EXE) --threads=1

test-parallel: $(EXE)
	./$(EXE) --threads=2

# The following sets threads to be equal to the number of cores listed in /proc/cpuinfo...
test-parallel-smart: $(EXE)
	@echo "Number of processors (to be set to the number of threads): "`cat /proc/cpuinfo | grep processor | cut -d: -f 2- | sed -e 's/ //g' | sort -n | tail -n 1`
	./$(EXE) --threads=`cat /proc/cpuinfo | grep processor | cut -d: -f 2- | sed -e 's/ //g' | sort -n | tail -n 1`
	
