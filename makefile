# this makefile is tuned for Microsoft nmake build tool.
# dependent on win-flex-bison and llvm.

SHELL_DEL = del
CC = cl
# careful not to override existing variable
INCLUDE_FLAG = /I ./include /I .
LLVM_PATH = ./bin/llvm
WIN_FLEX_BISON_PATH = ./bin/win_flex_bison
# build by dynamic link
FLAG = /EHsc /MD
TEST_FLAG = /link /subsystem:console

# llvm library should include gtest
# TEST_LIB = gtest.lib gtest_main.lib
LLVM_LIB = $(LLVM_PATH)/lib/*.lib
SRC = frontend/*.cc util/*.cc compiler/*.cc visitor/syntax/*.cc
CMD = cmd/*.cc

# to fix `spawn failed error` for now
LEX = start $(WIN_FLEX_BISON_PATH)/win_flex.exe
YACC = start $(WIN_FLEX_BISON_PATH)/win_bison.exe

all: gen unittest build
.PHONY : all

build:
  $(CC) /Fejuicyc.exe $(INCLUDE_FLAG) $(LLVM_LIB) $(SRC) $(CMD) $(FLAG)
.PHONY : build

unittest:
  $(CC) /Feunittest.exe $(INCLUDE_FLAG) $(TEST_LIB) $(LLVM_LIB) $(SRC) test/*.cc $(FLAG) $(TEST_FLAG)
.PHONY : unittest

mingw: fallback_build fallback_unittest
.PHONY : mingw

fallback_build:
  g++ -o juicyc -std=gnu++11 -I. -I./include $(SRC) $(CMD)
.PHONY : fallback_unittest

fallback_unittest:
  g++ -o unittest -static -std=gnu++14 -I. -I./include $(SRC) test/*.cc -lgtest -lpthread
.PHONY : fallback_unittest

gen:
	$(LEX) -o./frontend/gen_scan.cc --wincompat ./frontend/c.l
	$(YACC) -d -o./frontend/gen_parse.cc ./frontend/c.y
.PHONY : gen

clean:
  $(SHELL_DEL) *.exe
  $(SHELL_DEL) *.obj
  $(SHELL_DEL) *.o
  $(SHELL_DEL) *.ll
  $(SHELL_DEL) *.json
  $(SHELL_DEL) *.s
.PHONY : clean