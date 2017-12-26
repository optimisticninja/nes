PROJECT := nes

BINDIR := bin
OBJDIR := obj
SRCDIR := src

TESTDIR := tests
TESTOBJDIR := $(OBJDIR)/$(TESTDIR)

LIBRARY := $(BINDIR)/lib$(PROJECT).so
SOURCES := $(shell find $(SRCDIR) -name '**.cc')
OBJECTS := $(shell echo $(SOURCES:.cc=.o) | sed 's/$(SRCDIR)/$(OBJDIR)/g')

TEST_BINARY := $(BINDIR)/$(PROJECT)-tests
TEST_SOURCES := $(shell find $(TESTDIR) -name '**.cc')
TEST_OBJECTS := $(shell echo $(TEST_SOURCES:.cc=.o) | sed 's/$(TESTDIR)/$(OBJDIR)\/$(TESTDIR)/g')

CXXSTD := -std=c++14
WARNINGS := -Wall -Werror -Wextra
# DEFINITIONS (CFLAG OPTIONS):
#	-DDEBUG == Debug logging
DEFINITIONS := -DDEBUG
CFLAGS := $(DEFINITIONS) -fPIC
LIBS := 
TEST_LIBS := -pthread -lgtest -lgtest_main

ifneq (,$(findstring -DDEBUG, $(CFLAGS)))
	CFLAGS += -g
endif

CXX += $(CXXSTD) $(WARNINGS) $(CFLAGS)

all: tests $(LIBRARY)

tests: $(TEST_BINARY)
	./$<

$(LIBRARY): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $^ -shared $(LIBS) -o $@

$(OBJECTS): $(OBJDIR)%.o: $(SRCDIR)%.cc
	@mkdir -p $(@D)
	$(CXX) $< -c -o $@

$(TEST_BINARY): $(filter-out obj/main.o, $(OBJECTS)) $(TEST_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $^ $(LIBS) $(TEST_LIBS) -o $@

$(TEST_OBJECTS): $(TESTOBJDIR)%.o: $(TESTDIR)%.cc
	@mkdir -p $(@D)
	$(CXX) $< -c -o $@

clean:
	rm -rf $(BINDIR) $(OBJDIR)

