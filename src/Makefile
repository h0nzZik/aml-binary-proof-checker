# https://stackoverflow.com/a/64068109/6209703
makeFileDir := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

BUILD_DIR := ${makeFileDir}/.build
$(shell mkdir -p ${BUILD_DIR})


CFLAGS := -std=c11 -Wall -Wextra -pedantic

.PHONY: default
default: $(BUILD_DIR)/checker $(BUILD_DIR)/test

$(BUILD_DIR)/check.o: check.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/test: $(BUILD_DIR)/check.o test.c
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/checker: $(BUILD_DIR)/check.o main.c
	$(CC) $(CFLAGS) $^ -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

INSTALL_PREFIX ?= /usr
.PHONY: install
install:
	install ./build/checker $(INSTALL_PREFIX)/bin


