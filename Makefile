
CXX ?=

CXXFLAGS ?=

#===============#

BUILD_DIR ?= ./build
BUILD_TYPE ?= Debug

#===============#

.PHONY:
all: build

.PHONY:
.ONESHELL:
build:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR)

	@export CPPFLAGS="$(CXXFLAGS)"

	@cmake -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) ..

	@make

.PHONY:
clean:
	@-rm -rf $(BUILD_DIR)