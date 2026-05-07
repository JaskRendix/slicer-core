BUILD_DIR := build

all: configure build-all

configure:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake ..

build-all:
	cd $(BUILD_DIR) && make -j

example:
	cd $(BUILD_DIR) && ./example

test:
	cd $(BUILD_DIR) && ctest --output-on-failure

clean:
	rm -rf $(BUILD_DIR)

format:
	find include src -name "*.h" -o -name "*.cpp" | xargs clang-format -i
