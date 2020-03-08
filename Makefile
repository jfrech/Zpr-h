CC = gcc
override CFLAGS += -Wall -Wpedantic -Werror -O3

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)
OBJECTS = $(patsubst %.c, %.o, $(SOURCES))

ZPRH_STDLIB = $(wildcard stdlib/*.zpr)
ZPRH_TESTFILE = stdlib/_on_build_test.zpr

Zprh: $(SOURCES) $(HEADERS) $(ZPRH_STDLIB)
	$(CC) $(CFLAGS) -c $(SOURCES)
	$(CC) $(OBJECTS) -o $@
	rm -f $(OBJECTS)
ifneq ($(test),skip)
	@printf "\nRunning an on-build test ... "
	@printf "(Run \"make test=skip\" to skip it.)\n"
ifeq ($(test),valgrind)
	valgrind --leak-check=full --tool=memcheck \
		./Zprh $(ZPRH_TESTFILE) 1> /dev/null
else
	@printf "Skipping memory check. "
	@printf "(Run \"make test=valgrind\" to perform one.)\n"
endif
	@printf "\n\n    \33[4m\33[93m"
	@./Zprh $(ZPRH_TESTFILE)
	@printf "\33[0m\n\n"
	./Zprh $(ZPRH_TESTFILE) --rule-info 1> /dev/null
endif
