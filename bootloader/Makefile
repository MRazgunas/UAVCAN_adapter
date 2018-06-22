##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

ifeq ($(HAL_VERSION),)
	HAL_VERSION = 1
endif

APPLICATION_OFFSET = 32768
BL_VERSION_MAJOR = 1
BL_VERSION_MINOR = 0
GIT_HASH = $(shell git rev-parse --short HEAD)
HW_VERSION_MAJOR = HAL_VERSION
PRODUCT_NAME_STRING = \"org.mr.uavcan.adapter.bl.$(BL_VERSION_MAJOR).$(BL_VERSION_MINOR)\"

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = -Os -ggdb -fomit-frame-pointer -falign-functions=16
 # USE_OPT += -nodefaultlibs
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
  USE_LTO = yes
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
PROJECT = adapter_bootloader

# Imported source files and paths
CHIBIOS = modules/ChibiOS
UAVCAN = modules/libcanard
# Startup files.
include $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk/startup_stm32f1xx.mk
# HAL-OSAL files (optional).
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/hal/ports/STM32/STM32F1xx/platform.mk
include $(CHIBIOS)/os/hal/osal/rt/osal.mk
# RTOS files (optional).
include $(CHIBIOS)/os/rt/rt.mk
include $(CHIBIOS)/os/common/ports/ARMCMx/compilers/GCC/mk/port_v7m.mk
include $(CHIBIOS)/os/various/cpp_wrappers/chcpp.mk

# Define linker script file here
#LDSCRIPT= ld.ld
LDSCRIPT= $(STARTUPLD)/STM32F103xB.ld

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(STARTUPSRC) \
       $(KERNSRC) \
       $(PORTSRC) \
       $(OSALSRC) \
       $(HALSRC) \
       $(PLATFORMSRC) \
       $(CHIBIOS)/os/various/syscalls.c \
       $(UAVCAN)/canard.c \
       $(UAVCAN)/drivers/stm32/canard_stm32.c \
       $(CHIBIOS)/os/hal/lib/streams/chprintf.c

CSRC += $(shell find src -type f -name '*.c')

# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC += $(CHCPPSRC)
CPPSRC += $(shell find src -type f -name '*.cpp')
		 
UINCDIR += $(UAVCAN)/drivers/stm32/
UINCDIR += $(UAVCAN)

UDEFS += -DAPPLICATION_OFFSET=$(APPLICATION_OFFSET)
UDEFS += -DPRODUCT_NAME_STRING=$(PRODUCT_NAME_STRING)
UDEFS += -DBL_VERSION_MAJOR=$(BL_VERSION_MAJOR)
UDEFS += -DBL_VERSION_MINOR=$(BL_VERSION_MINOR)
UDEFS += -DGIT_HASH=$(GIT_HASH)
UDEFS += -DHW_VERSION_MAJOR=$(HW_VERSION_MAJOR)
#UDEFS += -DDEBUG_BUILD=1
UDEFS += -DRELEASE_BUILD=1
UDEFS += -DHAL_BRD_VERSION=$(HAL_VERSION)
#UDEFS += -DDISABLE_WATCHDOG=1
#CSRC += $(UAVCAN)/canard.c

# Invoke DSDL compiler and add its default output directory to the include search path
#$(info $(shell python $(LIBUAVCAN_DSDLC) $(UAVCAN_DSDL_DIR)))
#UINCDIR += dsdlc_generated      # This is where the generated headers are stored by default

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

INCDIR = src $(CHIBIOS)/os/license \
         $(STARTUPINC) $(KERNINC) $(PORTINC) $(OSALINC) \
         $(HALINC) $(PLATFORMINC) $(BOARDINC) $(TESTINC) \
         $(CHIBIOS)/os/various $(CHCPPINC) $(CHIBIOS)/os/hal/lib/streams
         
UINCDIR += src src/sys



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
LD   = $(TRGT)gcc
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


