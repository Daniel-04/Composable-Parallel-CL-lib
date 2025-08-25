BUILD ?= release
TYPE ?= static
ifeq ($(filter debug,$(MAKECMDGOALS)),debug)
	BUILD := debug
endif
ifeq ($(filter release,$(MAKECMDGOALS)),release)
	BUILD := release
endif
ifeq ($(filter shared,$(MAKECMDGOALS)),shared)
	TYPE := shared
endif
ifeq ($(filter static,$(MAKECMDGOALS)),static)
	TYPE := static
endif

TARGET_NAME := clutils
BUILD_DIR := ./build
SRC_DIR := ./src
SRCS := $(shell find $(SRC_DIR) -name "*.c")
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

CC := gcc
AR := ar
EXTRA_INC_DIRS ?=
INC_DIRS := $(shell find $(SRC_DIR) -type d) $(EXTRA_INC_DIRS)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))
LDFLAGS ?=
LDLIBS := -lOpenCL
EXTRA_LDLIBS ?=
CLBLAST_AVAILABLE := $(shell ld -lclblast --verbose >/dev/null 2>&1 && echo yes || echo no)
ifeq ($(CLBLAST_AVAILABLE),yes)
	EXTRA_LDLIBS += -lclblast
endif
EXTRA_LDLIBS += -lm
CFLAGS := -Wall -Wextra
ifeq ($(BUILD),debug)
	CFLAGS += -g3
else ifeq ($(BUILD),release)
	CFLAGS += -O3
endif
ifeq ($(TYPE),static)
	TARGET := $(BUILD_DIR)/$(TARGET_NAME).a
	LDLIBS += $(TARGET)
else ifeq ($(TYPE),shared)
	TARGET := $(BUILD_DIR)/lib$(TARGET_NAME).so
	CFLAGS += -fPIC
	LDFLAGS += -shared
	LDLIBS += -L$(BUILD_DIR) -l$(TARGET_NAME)
endif

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(dir $@)
ifeq ($(TYPE),static)
	$(AR) rcs $@ $^
else ifeq ($(TYPE),shared)
	$(CC) $(LDFLAGS) -o $@ $^
endif

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC_FLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)

debug release static shared: all
	@true

%: %.c all
	$(CC) $(CFLAGS) $(EXTRA_LDLIBS) $(LDLIBS) $(INC_FLAGS) $< -o $@ $(LDLIBS)

%/: all
	@$(MAKE) $(patsubst %.c,%,$(shell find $* -name '*.c'))

docs:
	doxygen Doxyfile

.PHONY: all clean debug release static shared
