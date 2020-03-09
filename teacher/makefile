CC := gcc
CFLAGS :=  -Wall -g -Os -pthread
SHDIR := ../common
OBJS := teacher.o ${SHDIR}/common.o
all: dohelp
	@echo "This is OK"

dohelp: ${OBJS}
	@${CC} -o $@ ${CFLAGS} ${OBJS}

${OBJS}: %.o: %.c
	@${CC} -c ${CFLAGS} $< -o $@

.PHONY: clean
clean:
	@rm -f *.o dohelp
	@rm -f ../common/*.o
	@echo "Cleaning done"

install:
	@bash .install.sh
