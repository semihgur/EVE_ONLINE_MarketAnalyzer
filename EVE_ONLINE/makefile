
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -Wpedantic

# Path to the RapidJSON header files (relative to the project root)
RAPIDJSON_INCLUDE = -I./lib/rapidjson

# libcurl settings (assuming system-wide installation)
CURL_INCLUDE = -I./lib/curl
CURL_LIB = -L./lib -lcurl

SFML_INCLUDE = -I./lib/SFML
SFML_LIB = -L./lib -lsfml-audio -lsfml-graphics -lsfml-network -lsfml-system -lsfml-window

# Target executables
TARGET = evemarketanalyzer jitaOrderSeeker_VolumeUnder300 jita_Item_ID_Getter

# Source files
SOURCES = evemarketanalyzer.cpp jitaOrderSeeker_VolumeUnder300.cpp jita_Item_ID_Getter.cpp

# Object files directory
OBJECT_DIR = ./Objects

# Source files directory
SRC_DIR = ./src

# Executables directory
EXECUTABLE_DIR = ./Executables

all: $(TARGET)

# Rule for object files
$(OBJECT_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(RAPIDJSON_INCLUDE) $(CURL_INCLUDE) -c -o $@ $<

# Rules for executables
evemarketanalyzer: $(OBJECT_DIR)/evemarketanalyzer.o
	$(CXX) $(CXXFLAGS) $(RAPIDJSON_INCLUDE) $(SFML_INCLUDE) $(CURL_INCLUDE) -o $(EXECUTABLE_DIR)/$@ $^ $(CURL_LIB) $(SFML_LIB)

jitaOrderSeeker_VolumeUnder300: $(OBJECT_DIR)/jitaOrderSeeker_VolumeUnder300.o
	$(CXX) $(CXXFLAGS) $(RAPIDJSON_INCLUDE) $(CURL_INCLUDE) -o $(EXECUTABLE_DIR)/$@ $^ $(CURL_LIB)

jita_Item_ID_Getter: $(OBJECT_DIR)/jita_Item_ID_Getter.o
	$(CXX) $(CXXFLAGS) $(RAPIDJSON_INCLUDE) $(CURL_INCLUDE) -o $(EXECUTABLE_DIR)/$@ $^ $(CURL_LIB)

clean:
	rm -f $(OBJECTS) $(EXECUTABLE_DIR)/*
