CC := gcc

OBJ_DIR := build
BIN_DIR := bin

PRIVATE_SRC_DIR := src
SHARED_SRC_DIR := ../src
SRC_DIRS := $(PRIVATE_SRC_DIR) $(SHARED_SRC_DIR)

PRIVATE_INC := include
SHARED_INC := ../include
INCLUDE := $(PRIVATE_INC) $(SHARED_INC)

LD_LIB := mysqlclient
LDFLAGS := -l$(LD_LIB)
CFLAGS := -Wall -O3 $(addprefix -I, $(INCLUDE))

# Scan all .c file
SRCS := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c))

# Convert all .c file name to .o
OBJS := $(patsubst %.c, $(OBJ_DIR)/%.o, $(notdir $(SRCS)))

TARGET := $(BIN_DIR)/db-server

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# 搜尋 .c 的檔案必須在當前路徑或 SRC_DIRS，這樣 %.c 才能匹配
vpath %.c $(SRC_DIRS)

clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean

