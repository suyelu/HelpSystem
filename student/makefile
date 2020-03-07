CC := gcc
CFLAGS := -Wall -g -Os -lpthread
SHDIR := ../common
OBJS := student.o ${SHDIR}/common.o
all: helpme
	@echo "This is OK"

helpme: ${OBJS}
	@${CC} -o $@ ${CFLAGS} ${OBJS}

${OBJS}: %.o: %.c
	@${CC} -c ${CFLAGS} $< -o $@

.PHONY: clean
	@rm -f *.o helpme
	@rm -f ../common/*.o
	@echo "Cleaning done"

install:
	@bash .install.sh
