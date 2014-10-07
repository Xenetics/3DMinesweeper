//***************************************************************************************
// FileWriter.cpp by Alexander Burton (C) 2014 All Rights Reserved.
//***************************************************************************************

#include "FileWriter.h"
//each instance of this class wil read/write to a single file


FileWriter::FileWriter(string name) : filename(name)
{
	
}


void FileWriter::WriteData(std::string token, std::string data)
{
	std::vector<std::string> strReplace = ReadData(token);
	std::ifstream filein(filename); //File to read from
	std::ofstream fileout("fileout.txt"); //Temporary file
	if (!filein || !fileout)
	{
		std::cout << "Error opening files!" << std::endl;
		return;
	}

	string strTemp;
	//bool found = false;
	while (filein >> strTemp)
	{
		if (strTemp == strReplace[0]){
			strTemp = data;
			//found = true;
		}
		strTemp += "\n";
		fileout << strTemp;
		//if(found) break;
	}
	filein.close();
	fileout.close();
	remove(filename.c_str());
	int test = rename("fileout.txt", filename.c_str());
	int jam;
	return;
}



//will return a vector of strings that were found after the tag/token requested
std::vector<std::string> FileWriter::ReadData(std::string token)
{
	string buf;
	std::vector<std::string> ret;


	std::ifstream file;
	file.open(filename, std::ios::in);
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
		else if (buf == token)
		{
			//get the requested tag
			file >> buf;
			ret.push_back(buf.c_str());
		}
	}

	file.close();
	return ret;
}