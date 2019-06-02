# JuicyC
C compiler for fun

## Dependency

-	Win-Flex-Bison: binary needed
	-	download `win_flex_bison` and unpack it to `$(JUICYC_DIR)/bin/win_flex_bison` folder
	- override `WIN_FLEX_BISON_PATH` in Makefile if needed
-	LLVM: library + header needed
	-	compile LLVM src and place Release-Dynamic (/MD) library to `$(JUICYC_DIR)/bin/llvm/lib` folder
	-	override `LLVM_PATH` in Makefile if needed
	-	make sure llvm header can be accessed through `INCLUDE env variable`, or append your local header path to `INCLUDE_FLAG` in Makefile
-	google test: header needed
	-	LLVM library should come with gtest, thus only gtest header is needed

## Overview

-	Preprocessor
	-	macro and include command
-	FrontEnd
	-	build parse tree using Yacc
-	Visitor
	-	SyntaxVisitor: generate IR from parse tree
	- JsonVisitor: generate JSON format abstract of parse tree
-	BackEnd
	-	convert IR to assembly using LLVM