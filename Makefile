CFLAGS=-g -std=c11 -O2 -Wall -Isrc -Iinclude -I. $(OPTFLAGS)
CXXFLAGS=-g -O2 -std=c++11 -Wall -Isrc -Iinclude -I. $(OPTFLAGS)
LDLIBS=$(OPTLIBS)
LDFLAGS=-Lhttp-parser

CC=gcc
CXX=g++
LD=gcc

SRCDIR=src/
OBJDIR=obj/
RESDIR=res/
# removes the directory and returns a list of files with .c extension
# SOURCES=$(wildcard src/**/*.c src/*.c)
CSOURCES=$(wildcard $(SRCDIR)*.c)
COBJECTS=$(patsubst $(SRCDIR)%.c,$(OBJDIR)%.o,$(CSOURCES))
CPPSOURCES=$(wildcard $(SRCDIR)*.cpp)
CPPOBJECTS=$(patsubst $(SRCDIR)%.cpp,$(OBJDIR)%.o,$(CPPSOURCES))
OBJECTS=$(COBJECTS) $(CPPOBJECTS)
BINARIES=bin/
OPTLIBS=-lstdc++ -lpthread -ldl `pkg-config --libs openssl sqlite3` $(HTTPPARSER_LDLIBS)

HTTPPARSER_SUBDIR=$(CURDIR)/http-parser
HTTPPARSER_LIBSFILES=$(HTTPPARSER_SUBDIR)/libhttp_parser.a
HTTPPARSER_LDLIBS=-lhttp_parser

APP_DIR=appsrc/
APP_SRC=$(wildcard $(APP_DIR)*.c)
APPS=$(patsubst %.c,%.exe,$(APP_SRC))

TEST_DIR=testsrc/
TEST_SRC=$(wildcard $(TEST_DIR)*_tests.c)
TESTS=$(patsubst %.c,%.exe,$(TEST_SRC))

# dev: CFLAGS=-g -Wall -Isrc -Wall -Wextra $(OPTFLAGS)
dev: $(TARGET)

$(CPPOBJECTS): $(OBJDIR)%.o : $(SRCDIR)%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(COBJECTS): $(OBJDIR)%.o : $(SRCDIR)%.c
	$(CC) $(CFLAGS) -c -o $@ $<

#$(RESOURCES): $(OBJDIR)%.o : $(RESDIR)%.rc
#	windres --codepage 65001 -i $< -o $@
	
%: %.c $(OBJECTS) $(HTTPPARSER_LIBSFILES)
	$(CC) $(LDFLAGS) $(CFLAGS) $(OBJECTS) $(RESOURCES) $< $(LDLIBS) -o $@

%: %.cpp $(OBJECTS) $(HTTPPARSER_LIBSFILES)
	$(CXX) $(LDFLAGS) $(CXXFLAGS) $(OBJECTS) $(RESOURCES) $< $(LDLIBS) -o $@

build:
	@mkdir -p $(BINARIES)
	@mkdir -p $(OBJDIR)

# The Unit Tests
.PHONY: tests
tests: LDLIBS = $(OPTLIBS)
tests: $(TESTS)
	echo "not running tests... takes too long."
#	sh ./testsrc/runtests.sh

$(HTTPPARSER_LIBSFILES): libhttp_parser ;

.PHONY: libhttp_parser
libhttp_parser:
	$(MAKE) -C $(HTTPPARSER_SUBDIR) package

testsclean:
	rm -f $(TESTS)

appsclean:
	@rm -f $(APPS)

# The Applications
.PHONY: apps
apps: build
apps: LDLIBS = $(OPTLIBS)
apps: $(OBJECTS) $(APPS)

# The Cleaner
clean:
	rm -rf $(BINARIES) $(OBJECTS) $(TESTS) $(APPS)
	rm -f testsrc/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`

