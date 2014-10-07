//***************************************************************************************
// FileWriter.h by Alexander Burton (C) 2014 All Rights Reserved.
//***************************************************************************************
//TODO
//make an overloaded read data that takes a tag and a int
//the int will be how many many things to read out affter the token
//it would return a vector of vectors? Some such a thing like this

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
	FileWriter(string name);

	std::vector<std::string> ReadData(std::string token);
	void WriteData(std::string token, std::string data);
private:
	std::string filename;
};

