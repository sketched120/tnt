CC = cc
CFLAGS = -Wall -Wextra -O2 -Iinclude -lcurl -lcjson -lzip

TARGET = zap

SRC = main.c \
      auth.c \
      download.c \
      fabric.c \
      fast.c \
      jvm_args.c \
      launch.c \
      natives.c \
      utils.c \
      version.c

OBJ = $(SRC:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
