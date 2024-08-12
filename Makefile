BUILD_DIR = build
EXE = game

compile: build
	cmake --build $(BUILD_DIR)

build:
	cmake -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=on

clean:
	rm -rf $(BUILD_DIR)

run: compile
	cp -R config.txt $(BUILD_DIR)/bin
	cp -R fonts $(BUILD_DIR)/bin
	./$(BUILD_DIR)/bin/$(EXE)

all: run

