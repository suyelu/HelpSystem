CC := gcc
CFLAGS := -Wall -g -Os -pthread
SHDIR := ../common
OBJS := master.o ${SHDIR}/common.o
all: helper
	@echo "This is OK"

helper: ${OBJS}
	@${CC} -o $@ ${CFLAGS} ${OBJS}

${OBJS}: %.o: %.c
	@${CC} -c ${CFLAGS} $< -o $@

.PHONY: clean
clean:
	@rm -f *.o helper
	@rm -f ../common/*.o
	@echo "Cleaning done"

install:
	@pkill helper
	@bash .install.sh
	@helper &
