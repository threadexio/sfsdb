
CXX ?=

CXXFLAGS ?=
CMFLAGS ?= 

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

	@cmake -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) $(CMFLAGS) ..

	@make

.PHONY:
clean:
	@-rm -rf $(BUILD_DIR)