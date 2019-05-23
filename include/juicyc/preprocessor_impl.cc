//Created by FENG, H
#include "PreprocessorImpl.h"
#include <sstream>
#include <string>
#include <fstream>
#include <map>
#include <regex>
using namespace std;
namespace juicyc 
{

	Status PreprocessorImpl::Run()
	{
		size_t pos;
		map <string, string> macro_map;
		ifstream in;
		string lineStr, segment, tmp = "", key, value;
		for(auto it: input_files_names_)
		{
			macro_map.clear();
			tmp = "";
			in.open(it,ios::in);
			if(!in)
			{
				cout << "File does not exist!" << endl;
				return Status::IOError("File does not exist");
			}
			while(getline(in, lineStr))
			{
				stringstream sstr(lineStr);
				sstr >> segment;
				//process define
				if(segment == "#define")
				{
					if(sstr >> key >> value)
						macro_map[key] = value;
					else
					{
						cout << "Error" << endl;
						return Status::IOError("define error");
					}
				}
				//process include
				else if(segment == "#include")
				{
					if(sstr >> value)
					{
						if(!regex_match(value, regex("\".*\"")))
						{
							cout << "Include error, geshi error" << endl;
							return Status::IOError("include error");
						}
						value = value.substr(1, value.size()-2);
						ifstream ftmp;
						ftmp.open(value, ios::in);
						if(!ftmp)
						{
							cout << "Include error, no such file" << endl;
							return Status::IOError("include error");
						}
						stringstream buffer;
						buffer << ftmp.rdbuf();
						string contents(buffer.str());
						ftmp.close();
						tmp += contents;
						tmp += "\n";

					}
					else
					{
						cout << "Include error" << endl;
						return Status::IOError("include error");
					}
				}
				//not macro
				else
				{
					for(auto it: macro_map)
					{
						key = it.first;
						value = it.second;
						while((pos = lineStr.find(key)) != string::npos)
						{
							if((key.size() == lineStr.size()) ||
							   (pos == 0 && regex_match(lineStr.substr(key.size(), 1), regex("[^_a-zA-Z0-9]*"))) ||
							   (pos + key.size() == lineStr.size() && regex_match(lineStr.substr(pos - 1, 1), regex("[^_a-zA-Z0-9]*")))
							  )
								lineStr.replace(pos, key.size(), value);
							else if( regex_match(lineStr.substr(pos - 1, 1), regex("[^_a-zA-Z0-9]*")) && regex_match(lineStr.substr(pos + key.size(), 1), regex("[^_a-zA-Z0-9]*")))
								lineStr.replace(pos, key.size(), value);
						}
					}
					tmp += lineStr;
					tmp +='\n';				
				}
			}
			output_files_.push_back(tmp);
			in.close();
		}
		return Status::OK();
	}
}