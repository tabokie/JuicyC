#include "juicyc/compiler.h"
#include "juicyc/status.h"

using namespace juicyc;

int main(int argc, char** argv) {

	// parse option
	CompilerOption opt;

	auto p = Compiler::NewCompiler(opt);
	if(!p) {
		return 1;
	}

	auto s = p->Parse();
	if(!s.ok()) {
		return 1;
	}

	s = p->GenerateIR();
	if(!s.ok()) {
		return 1;
	}

	s = p->GenerateAsm();
	if(!s.ok()) {
		return 1;
	}

	return 0;
}