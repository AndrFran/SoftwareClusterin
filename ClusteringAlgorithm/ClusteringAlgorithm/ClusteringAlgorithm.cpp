
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include <algorithm>
#include <cstdlib>
#include <filesystem>

#include "CLusterinData.h"

using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
using namespace std;
vector<string> sytems = { "printf()","scanf()", "system()", "getch()" };


bool isInArr(string* pArr, string cmp, int count)
{
	for (int i = 0; i < count; i++)
	{
		if (pArr[i] == cmp)
		{
			return true;
		}
	}
	return false;
}



int calculate_indent(string str)
{
	int i = 0;
	if (0 == str.size())
	{
		return 0;
	}
	while (' ' == str.at(i))
	{
		i++;
	}
	return i;
}

string GetFunctionName(string str)
{
	return str.substr(calculate_indent(str), str.find(')') - calculate_indent(str)-1);
}
string GetFileName(string str)
{
	for (int i = 0; i < sytems.size(); i++)
	{
		if (std::string::npos != str.find(sytems.at(i)))
		{

			return "system";
		}
	}
	return str.substr(str.find("at "), str.find(':'));
}

bool checkPresense(string str, vector<string> vec)
{
	for (int i = 0; i < vec.size(); i++)
	{
		if (vec[i] == str)
		{
			return true;
		}
	}
	return false;
}



/*main container for the nodes*/
static std::vector<Node*> RootNodes;


int main()
{
	ifstream inFile;
	// open the file stream

	string myPath = "./ProjectUnderTest/Chess";

	vector<filesystem::directory_entry> entries;
	for (const filesystem::directory_entry& dirEntry : recursive_directory_iterator(myPath))
	{
		if (".c" == dirEntry.path().extension())
		{
			entries.push_back(dirEntry);
		}

		std::cout << dirEntry.path() << " extension " << dirEntry.path().extension() << std::endl;
	}


	//construct a filenames string
	string filenames;

	// Iterate over a vector using range based for loop
	for (auto& elem : entries)
	{
		filenames += elem.path().generic_string() + "  ";
	}

	cout << filenames << endl;

	string cflow = "cflow.exe -b --verbose  -d=5 ";
	string output_direction = "> InputExample.txt";
	system("cd");
	string result = cflow + filenames + output_direction;
	cout << result << endl;
	system(result.c_str());

	inFile.open("InputExample.txt");
	// check if opening a file failed
	if (inFile.fail()) {
		cerr << "Error opeing a file" << endl;
		inFile.close();
		exit(1);
	}
	string line;
	int prev_indent = 0;
	stack<Node*> nodes;
	Node* lastEntry;
	while (getline(inFile, line))
	{
		cout << line << endl;
		// check if the root found
		if (0 == calculate_indent(line))
		{
			RootNodes.push_back(new Node(0, GetFunctionName(line), GetFileName(line)));
			prev_indent = 0;
			lastEntry = RootNodes.back();
			continue;
		}
		else if (calculate_indent(line) > prev_indent)
		{
			prev_indent = calculate_indent(line);
			nodes.push(lastEntry);
		}
		else if (calculate_indent(line) < prev_indent)
		{
			prev_indent = calculate_indent(line);
			nodes.pop();

		}
		nodes.top()->AddCaller(new Node(calculate_indent(line), GetFileName(line), GetFunctionName(line)));
		lastEntry = nodes.top()->GetLastCaller();
	}
	// close the file stream
	inFile.close();

	string use_ast = "python using_gcc_E_libc.py " + filenames + "> AST.txt";
	system(use_ast.c_str());

	inFile.open("AST.txt");
	// check if opening a file failed
	if (inFile.fail()) {
		cerr << "Error opeing a file" << endl;
		inFile.close();
		exit(1);
	}

	// parse the data calls
	while (getline(inFile, line))
	{
	FuncDef:
		cout << line << endl;
		if (std::string::npos != line.find("FuncDef"))
		{   
			// Get a function name
			getline(inFile, line);
			std::string name = line.substr(line.find("Decl: ")+strlen("Decl: "), line.find(",")- line.find("Decl: ")- strlen("Decl: "));
			
			
			std::vector<string> params;
			int function_level_indent = calculate_indent(line);
			int current_indent = function_level_indent + 1;
			
			while(current_indent > function_level_indent)
			{
				getline(inFile, line);

				if (std::string::npos != line.find("FuncDef"))
				{
					goto FuncDef;
				}

				cout << line << endl;
				current_indent = calculate_indent(line);
				if (std::string::npos != line.find("ID:"))
				{
					cout << "catch" << endl;
				}

				//parse parameters list
				if (std::string::npos != line.find("ParamList"))
				{
					int param_indent = calculate_indent(line);
					int curr_indent = param_indent+1;
					while (curr_indent > param_indent)
					{
						getline(inFile, line);
						curr_indent = calculate_indent(line);
						
						if (std::string::npos != line.find("Decl: ")
							&& std::string::npos != line.find(","))
						{
							params.push_back(line.substr(line.find("Decl: ")+ strlen("Decl: "),line.find(",") - line.find("Decl: ") - strlen("Decl: ")));
						}
					}
				}

				//parse the function compound
				if (std::string::npos != line.find("Compound"))
				{
					std::vector<string> internals;

					
					int param_indent = calculate_indent(line);
					int curr_indent = param_indent+1;

					while (curr_indent > param_indent)
					{
						getline(inFile, line);
						if (std::string::npos != line.find("FuncDef"))
						{
							goto FuncDef;
						}
						curr_indent = calculate_indent(line);
						
						// get the internal
						if (std::string::npos != line.find("Decl: ")
							&& std::string::npos != line.find(","))
						{
							int a = line.find("Decl: ")+strlen("Decl: ");
							int b = line.find(',');
							std::string sub = line.substr(a, b-a);
							if (false == checkPresense(sub, internals))
							{
								internals.push_back(sub);
							}
							continue;
						}

						// filter function call
						if (std::string::npos != line.find("FuncCall:"))
						{
							// skip the function ID
							getline(inFile, line);
							continue;
							// skip the funccall ID
							//internals.push_back(line.substr(line.find("ID: "), line.size()));

							// Get next line
						}

						// identify data calls
						if (std::string::npos != line.find("ID:"))
						{
							int a = line.find("ID:") + strlen("ID: ");
							int b = line.length();
						
							std::string data_access = line.substr(a, b-a);
							

							if (false == checkPresense(data_access, internals) &&
								false == checkPresense(data_access, params))
							{
								RootNodes[0]->AddDataLink(new DataLink(true,true, data_access,"CHESSPR.c"), name);
							}
						}

					}

				}
			}
		}
	}


}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started : 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
