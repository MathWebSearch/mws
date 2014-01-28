all: build/cmake_done_config
	@cd build && make $@

clean:
	@rm -rf build

%: build/cmake_done_config
	@cd build && make $@

config: build/cmake_done_config
	@ccmake build/

build/cmake_done_config:
	@mkdir -p build
	@cd build && cmake ..
	@touch build/cmake_done_config

.PHONY: all clean config
