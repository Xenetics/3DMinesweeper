//***************************************************************************************
// FileWriter.cpp by Alexander Burton (C) 2014 All Rights Reserved.
//***************************************************************************************

#include "FileWriter.h"
//each instance of this class wil read/write to a single file


FileWriter::FileWriter(string filename) 
{
	file.open(filename, std::ios::in);
}


FileWriter::~FileWriter()
{
}



//will return a vector of strings that were found after the tag/token requested
std::vector<std::string> FileWriter::ReadData(std::string tag)
{
	string buf;
	std::vector<std::string> ret;

	if (!file.is_open())
	{
		file.close();
		std::cerr << "Error, cant open data file: " << std::endl;
		return ret;
	}
	

	while (file.good())
	{
		//get the token 
		file >> buf;
		if (buf[0] == '#')
		{
			//comments
			getline(file, buf);
			continue;
		}
		else if (buf == tag)
		{
			//get the requested tag
			file >> buf;
			ret.push_back(buf.c_str());
		}
	}

	file.close();
	return ret;
}