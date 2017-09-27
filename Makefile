GXX=g++
CXXFLAGS=-std=c++11
PROGRAM=gilmore

all: main.o gilmore.o first_order_logic.o herbrand.o
	$(GXX) $(CXXFLAGS) $^ -o $(PROGRAM)

main.o: main.cpp
	$(GXX) $(CXXFLAGS) -c $< -o $@

first_order_logic.o: first_order_logic.cpp first_order_logic.h
	$(GXX) $(CXXFLAGS) -c $< -o $@

gilmore.o: gilmore.cpp gilmore.h
	$(GXX) $(CXXFLAGS) -c $< -o $@

herbrand.o: herbrand.cpp herbrand.h
	$(GXX) $(CXXFLAGS) -c $< -o $@


.PHONY: clean

clean: 
	rm -rf *.o $(PROGRAM)
