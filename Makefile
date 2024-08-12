CXX = clang++
OUTPUT = out

CXXFLAGS = -std=c++20 -O3 -Wno-unused-result
CXXFLAGS += -Ibuild/include -Iexternal/SFML/include -Iexternal/freetype/include
CXXFLAGS += -Iexternal/imgui -Iexternal/imgui-sfml

LDFLAGS = -Lbuild/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

PROJECT_DIR=$(shell pwd)
SRC = src/main.cpp $(wildcard external/imgui/*.cpp) $(wildcard external/imgui-sfml/*.cpp)
OBJ_DIR = obj
OBJ = $(SRC:%.cpp=$(OBJ_DIR)/%.o)

SFML_SRC_DIR = $(PROJECT_DIR)/external/SFML
FREETYPE_SRC_DIR = $(PROJECT_DIR)/external/freetype

.PHONY: all clean run freetype sfml

all: freetype sfml $(OBJ_DIR) $(OUTPUT)

freetype:
	@if [ ! -d "build/freetype" ]; then \
		mkdir -p build/freetype; \
	fi
	@cd build/freetype && cmake $(FREETYPE_SRC_DIR) -DCMAKE_INSTALL_PREFIX=$(PROJECT_DIR)/build
	@cd build/freetype && make
	@cd build/freetype && make install

sfml: freetype
	@if [ ! -d "build/sfml" ]; then \
		mkdir -p build/sfml; \
	fi
	@cd build/sfml && cmake $(SFML_SRC_DIR) -DCMAKE_INSTALL_PREFIX=$(PROJECT_DIR)/build -DFREETYPE_INCLUDE_DIR_freetype2=$(PROJECT_DIR)/build/include/freetype2 -DFREETYPE_INCLUDE_DIR_ft2build=$(PROJECT_DIR)/build/include/freetype2
	@cd build/sfml && make
	@cd build/sfml && make install

$(OUTPUT): $(OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) -c $< -o $@ $(CXXFLAGS)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(OUTPUT) build .cache .DS_Store

run: all
	./$(OUTPUT)

