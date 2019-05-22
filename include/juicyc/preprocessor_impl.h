//Created by FENG, H
#ifndef JUICYC_PREPROCESSOR_IMPL_H_
#define JUICYC_PREPROCESSOR_IMPL_H_

#include "preprocessor.h" 
using namespace std;
namespace juicy {

//pp_impl class
//create a pp_impl class
//specify the construction function with one parameter
//with construct function pass parameters about input files(CompilerOption& opt)
class Preprocessor_impl: public Preprocessor
{
public:
	//public inherited
	bool eof();
	char get();
	std::string file_name();
	uint32_t line_no();
	uint32_t col_no();
	//task scheduler
	void push(std::string& file);
	//added 
	//noted, from opt -> input_files_names
	//output_files -> opt
	vector<string> input_files_names;
	vector<string> output_files;
	Preprocessor_impl(CompilerOption& opt):
		input_files_names(opt.files)
	{}
	~Preprocessor_impl() {}
	void process_define();
	void process_include();
};

}
#endif