##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
ifeq ($(USE_OPT),)	
  USE_OPT = -O2 -ggdb -fomit-frame-pointer -falign-functions=16
  USE_OPT += -nodefaultlibs -lc -lgcc -lm
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = 
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -fno-rtti
endif

# Enable this if you want the linker to remove unused code and data
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# Linker extra options here.
ifeq ($(USE_LDOPT),)
  USE_LDOPT = 
endif

# Enable this if you want link time optimizations (LTO)
ifeq ($(USE_LTO),)
  USE_LTO = no
endif

# If enabled, this option allows to compile the application in THUMB mode.
ifeq ($(USE_THUMB),)
  USE_THUMB = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

# If enabled, this option makes the build process faster by not compiling
# modules not used in the current configuration.
ifeq ($(USE_SMART_BUILD),)
  USE_SMART_BUILD = yes
endif

#
# Build global options
##############################################################################
USE_CPPOPT += -std=c++14 -fno-rtti -fno-exceptions -fno-threadsafe-statics
USE_COPT += -std=c99

##############################################################################
# Architecture or project specific options
#

# Stack size to be allocated to the Cortex-M process stack. This stack is
# the stack used by the main() thread.
ifeq ($(USE_PROCESS_STACKSIZE),)
  USE_PROCESS_STACKSIZE = 0x400
endif

# Stack size to the allocated to the Cortex-M main/exceptions stack. This
# stack is used for processing interrupts and exceptions.
ifeq ($(USE_EXCEPTIONS_STACKSIZE),)
  USE_EXCEPTIONS_STACKSIZE = 0x400
endif

# Enables the use of FPU (no, softfp, hard).
ifeq ($(USE_FPU),)
  USE_FPU = no
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, sources and paths
#

# Define project name here
PROJECT = uavcan_adapter

# Imported source files and paths
CHIBIOS = modules/ChibiOS
UAVCAN = modules/libuavcan
# Startup files.
include $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk/startup_stm32f1xx.mk
# HAL-OSAL files (optional).
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/hal/ports/STM32/STM32F1xx/platform_f105_f107.mk
include $(CHIBIOS)/os/hal/osal/rt/osal.mk
# RTOS files (optional).
include $(CHIBIOS)/os/rt/rt.mk
include $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/mk/port_v7m.mk
include $(CHIBIOS)/os/various/cpp_wrappers/chcpp.mk
include $(CHIBIOS)/os/various/shell/shell.mk
include $(CHIBIOS)/os/hal/lib/streams/streams.mk

# Define linker script file here
LDSCRIPT= ld.ld

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(STARTUPSRC) \
       $(KERNSRC) \
       $(PORTSRC) \
       $(OSALSRC) \
       $(HALSRC) \
       $(PLATFORMSRC) \
       $(CHIBIOS)/os/various/syscalls.c \
       src/board/board.c \
       src/sys/exceptionvectors.c
#       $(SHELLSRC) \
#       $(STREAMSSRC) \

# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC += $(CHCPPSRC)
CPPSRC += $(shell find src -type f -name '*.cpp')

HW_VERSION_MAJOR = 1
FW_VERSION_MAJOR = 1
FW_VERSION_MINOR = 0
GIT_HASH = $(shell git rev-parse --short HEAD)

BOOTLOADER_SIZE = 32768
DDEFS += -DCORTEX_VTOR_INIT=$(BOOTLOADER_SIZE)            \
         -DCRT1_AREAS_NUMBER=0
		 
UDEFS += -DUAVCAN_STM32_CHIBIOS=1 \
		 -DUAVCAN_STM32_TIMER_NUMBER=6 \
		 -DUAVCAN_STM32_NUM_IFACES=1 \
		 -DUAVCAN_CPP_VERSION=UAVCAN_CPP11 \
		 -DGIT_HASH=0x$(GIT_HASH) \
		 -DFW_VERSION_MAJOR=$(FW_VERSION_MAJOR) \
		 -DFW_VERSION_MINOR=$(FW_VERSION_MINOR) \
		 -DRELEASE_BUILD=1
#		 -DDEBUG_BUILD=1 \
#		 -DDISABLE_WATCHDOG=1
		 
#UDEFS += -DSHELL_CONFIG_FILE
		 
include $(UAVCAN)/libuavcan/include.mk
CPPSRC += $(LIBUAVCAN_SRC)
UINCDIR += $(LIBUAVCAN_INC)

include $(UAVCAN)/libuavcan_drivers/stm32/driver/include.mk
CPPSRC += $(LIBUAVCAN_STM32_SRC)
UINCDIR += $(LIBUAVCAN_STM32_INC)

# Invoke DSDL compiler and add its default output directory to the include search path
$(info $(shell python $(LIBUAVCAN_DSDLC) $(UAVCAN_DSDL_DIR)))
UINCDIR += dsdlc_generated      # This is where the generated headers are stored by default

# C sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACSRC =

# C++ sources to be compiled in ARM mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
ACPPSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCSRC =

# C sources to be compiled in THUMB mode regardless of the global setting.
# NOTE: Mixing ARM and THUMB mode enables the -mthumb-interwork compiler
#       option that results in lower performance and larger code size.
TCPPSRC =

# List ASM source files here
ASMSRC =
ASMXSRC = $(STARTUPASM) $(PORTASM) $(OSALASM)

INCDIR = src src/board $(CHIBIOS)/os/license \
         $(STARTUPINC) $(KERNINC) $(PORTINC) $(OSALINC) \
         $(HALINC) $(PLATFORMINC) $(BOARDINC) $(TESTINC) \
         $(CHIBIOS)/os/various $(CHCPPINC) $(SHELLINC) $(STREAMSINC) \
         src/os src/config src/sys
         
#UINCDIR += src src/sys

#
# Project, sources and paths
##############################################################################

##############################################################################
# Compiler settings
#

MCU  = cortex-m3

#TRGT = arm-elf-
TRGT = arm-none-eabi-
CC   = $(TRGT)gcc
CPPC = $(TRGT)g++
# Enable loading with g++ only if you need C++ runtime support.
# NOTE: You can use C++ even without C++ support if you are careful. C++
#       runtime support makes code size explode.
#LD   = $(TRGT)gcc
LD   = $(TRGT)g++
CP   = $(TRGT)objcopy
AS   = $(TRGT)gcc -x assembler-with-cpp
AR   = $(TRGT)ar
OD   = $(TRGT)objdump
SZ   = $(TRGT)size
HEX  = $(CP) -O ihex
BIN  = $(CP) -O binary

# ARM-specific options here
AOPT =

# THUMB-specific options here
TOPT = -mthumb -DTHUMB

# Define C warning options here
CWARN = -Wall -Wextra -Wundef -Wstrict-prototypes

# Define C++ warning options here
CPPWARN = -Wall -Wextra -Wundef

#
# Compiler settings
##############################################################################

##############################################################################
# Start of user section
#

# Define ASM defines here
UADEFS =

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

#
# End of user defines
##############################################################################

RULESPATH = $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC
include $(RULESPATH)/rules.mk

HW_VERSION_MAJOR_MINOR := 1.0#$(HAL_VERSION).0
FW_VERSION_MAJOR_MINOR_VCS_HASH := $(FW_VERSION_MAJOR).$(FW_VERSION_MINOR).$(GIT_HASH)
COMPOUND_IMAGE_FILE := $(PROJECT)-$(HW_VERSION_MAJOR_MINOR)-$(FW_VERSION_MAJOR_MINOR_VCS_HASH).compound.bin
APPLICATION_IMAGE_FILE := $(PROJECT)-$(HW_VERSION_MAJOR_MINOR)-$(FW_VERSION_MAJOR_MINOR_VCS_HASH).application.bin

BOOTLOADER_IMAGE := ../bootloader/build/adapter_bootloader.bin

.PHONY: binaries
POST_MAKE_ALL_RULE_HOOK:
	# Removing previous build outputs that could use a different git hash
	rm -rf build/*.application.bin build/*.compound.bin

	# Generating compound image with embedded bootloader
	cd build && dd if=/dev/zero bs=$(BOOTLOADER_SIZE) count=1 | tr "\000" "\377" >padded_bootloader.tmp.bin
	cd build && dd if=$(BOOTLOADER_IMAGE) of=padded_bootloader.tmp.bin conv=notrunc
	#cd build && cat padded_bootloader.tmp.bin $(PROJECT).bin >$(COMPOUND_IMAGE_FILE)

	# Generating the signed image for the bootloader
	cd build && python2 ../tools/make_boot_descriptor.py $(PROJECT).bin $(PROJECT) $(HW_VERSION_MAJOR_MINOR) \
	                                                           --also-patch-descriptor-in=$(PROJECT).elf -v

	cd build && cat padded_bootloader.tmp.bin $(APPLICATION_IMAGE_FILE) >$(COMPOUND_IMAGE_FILE)
	
	# Injecting the bootloader into the final ELF
	cd build && $(CP) --add-section bootloader=$(BOOTLOADER_IMAGE)   \
	                                        --set-section-flags bootloader=load,alloc      \
	                                        --change-section-address bootloader=0x08000000 \
	                                        $(PROJECT).elf compound.elf

	# Removing temporary files
	cd build && rm -f $(PROJECT).bin $(PROJECT).elf *.hex *.tmp.bin

