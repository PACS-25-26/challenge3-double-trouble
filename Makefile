CXX = mpic++
CXX_SERIAL = g++
CXXFLAGS = -std=c++17 -O3 -Wall -fopenmp -Iinclude -lmuparser
CXXFLAGS_SERIAL = -std=c++17 -O3 -Wall -Iinclude
LDFLAGS = -fopenmp -lmuparser
LDFLAGS_SERIAL = -lmuparser

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

# Parallel sources
SOURCES = $(filter-out $(SRC_DIR)/main_serial.cpp, $(wildcard $(SRC_DIR)/*.cpp))
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Serial sources
SOURCES_SERIAL = $(SRC_DIR)/main_serial.cpp $(SRC_DIR)/JacobiSerial.cpp
OBJECTS_SERIAL = $(SOURCES_SERIAL:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%_serial.o)

TARGET = jacobi_solver
TARGET_SERIAL = jacobi_serial

all: $(TARGET) $(TARGET_SERIAL)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $@ $^

$(TARGET_SERIAL): $(OBJECTS_SERIAL)
	$(CXX_SERIAL) $(LDFLAGS_SERIAL) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $

$(OBJ_DIR)/%_serial.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX_SERIAL) $(CXXFLAGS_SERIAL) -c -o $@ $

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(TARGET_SERIAL)

.PHONY: all clean