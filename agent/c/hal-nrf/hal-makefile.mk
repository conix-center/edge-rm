PROJECT_NAME = $(shell basename "$(realpath ./)")
CURRENT_DIR = $(notdir $(shell pwd))

# Configurations
NRF_IC = nrf52840
SDK_VERSION = 16
SOFTDEVICE_MODEL = blank
USE_THREAD = 1
PROTO_DIR = ../../proto
PROTO_DIR = ./hal-nrf/permamote/docs/gateway

# Source and header files
APP_HEADER_PATHS += .
APP_SOURCE_PATHS += .
APP_SOURCE_PATHS += ./hal-nrf
APP_SOURCES += $(notdir $(wildcard ./hal-nrf/*.c))
APP_SOURCES += $(notdir $(wildcard ./*.c))
APP_SOURCES += $(notdir $(wildcard ./*.proto))
APP_SOURCES += $(notdir $(wildcard ./hal-nrf/permamote/docs/gateway/*.proto))

NRF_BASE_DIR ?= ./hal-nrf/permamote/software/nrf52x-base
#LINKER_SCRIPT = $(NRF_BASE_DIR)/make/ld/gcc_nrf52840_dfu_blank_0_256_1024.ld

# Include board Makefile (if any)
include $(NRF_BASE_DIR)/../boards/permamote/Board.mk
# Include main Makefile
include $(NRF_BASE_DIR)/make/AppMakefile.mk
