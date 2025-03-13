BOARD ?= beta
DEBUG ?= 1

BUILD_ROOT = build

TARGET = ceti-whale-tag

### Tools ###
RM := rm
MKDIR := mkdir
include print.mk

CROSS := arm-none-eabi-
CC := $(CROSS)gcc
CP := $(CROSS)objcopy
SZ := $(CROSS)size

### C Compilation settings ###
# Board specific 
ifeq ($(BOARD), alpha)
	CPU = -mcpu=cortex-m33
	FPU = -mfpu=fpv4-sp-d16
	FLOAT-ABI = -mfloat-abi=hard
	MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)
	C_DEFS += -DSTM32U5A5xx
	C_DEFS += --DHW_VERSION=0
endif

ifeq ($(BOARD), beta)
	CPU = -mcpu=cortex-m33
	FPU = -mfpu=fpv4-sp-d16
	FLOAT-ABI = -mfloat-abi=hard
	MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)
	C_DEFS += -DSTM32U595xx
	C_DEFS += -DHW_VERSION=1
endif

C_DEFS +=  \
-DFX_INCLUDE_USER_DEFINE_FILE \
-DUSE_HAL_DRIVER

ifeq ($(DEBUG), 1)
	COPT = -Og -g -gdwarf-2
	C_DEFS += -DDEBUG
else
	COPT = -O2
endif

# Generate dependency information
C_INCLUDES := $(addprefix -I,$(shell find board/$(BOARD) -type d \( -iname 'inc' -o -iname 'include' -o -iwholename '*/inc/legacy' -o -iname 'app' \) 2> /dev/null))
CFLAGS += $(MCU) $(C_DEFS) $(C_INCLUDES) $(COPT) -Wall -fdata-sections -ffunction-sections
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

# link script
LDSCRIPT = $(addprefix -T,$(shell find board/$(BOARD) -type f -iname '*_FLASH.ld'))

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs $(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections


### Files and Directories ###

# source files
C_SRCS = $(shell find board/$(BOARD) src -type f -iname '*.c' 2> /dev/null)
ASM_SRCS = $(shell find board/$(BOARD) src -type f -iname '*.s' 2> /dev/null)

# objects files
C_OBJS = $(addprefix $(BUILD_DIR)/,$(C_SRCS:.c=.c.o))
ASM_OBJS = $(addprefix $(BUILD_DIR)/,$(ASM_SRCS:.s=.s.o))
ALL_OBJS = $(C_OBJS) $(ASM_OBJS)

# Build folders
ifeq ($(DEBUG), 1)
	BUILD_DIR = $(BUILD_ROOT)/$(BOARD)/debug
else
	BUILD_DIR = $(BUILD_ROOT)/$(BOARD)/release
endif
C_BUILD_DIRS := $(sort $(dir $(C_OBJS)))
ASM_BUILD_DIRS := $(sort $(dir $(ASM_OBJS)))
ALL_DIRS := $(BUILD_DIR) $(C_BUILD_DIRS) $(ASM_BUILD_DIRS)


### Rules ###

# default target
all: $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex

# mkdirs
$(ALL_DIRS):
	$(call print1,Making Folder:,$@)
	@$(MKDIR) -p $@

# .s -> .o
$(BUILD_DIR)/%.s.o : %.s | $(ASM_BUILD_DIRS)
	$(call print2,Assembling:,$<,$@)
	@$(CC) -x assembler-with-cpp -c $(CFLAGS) $< -o $@

# .c -> .o
$(BUILD_DIR)/%.c.o : %.c | $(C_BUILD_DIRS)
	$(call print2,Compiling:,$<,$@)
	@$(CC) -c $(CFLAGS) $< -o $@ 

$(BUILD_DIR)/$(TARGET).elf: $(ALL_OBJS) | $(BUILD_DIR)
	$(call print1,Linking elf:,$@)
	@$(CC) $^ $(LDFLAGS) -o $@
	$(SZ) $@
	@$(PRINT) "\n$$(cat logo.ansi.txt)\n"	

$(BUILD_DIR)/$(TARGET).hex: $(BUILD_DIR)/$(TARGET).elf 
	$(call print1,Creating hex:,$@)
	@$(CP) -O ihex $< $@

$(BUILD_DIR)/$(TARGET).bin: $(BUILD_DIR)/$(TARGET).elf 
	$(call print1,Creating bin:,$@)
	@$(CP) -O binary -S $< $@

release: 
	@$(MAKE) DEBUG=0

test:
	$(call todo1, Implement Code Unit Tests)

flash: $(BUILD_DIR)/$(TARGET).elf
	$(call print0, Flashing via stlink)
	STM32_Programmer_CLI --connect port=swd --write $< --go

clean: 
	$(call print0, Cleaning build artifacts)
	@$(RM) -rf $(BUILD_ROOT)

DOCKER_IMAGE = stm32_build_img

build: $(DOCKER_IMAGE)
	$(call print0, Running make inside docker)
	docker run --rm \
		--user $(shell id -u):$(shell id -g) \
		--volume $(shell pwd):/ceti-firmware \
		$(DOCKER_IMAGE) \
			$(MAKE)

lint:
	docker run \
		-e RUN_LOCAL=true \
		-e VALIDATE_CLANG_FORMAT=true \
		-e LINTER_RULES_PATH=.github/linters \
		-e DEFAULT_BRANCH=main \
		-e FILTER_REGEX_EXCLUDE='.*(board|lib)/.*' \
		-v $(shell pwd)/.:/tmp/lint \
		--rm ghcr.io/super-linter/super-linter:latest

lint_fix:
	docker run \
		-e RUN_LOCAL=true \
		-e VALIDATE_CLANG_FORMAT=true \
		-e FIX_CLANG_FORMAT=true \
		-e LINTER_RULES_PATH=.github/linters \
		-e DEFAULT_BRANCH=main \
		-e FILTER_REGEX_EXCLUDE='.*(board|lib)/.*' \
		-v $(shell pwd)/.:/tmp/lint \
		--rm ghcr.io/super-linter/super-linter:latest

$(DOCKER_IMAGE): Dockerfile packages.txt
	$(call print0, Building docker image)
	docker build -t $(DOCKER_IMAGE) .

.PHONY: all \
	release \
	debug \
	docker \
	clean \
	flash \
	build \
	$(DOCKER_IMAGE)