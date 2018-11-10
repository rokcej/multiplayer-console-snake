# Flags
CC=gcc
CFLAGS=-Wall
LDFLAGS=
LDLIBS=-pthread -lm

# Parameters
SRC_DIR=src
BUILD_DIR=build

SERVER_EXEC=server
CLIENT_EXEC=client

SERVER_SRCS=server.c socket_comm.c
CLIENT_SRCS=client.c socket_comm.c

# Files
SERVER_EXEC:=$(BUILD_DIR)/$(SERVER_EXEC)
CLIENT_EXEC:=$(BUILD_DIR)/$(CLIENT_EXEC)

SERVER_SRCS:=$(patsubst %,$(SRC_DIR)/%,$(SERVER_SRCS))
CLIENT_SRCS:=$(patsubst %,$(SRC_DIR)/%,$(CLIENT_SRCS))

SERVER_OBJS:=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SERVER_SRCS))
CLIENT_OBJS:=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(CLIENT_SRCS))

# Targets
## Default
.PHONY: all
all: $(BUILD_DIR) $(SERVER_EXEC) $(CLIENT_EXEC)

## Create build directory
$(BUILD_DIR):
	@mkdir -p $@

## Link
$(SERVER_EXEC): $(SERVER_OBJS)
$(CLIENT_EXEC): $(CLIENT_OBJS)

$(SERVER_EXEC) $(CLIENT_EXEC):
	@$(CC) $(LDFLAGS) $^ -o $@ $(LDLIBS)

## Compile
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@$(CC) $(CFLAGS) -c $< -o $@

## Clean build directory
.PHONY: clean
clean:
	@rm -f $(SERVER_EXEC) $(CLIENT_EXEC) $(SERVER_OBJS) $(CLIENT_OBJS)
