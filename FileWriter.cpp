#include "FileWriter.h"
//each instance of this class wil read/write to a single file

FileWriter::FileWriter()
{
}


FileWriter::~FileWriter()
{
}


//should return any values after the token in a vector 
//but for now
std::vector<std::string> FileWriter::ReadData(std::string tag)
{
	string buf;
	std::vector<std::string> ret;
	while (file.good())
	{
		//get the token 
		file >> buf;
		if (buf[0] == '#')
		{
			getline(file, buf);
			continue;
		}
		else if (buf == tag)
		{
			//get the file path'
			file >> buf;
			ret.push_back(buf.c_str());
		}
	}
}