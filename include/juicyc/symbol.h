#ifndef JUICYC_SYMBOL_H_
#define JUICYC_SYMBOL_H_

#include <string>

namespace juicyc {

struct Symbol {
	bool is_terminal = true;
	int type = 0;
	Symbol* left = nullptr;
	Symbol* right = nullptr;
	Symbol(bool is_terminal): is_terminal(is_terminal) {}
	~Symbol() {
		delete right;
	}
};

struct Terminal : public Symbol {
	// refer to tagger for detailed string
	std::string value;
	uint16_t file;
	uint32_t line;
	uint32_t col;
	Terminal() : Symbol(true) { }
	~Terminal() { }
};

struct NonTerminal : public Symbol {
	Symbol* childs;
	NonTerminal() : Symbol(false) { }
	~NonTerminal() {
		delete childs;
	}
};

} // namespace juicyc

#endif // JUICYC_SYMBOL_H_