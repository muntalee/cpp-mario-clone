CXX = g++
OUTPUT = out
PROJECT_DIR=$(shell pwd)

SRC =  src/main.cpp
SRC += $(wildcard external/imgui/*.cpp)
SRC += $(wildcard external/imgui-sfml/*.cpp)

OBJ_DIR = obj
OBJ = $(SRC:%.cpp=$(OBJ_DIR)/%.o)

# for our libraries
SFML_SRC_DIR = $(PROJECT_DIR)/external/SFML
FREETYPE_SRC_DIR = $(PROJECT_DIR)/external/freetype

# our compiler flags
CXXFLAGS = -std=c++20 -O3 -Wno-unused-result -Ibuild/include

# our linker flags
LDFLAGS = -Lbuild/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

# specific configs for macos
ifeq ($(shell uname -s), Darwin)
	LDFLAGS += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo
endif

# specific configs for linux
ifeq ($(shell uname -s), Linux)
	LDFLAGS += -ldl -lpthread -lGL
endif


.PHONY: all clean run

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
	$(CXX) -c $< -o $@ $(CXXFLAGS) $(LDFLAGS)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(OUTPUT) .cache .DS_Store build

run: all
	./$(OUTPUT)

