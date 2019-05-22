//Created by FENG, H
#include "preprocessor_impl.h"
#include <sstream>
#include <string>
#include <fstream>
#include <map>
#include <regex>
using namespace std;
namespace juicy {

	void Preprocessor_impl::process_define()
	{
		size_t pos;
		map <string, string> macro_map;
		ifstream in;
		string lineStr, segment, tmp = "", key, value;
		for(auto it: input_files_names)
		{
			tmp = "";
			in.open(it,ios::in);
			if(!in)
			{
				cout << "File does not exist!" << endl;
				exit(-1);
			}
			while(getline(in, lineStr))
			{
				stringstream sstr(lineStr);
				sstr >> segment;
				if(segment == "#define")
				{
					if(sstr >> key >> value)
						macro_map[key] = value;
					else
					{
						cout << "Error" << endl;
						exit(-1);
					}
				}
				else
				{
					tmp += lineStr;
					tmp += "\n";
				}
			}
			for(auto it: macro_map)
			{
				key = it.first;
				value = it.second;
				size_t length = tmp.size();
				while((pos = tmp.find(key)) != string::npos)
				{
					regex re("[^_a-zA-Z0-9]*");
					if(pos == 0 && regex_match(tmp.substr(key.size(), 1), re))
						tmp.replace(pos, key.size(), value);
					else if( regex_match(tmp.substr(pos - 1, 1), re) && regex_match(tmp.substr(pos + key.size(), 1), re) )
						tmp.replace(pos, key.size(), value);
				}
			}
			output_files.push_back(tmp);
			in.close();
		}
	}

	void Preprocessor_impl::process_include()
	{
		//run process_define before running this func
		//output files means files processed by #define
		vector<string> files_tmp = output_files;
	}

}