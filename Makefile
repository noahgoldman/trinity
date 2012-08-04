# The different compiler tools available
CC       := arm-none-eabi-gcc
CXX      := arm-none-eabi-g++
LD       := arm-none-eabi-ld -v
AR       := arm-none-eabi-ar
AS       := arm-none-eabi-gcc
OBJCOPY  := arm-none-eabi-objcopy
DISAS    := arm-none-eabi-objdump
OBJDUMP  := arm-none-eabi-objdump
SIZE     := arm-none-eabi-size
DFU      := dfu-util

# Suppress annoying output unless V is set
ifndef V
   SILENT_CC       = @echo '  [CC]       ' $(@:$(BUILD_PATH)/%.o=%.c);
   SILENT_AS       = @echo '  [AS]       ' $(@:$(BUILD_PATH)/%.o=%.S);
   SILENT_CXX      = @echo '  [CXX]      ' $(@:$(BUILD_PATH)/%.o=%.cpp);
   SILENT_LD       = @echo '  [LD]       ' $(@F);
   SILENT_AR       = @echo '  [AR]       '
   SILENT_OBJCOPY  = @echo '  [OBJCOPY]  ' $(@F);
   SILENT_DISAS    = @echo '  [DISAS]    ' $(@:$(BUILD_PATH)/%.bin=%).disas;
   SILENT_OBJDUMP  = @echo '  [OBJDUMP]  ' $(OBJDUMP);
endif

# TARGET_FLAGS are to be passed while compiling, assembling, linking.
TARGET_FLAGS :=
# TARGET_LDFLAGS go to the linker
TARGET_LDFLAGS :=

# Configuration derived from $(MEMORY_TARGET)

LD_SCRIPT_PATH := $(LDDIR)/$(MEMORY_TARGET).ld

ifeq ($(MEMORY_TARGET), ram)
VECT_BASE_ADDR := VECT_TAB_RAM
endif
ifeq ($(MEMORY_TARGET), flash)
VECT_BASE_ADDR := VECT_TAB_FLASH
endif
ifeq ($(MEMORY_TARGET), jtag)
VECT_BASE_ADDR := VECT_TAB_BASE
endif

# Pull in the board configuration file here, so it can override the
# above.

include $(BOARD_INCLUDE_DIR)/$(BOARD).mk

# Configuration derived from $(BOARD).mk

LD_SERIES_PATH := $(LDDIR)/stm32/series/$(MCU_SERIES)
LD_MEM_PATH := $(LDDIR)/stm32/mem/$(LD_MEM_DIR)
ifeq ($(MCU_SERIES), stm32f1)
# Due to the Balkanization on F1, we need to specify the line when
# making linker decisions.
LD_SERIES_PATH := $(LD_SERIES_PATH)/$(MCU_F1_LINE)
endif

TARGET_LDFLAGS += -Xlinker -T$(LD_SCRIPT_PATH) \
                  -L $(LD_SERIES_PATH) \
                  -L $(LD_MEM_PATH) \
                  -L $(LDDIR)
TARGET_FLAGS += -DBOARD_$(BOARD) -DMCU_$(MCU) \
                -DERROR_LED_PORT=$(ERROR_LED_PORT) \
                -DERROR_LED_PIN=$(ERROR_LED_PIN) \
                -D$(VECT_BASE_ADDR)

LIBMAPLE_MODULE_SERIES := $(LIBMAPLE_PATH)/$(MCU_SERIES)

BUILD_PATH= build
SRCROOT := $(LIBMAPLE)
BUILDDIRS :=
TGT_BIN :=
TARGET_FLAGS :=

GLOBAL_CFLAGS := Os -g3 -gdwarf-2  -mcpu=cortex-m3 -mthumb -march=armv7-m \
		-nostdlib -ffunction-sections -fdata-sections         \
		-Wl,--gc-sections $(TARGET_FLAGS)
GLOBAL_CXXFLAGS := -fno-rtti -fno-exceptions -Wall $(TARGET_FLAGS)
GLOBAL_ASFLAGS := := -mcpu=cortex-m3 -march=armv7-m -mthumb            \
		-x assembler-with-cpp $(TARGET_FLAGS)
LDFLAGS = $(TARGET_LDFLAGS) -mcpu=cortex-m3 -mthumb \
		-Xlinker --gc-sections \
		-Xassembler --march=armv7-m -Wall

LIBMAPLE_MODULES += $(SRCROOT)/libmaple
LIBMAPLE_MODULES += $(SRCROOT)/libmaple/usb
LIBMAPLE_MODULES += $(
