CXX ?= g++
CXXFLAGS ?= -Wall -Wextra -g -O0 -no-pie

all: vtable_internals

vtable_internals: vtable_internals.cpp
	$(CXX) $(CXXFLAGS) vtable_internals.cpp -o vtable_internals

clean:
	rm -f vtable_internals
