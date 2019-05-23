//Created by FENG, H
#ifndef JUICYC_PreprocessorImpl_H_
#define JUICYC_PreprocessorImpl_H_

#include "preprocessor.h" 
using namespace std;
namespace juicyc {

//pp_impl class
//create a pp_impl class
//specify the construction function with one parameter
//with construct function pass parameters about input files(CompilerOption& opt)
class PreprocessorImpl: public Preprocessor
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
	//noted, from opt -> input_files_names_
	//output_files_ -> opt
	PreprocessorImpl(CompilerOption& opt):
		input_files_names_(opt.files)
	{}
	~PreprocessorImpl() {}
	Status Run();
protected:
	vector<string> input_files_names_;
	vector<string> output_files_;
};

}
#endif