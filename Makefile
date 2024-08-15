BUILD_DIR = build
EXE = game

compile: build
	cmake --build $(BUILD_DIR) --config Release

build:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=on

clean:
	rm -rf $(BUILD_DIR)

run: compile
	cp -R config.txt $(BUILD_DIR)/bin
	cp -R assets $(BUILD_DIR)/bin
	./$(BUILD_DIR)/bin/$(EXE)

all: run

