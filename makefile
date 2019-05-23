# this makefile is tuned for Microsoft nmake build tool
# with no wildcard capture
SHELL_DEL = del
CC = cl
# careful not to override existing variable
INCLUDE_FLAG = /I ./include /I .
FLAG = /EHsc
TEST_FLAG = /link /subsystem:console

TEST_LIB = gtest.lib gtest_main.lib
SRC = frontend/*.cc util/*.cc include/juicyc/*.cc
CMD = cmd/*.cc
# pp/*.cc backend/*.cc asm/*.cc

# to fix `spawn failed error` for now
LEX = start ./bin/win_flex_bison/win_flex.exe
YACC = start ./bin/win_flex_bison/win_bison.exe

all: gen unittest build
.PHONY : all

build:
  $(CC) /Fejuicyc.exe $(INCLUDE_FLAG) $(SRC) $(CMD) $(FLAG)
.PHONY : build

unittest:
  $(CC) /Feunittest.exe $(INCLUDE_FLAG) $(TEST_LIB) $(SRC) test/*.cc $(FLAG) $(TEST_FLAG)
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
.PHONY : clean