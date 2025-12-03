CXX = g++
CXXFLAGS_BASE = -std=c++17 -Iinclude -Wall -Wextra
comma := ,
LDFLAGS_BASE = -ldl -lpthread
SRCDIR = src
OBJDIR = obj
BINDIR = bin
GENDIR = include/generated
TARGET = $(BINDIR)/combine
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
CONFIG_HEADER = $(GENDIR)/BuildConfig.h
-include build.conf
RENDERER ?= OpenGL
SUPPORTED_SCRIPTS ?= ChaiScript
INIT_SCRIPT ?= scripts/init.chai
DEBUG ?= yes
STATIC ?= no
OPTIMIZATIONS ?= yes
CXXFLAGS = $(CXXFLAGS_BASE)
LDFLAGS = $(LDFLAGS_BASE)
ifeq ($(RENDERER),OpenGL)
    LDFLAGS += -lGL -lGLEW -lglfw
endif

ifeq ($(RENDERER),Vulkan)
    LDFLAGS += -lvulkan -lglfw
endif

ifneq (,$(findstring ChaiScript,$(SUPPORTED_SCRIPTS)))
    CXXFLAGS += -Iexternal/chaiscript/include
endif

ifneq (,$(findstring Lua,$(SUPPORTED_SCRIPTS)))
    LDFLAGS += -llua
endif

ifneq (,$(findstring Squirrel,$(SUPPORTED_SCRIPTS)))
    LDFLAGS += -lsquirrel -lsqstdlib
endif

ifeq ($(DEBUG),yes)
    CXXFLAGS += -g
endif
ifeq ($(STATIC),yes)
    CXXFLAGS += -static
    LDFLAGS += -static
endif

ifeq ($(OPTIMIZATIONS),yes)
    CXXFLAGS += -O2
else
    CXXFLAGS += -O0
endif

ifndef ECHO
HIT_TOTAL != ${MAKE} ${MAKECMDGOALS} --dry-run ECHO="HIT_MARK" | grep -c "HIT_MARK"
HIT_COUNT = $(eval HIT_N != expr ${HIT_N} + 1)${HIT_N}
ECHO = "[\033[1;32m`expr ${HIT_COUNT} '*' 100 / ${HIT_TOTAL}`%\033[1;0m]"
endif

SCRIPT_INCLUDES = $(shell echo '$(SUPPORTED_SCRIPTS)' | sed 's/,/\n/g' | while read s; do echo "\#include \"scripting/$${s}Engine.h\""; done)
SCRIPT_INITS = $(shell echo '$(SUPPORTED_SCRIPTS)' | sed 's/,/\n/g' | while read s; do echo "engine.addScriptEngine(std::make_unique<Combine::$${s}Engine>());"; done)
all: directories $(CONFIG_HEADER) $(TARGET)

directories:
	@mkdir -p $(OBJDIR) $(BINDIR) $(GENDIR) scripts

$(CONFIG_HEADER): build.conf
	@echo -e GEN $(ECHO) $@
	@echo "#ifndef BUILD_CONFIG_H" > $@
	@echo "#define BUILD_CONFIG_H" >> $@
	@echo "" >> $@
	@echo "#define COMBINE_RENDERER $(RENDERER)" >> $@
	@echo "#define COMBINE_RENDERER_HEADER \"renderers/$(RENDERER)Renderer.h\"" >> $@
	@echo "#define COMBINE_RENDERER_CLASS $(RENDERER)Renderer" >> $@
	@echo "" >> $@
	@echo "#define COMBINE_INIT_SCRIPT \"$(INIT_SCRIPT)\"" >> $@
	@echo "#define COMBINE_WINDOW_TITLE \"$(WINDOW_TITLE)\"" >> $@
	@echo "" >> $@
	@$(foreach script,$(subst $(comma), ,$(SUPPORTED_SCRIPTS)),echo "#define COMBINE_SCRIPT_$(shell echo $(script) | tr '[:lower:]' '[:upper:]') 1" >> $@;)
	@echo "" >> $@
	@if [ "$(DEBUG)" = "yes" ]; then echo "#define COMBINE_DEBUG 1" >> $@; fi
	@echo "" >> $@
	@echo "#endif" >> $@

$(TARGET): $(OBJECTS)
	@echo -e LD $(ECHO) $@
	@$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)
	@echo "-> $(TARGET)"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(CONFIG_HEADER)
	@echo -e CXX $(ECHO) $<
	@$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	@echo -e CLEAN $(ECHO)
	@rm -rf $(OBJDIR) $(BINDIR) $(GENDIR)

run: all
	./$(TARGET)

config:
	@echo "Build Configuration:"
	@echo "  RENDERER:          $(RENDERER)"
	@echo "  SUPPORTED_SCRIPTS: $(SUPPORTED_SCRIPTS)"
	@echo "  INIT_SCRIPT:       $(INIT_SCRIPT)"
	@echo "  DEBUG:             $(DEBUG)"
	@echo "  OPTIMIZATIONS:     $(OPTIMIZATIONS)"
	@echo "  STATIC:            $(STATIC)"
	@echo ""
	@echo "CXXFLAGS: $(CXXFLAGS)"
	@echo "LDFLAGS:  $(LDFLAGS)"

menuconfig:
	@./menuconfig.sh

.PHONY: all clean run directories config menuconfig
