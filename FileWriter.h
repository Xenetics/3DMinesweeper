#pragma once
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>

using std::string;

class FileWriter
{
public:
	FileWriter(string filename);
	~FileWriter();

	std::vector<std::string> ReadData(std::string token);
private:
	std::ifstream& file;
};

