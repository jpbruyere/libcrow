export PKG_CONFIG_PATH = /opt/mono/lib/pkgconfig

CC = gcc
CFLAGS = -fPIC -Wall -Wextra -O2 -g -DDEBUG
LDFLAGS = -shared

TARGET  = libcrow.so
SOURCES = crow_context.c crow_object.c crow_group.c crow_array.c tests.c
OBJECTS = $(SOURCES:.c=.o)

CFLAGS += `pkg-config --cflags --libs mono-2` 
CFLAGS += `pkg-config --cflags --libs cairo` 

.PHONY: all
all: ${TARGET}

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS)  ${LDFLAGS} -o $(TARGET) $(OBJECTS)

.PHONY: clean
clean:
	-rm -f ${TARGET} ${OBJECTS} 
