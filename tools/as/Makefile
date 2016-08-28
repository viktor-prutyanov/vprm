#
#   Building VPRM assembler
#   @author Viktor Prutyanov viktor.prutyanov@phystech.edu
#   
    
TOOL_NAME := as

TOOLS := ..
INCLUDE_DIRS := $(TOOLS)/include/ ./
INCLUDE := $(addprefix -I, $(INCLUDE_DIRS))
BIN_DIR := $(TOOLS)/bin
TESTS_DIR := $(TOOLS)/tests/$(TOOL_NAME)
CXXFLAGS := -std=c++11 -Wno-format

C_FILES := $(wildcard *.c)
OBJ_C_FILES := ${C_FILES:%.c=%.o}

CPP_FILES := $(wildcard *.cpp)
OBJ_CPP_FILES := ${CPP_FILES:%.cpp=%.o}

$(TOOL_NAME): $(OBJ_C_FILES) $(OBJ_CPP_FILES)
	$(CXX) $^ $(LDFLAGS) -o $(BIN_DIR)/$@

%.o: %.cpp
	$(CXX) $? $(CXXFLAGS) $(INCLUDE) -c -o $@

%.o: %.c
	$(CC) $? $(CFLAGS) $(INCLUDE) -c -o $@

.PHONY: all clean test $(TOOL_NAME)

test: $(TOOL_NAME)
	$(BIN_DIR)/$(TOOL_NAME) $(addprefix $(TESTS_DIR)/arithm,.s .bin .hex)    

clean:
	@-rm -rf *.o $(BIN_DIR)/$(TOOL_NAME) $(TESTS_DIR)/*.bin $(TESTS_DIR)/*.hex
