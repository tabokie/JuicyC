# this makefile is tuned for Microsoft nmake build tool
# with no wildcard capture

SHELL_DEL = del
CC = cl
# careful not to override existing variable
INCLUDE_FLAG = /I ./include /I .
FLAG = /EHsc
TEST_FLAG = /link /subsystem:console

TEST_LIB = gtest.lib gtest_main.lib
SRC = util/*.cc

build:
  $(CC) /Fejuicyc.exe $(INCLUDE_FLAG) $(SRC) $(FLAG)
.PHONY : build

unittest:
  $(CC) /Feunittest.exe $(INCLUDE_FLAG) $(TEST_LIB) $(SRC) test/*.cc $(FLAG) $(TEST_FLAG)
.PHONY : unittest

clean:
  $(SHELL_DEL) *.exe
  $(SHELL_DEL) *.obj
.PHONY : clean