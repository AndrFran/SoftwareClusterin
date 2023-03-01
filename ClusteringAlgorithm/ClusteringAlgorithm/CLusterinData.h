#pragma once

#include <string>
#include <vector>

using namespace std;
class DataLink
{
private:
	bool reading;
	bool writing;
	string name;
	string file_scope;

public:
	DataLink(bool read, bool write, string name, string scope)
	{
		this->reading = read;
		this->writing = write;
		this->name = name;
		this->file_scope = scope;
	}

	DataLink(DataLink* data)
	{
		this->file_scope = data->file_scope;
		this->name = data->name;
		this->writing = data->writing;
		this->reading = data->reading;
	}

	friend bool operator==(const DataLink& lhs, const DataLink& rhs)
	{
		if (lhs.file_scope == rhs.file_scope &&
			lhs.name == rhs.name &&
			lhs.writing == rhs.writing &&
			lhs.reading == rhs.reading)
		{
			return true;
		}
		return false;
	}
};

class Node
{
private:
	int level;

	// file and function together for and id
	string file;
	string function;
	std::vector<DataLink*> data;
	// functions called from
	std::vector<Node*> callers;
	// functions which calls
	std::vector<Node*> calle;

	int cluster_id;
public:
	Node(int level, string file, string function)
	{
		this->level = level;
		this->file = file;
		this->function = function;
		this->cluster_id = 0;
	}

	void AddCaller(Node* caller)
	{
		this->calle.push_back(caller);
		Node* tmp = this;
		caller->callers.push_back(tmp);
	}

	Node* GetLastCaller()
	{
		return calle.back();
	}

	void AddDataLink(DataLink data, string function_name)
	{
		if (function_name == this->function)
		{
			/** check if the data already present*/
			if (std::find(this->data.begin(), this->data.end(), data) == this->data.end()) {
				this->data.push_back(new DataLink(&data));
			}
		}
		for (int i = 0; i < this->calle.size(); i++)
		{
			this->calle[i]->AddDataLink(data, function_name);
		}
	}


};