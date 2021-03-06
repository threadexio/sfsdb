
CXX ?=

CXXFLAGS ?=
CMFLAGS ?= 

REQUIRED_CXXFLAGS := -Wall -Wextra

#===============#

BUILD_DIR ?= ./build
BUILD_TYPE ?= Debug

#===============#

MAKEFLAGS += --no-print-directory

#===============#

VERSION ?=
ifeq ($(VERSION),)
	VERSION := $(shell git log --format=%h -n1)
endif

BUILD_TESTS ?= n
ifeq ($(BUILD_TESTS), y)
	CMFLAGS += -DBUILD_TESTS:BOOL=ON
endif

.PHONY:
all: build

.PHONY:
.ONESHELL:
build: FORCE
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR)

	@cmake -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=ON \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_CXX_FLAGS="$(CXXFLAGS) $(REQUIRED_CXXFLAGS) -DVERSION='\"${VERSION}\"'" \
		$(CMFLAGS) \
		..

	@make

.PHONY:
clean:
	@-rm -rf $(BUILD_DIR)

.PHONY:
tests:
	./run_tests.sh

FORCE: ;