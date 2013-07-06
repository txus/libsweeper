CC=clang
CFLAGS=-g -O3 -std=c99 -Wall -Werror -Iinclude -DNDEBUG $(OPTFLAGS)
LIBS=$(OPTLIBS)
PREFIX?=/usr/local

SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

TARGET=build/libsweeper.a
SO_TARGET=$(patsubst %.a,%.so,$(TARGET))

# The Target Build
all: $(TARGET) $(SO_TARGET) tests

dev: CFLAGS=-g -std=c99 -Wall -Iinclude -Werror $(OPTFLAGS)
dev: all

$(TARGET): CFLAGS += -fPIC
$(TARGET): build $(OBJECTS)
				ar rcs $@ $(OBJECTS)
				ranlib $@

$(SO_TARGET): $(TARGET) $(OBJECTS)
				$(CC) -shared -o $@ $(OBJECTS)

build:
				@mkdir -p build

# The Unit Tests
.PHONY: tests
tests: CFLAGS += $(TARGET)
tests: $(TESTS)
				sh ./tests/runtests.sh

valgrind:
				VALGRIND="valgrind" $(MAKE)

# The Cleaner
clean:
				rm -rf build $(OBJECTS) $(TESTS)
				rm -f tests/tests.log
				rm -rf `find . -name "*.dSYM" -print`
				rm -rf `find . -name "*.o" -print`

# The Install
install: all
				install -d $(DESTDIR)/$(PREFIX)/lib/
				install $(TARGET) $(DESTDIR)/$(PREFIX)/lib/

# The Checker
BADFUNCS='[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_)'
check:
				@echo Files with potentially dangerous functions.
				@egrep $(BADFUNCS) $(SOURCES) || true
