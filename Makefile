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

LIBMAPLE_MODULES += 
