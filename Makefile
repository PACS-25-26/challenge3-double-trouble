CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Iinclude
LDFLAGS = -lmuparser

TARGET = jacobi_serial
SOURCES = src/main_serial.cpp src/JacobiSerial.cpp
OBJECTS = $(SOURCES:src/%.cpp=obj/%.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS)

obj/%.o: src/%.cpp | obj
	$(CXX) $(CXXFLAGS) -c -o $@ $<

obj:
	mkdir -p obj

clean:
	rm -rf obj $(TARGET)

.PHONY: all clean