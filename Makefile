export PKG_CONFIG_PATH = /opt/mono/lib/pkgconfig
CC = gcc # C compiler
CFLAGS = -fPIC -Wall -Wextra -O2 -g 
LDFLAGS = -shared  # linking flags
RM = rm -f  # rm command
TARGET_LIB = libcrow.so # target lib

SRCS = crow_context.c crow_object.c crow_array.c# source files
OBJS = $(SRCS:.c=.o)

CFLAGS += `pkg-config --cflags --libs mono-2` 
CFLAGS += `pkg-config --cflags --libs cairo` 

.PHONY: all
all: ${TARGET_LIB}

$(TARGET_LIB): $(OBJS)
	$(CC) ${LDFLAGS} -o $@ $^

$(SRCS:.c=.d):%.d:%.c
	$(CC) $(CFLAGS) -MM $< >$@

include $(SRCS:.c=.d)

.PHONY: clean
clean:
	-${RM} ${TARGET_LIB} ${OBJS} $(SRCS:.c=.d)
