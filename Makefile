.DEFAULT_GOAL := sketch

# define constants and paths

ifeq ($(LIBMAPLE),)
	SRCROOT := .
else
	SRCROOT := $(LIBMAPLE)
endif
BUILD_PATH = build
LIBMAPLE_PATH := $(SRCROOT)/libmaple
WIRISH_PATH := $(SRCROOT)/wirish
SUPPORT_PATH := $(SRCROOT)/support
LDDIR := $(SUPPORT_PATH)/ld
MAKEDIR := $(SUPPORT_PATH)/make
BOARD_INCLUDE_DIR := $(MAKEDIR)/board-includes

BOARD ?= maple
MEMORY_TARGET ?= flash

include $(MAKEDIR)/target-config.mk

GLOBAL_CFLAGS   := -Os -g3 -gdwarf-2  -mcpu=cortex-m3 -mthumb -march=armv7-m \
                   -nostdlib -ffunction-sections -fdata-sections             \
                   -Wl,--gc-sections $(TARGET_FLAGS)
GLOBAL_CXXFLAGS := -fno-rtti -fno-exceptions -Wall $(TARGET_FLAGS)
GLOBAL_ASFLAGS  := -mcpu=cortex-m3 -march=armv7-m -mthumb                    \
                   -x assembler-with-cpp $(TARGET_FLAGS)
LDFLAGS  = $(TARGET_LDFLAGS) -mcpu=cortex-m3 -mthumb \
           -Xlinker --gc-sections \
           -Xassembler --march=armv7-m -Wall
#          -Xlinker --print-gc-sections \

include $(MAKEDIR)/build-rules.mk
include $(MAKEDIR)/build-templates.mk

LIBMAPLE_MODULES += $(SRCROOT)/libmaple
LIBMAPLE_MODULES += $(SRCROOT)/libmaple/usb   # The USB module is kept separate
LIBMAPLE_MODULES += $(LIBMAPLE_MODULE_SERIES) # STM32 series submodule in libmaple
LIBMAPLE_MODULES += $(SRCROOT)/wirish

LIBMAPLE_MODULES += $(SRCROOT)/libraries/Servo
LIBMAPLE_MODULES += $(SRCROOT)/libraries/Wire

$(foreach m,$(LIBMAPLE_MODULES),$(eval $(call LIBMAPLE_MODULE_template,$(m))))

# Trinity-specific targets

$(BUILD_PATH)/trinity.o: trinity.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(LIBMAPLE_INCLUDES) $(WIRISH_INCLUDES) \
		-I$(SRCROOT)/libraries/Servo -I$(SRCROOT)/libraries/Wire -o $@ -c $<

$(BUILD_PATH)/robot.o: robot.cpp
	$(CXX) $(CFLAGS) $(CXXFLAGS) $(LIBMAPLE_INCLUDES) $(WIRISH_INCLUDES) -o $@ -c $<

# Upload Targets

$(BUILD_PATH)/$(BOARD).elf: $(BUILDDIRS) $(TGT_BIN) $(BUILD_PATH)/trinity.o
	$(SILENT_LD) $(CXX) $(LDFLAGS) -o $@ $(TGT_BIN) $(BUILD_PATH)/trinity.o -Wl,-Map,$(BUILD_PATH)/$(BOARD).map

$(BUILD_PATH)/$(BOARD).bin: $(BUILD_PATH)/$(BOARD).elf
	$(SILENT_OBJCOPY) $(OBJCOPY) -v -Obinary $(BUILD_PATH)/$(BOARD).elf $@ 1>/dev/null
	$(SILENT_DISAS) $(DISAS) -d $(BUILD_PATH)/$(BOARD).elf > $(BUILD_PATH)/$(BOARD).disas
	@echo " "
	@echo "Object file sizes:"
	@find $(BUILD_PATH) -iname *.o | xargs $(SIZE) -t > $(BUILD_PATH)/$(BOARD).sizes
	@cat $(BUILD_PATH)/$(BOARD).sizes
	@echo " "
	@echo "Final Size:"
	@$(SIZE) $<
	@echo $(MEMORY_TARGET) > $(BUILD_PATH)/build-type

$(BUILDDIRS):
	@mkdir -p $@

MSG_INFO:
	@echo "================================================================================"
	@echo ""
	@echo "  Build info:"
	@echo "     BOARD:          " $(BOARD)
	@echo "     MCU:            " $(MCU)
	@echo "     MEMORY_TARGET:  " $(MEMORY_TARGET)
	@echo ""
	@echo "  See 'make help' for all possible targets"
	@echo ""
	@echo "================================================================================"
	@echo ""

#
# Maple upload section
#

.PHONY: install sketch clean

# Target upload commands
# USB ID for DFU upload -- FIXME: do something smarter with this
BOARD_USB_VENDOR_ID  := 1EAF
BOARD_USB_PRODUCT_ID := 0003
UPLOAD_ram   := $(SUPPORT_PATH)/scripts/reset.py && \
	sleep 1                  && \
	$(DFU) -a0 -d $(BOARD_USB_VENDOR_ID):$(BOARD_USB_PRODUCT_ID) -D $(BUILD_PATH)/$(BOARD).bin -R
UPLOAD_flash := $(SUPPORT_PATH)/scripts/reset.py && \
	sleep 1                  && \
	$(DFU) -a1 -d $(BOARD_USB_VENDOR_ID):$(BOARD_USB_PRODUCT_ID) -D $(BUILD_PATH)/$(BOARD).bin -R

# Conditionally upload to whatever the last build was
install: INSTALL_TARGET = $(shell cat $(BUILD_PATH)/build-type 2>/dev/null)
install: $(BUILD_PATH)/$(BOARD).bin
	@echo "Install target:" $(INSTALL_TARGET)
	$(UPLOAD_$(INSTALL_TARGET))

# Force a rebuild if the target changed
PREV_BUILD_TYPE = $(shell cat $(BUILD_PATH)/build-type 2>/dev/null)
build-check:
ifneq ($(PREV_BUILD_TYPE), $(MEMORY_TARGET))
	$(shell rm -rf $(BUILD_PATH))
endif

sketch: build-check MSG_INFO $(BUILD_PATH)/$(BOARD).bin

clean:
	rm -rf build

