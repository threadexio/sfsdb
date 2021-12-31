
CXX ?=

CXXFLAGS ?=
CMFLAGS ?= 

#===============#

BUILD_DIR ?= ./build
BUILD_TYPE ?= Debug

#===============#

MAKEFLAGS += --no-print-directory

#===============#

BUILD_TESTS ?= n
ifeq ($(BUILD_TESTS), y)
	CMFLAGS += -DNIO_BUILD_TESTS:BOOL=ON -DRESP_BUILD_TESTS:BOOL=ON -DBUILD_TESTS:BOOL=ON
endif

.PHONY:
all: build

.PHONY:
.ONESHELL:
build: FORCE
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR)

	@export CPPFLAGS="$(CXXFLAGS) -Wall -Wextra"

	@cmake -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) $(CMFLAGS) ..

	@make

.PHONY:
clean:
	@-rm -rf $(BUILD_DIR)

FORCE: ;