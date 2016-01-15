#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<algorithm>
#include<time.h>
#include<map>
using namespace std;
const int total_blocks = 30;
string filename = "D:/Books/OS/Project 1/FS/FS/fusedata.";
string ext;
string fileopen(string ext)                                         //Open a file and read it
{
	ifstream inputFile;
	string data, strcpy;
	inputFile.open(filename + ext);

	if (!inputFile.is_open())
	{
		cerr << "Error Opening File" << endl;
	}

	else {
		do
		{
			inputFile >> data;
			strcpy.append(data);
		} while (!inputFile.eof());
		return strcpy;
	}
	inputFile.close();
}
string split1(string info)                                           //Remove one pair of curly brackets from a file
{
	if (info[0] == '{')
	{
		info.erase(0, 1);
		info.erase((info.size() - 1), 1);
	}
	return info;
}
vector<string> split2(string info, char symbol, bool spaces = true)  //Remove symbols like comma, colons, etc., when required
{
	vector<string> strparts;
	string str;
	int count = 0;
	int info_size = info.size();
	for (int i = 0; i < info_size; ++i)
	{
		if (info[i] == symbol || i == info.size() - 1)
		{
			if (i == info.size() - 1)
			{
				str = info.substr(count);
			}
			else
			{
				str = info.substr(count, i - count);
			}
			if (spaces)
			{
				int str_size = str.size();
				for (int i = 0; i < str_size; ++i)
				{
					if (str[i] == ' ')
					{
						str.erase(i, 1);
					}
				}
			}
			strparts.push_back(str);
			count = i + 1;
		}
	}
	return strparts;
}
bool DeviceID(vector<string> strparts)                               //Check Device ID = 20 for file fusedata.0
{
	for each (string part in strparts)
	{
		vector<string> strpart1 = split2(part, ':', false);
		if (strpart1[0] == "devId")
		{
			if (strpart1[1] == "20")
				return true;
			else
				cout << "Incorrect DeviceID\n";
		}
		else
			continue;
		return false;
	}
	return false;
}

void createfreeblocklist(vector<string> &freeblocks, vector<int> &freeblocklist)
{
	for (int ext = 1; ext < 26; ++ext)
	{
		string strcpy1 = fileopen(to_string(ext));
		if (!strcpy1.empty())
		{
			vector<string> freeblock = split2(strcpy1, ',');
			string filenum_str = to_string(ext);
			for each(string free in freeblock)
			{
				freeblocks.push_back(free);
				freeblocklist.push_back(stoi(free));
			}
		}
	}
}

bool AllTimes(vector<string> strparts, int fileno)                   //All Times should be in the past
{
	time_t t = time(0);                    //current time
	for each(string part in strparts)
	{
		vector<string> strpart1 = split2(part, ':', false);
		if (strpart1[0] == "atime" || strpart1[0] == "ctime" || strpart1[0] == "mtime" || strpart1[0] == "creationTime")
		{
			int t_temp = stoi(strpart1[1]);
			if (t_temp < t)
				continue;
		}
		else
			continue;
		cout << "In fusedata." + to_string(fileno) + ", " + strpart1[0] + " is incorrect.\n";
		return false;
	}
	return true;
}

bool FreeBlock(vector<string> strparts, int fileno, vector<int> &freeblocklist, vector<string> &freeblocks)     //Check if the current file is a Free Block
{
	if (!strparts.empty())
		return true;

	for each(int block in freeblocklist)
	{
		if (block == fileno)
			return true;
		else
			continue;
	}
	cout << "Free Block List does not contain Free Block " + to_string(fileno) + ".\n";
	return false;
}

bool Directory(string strcpy, int fileno)          //Check if Directory entries are correct
{
	vector<string> strparts;
	strparts = split2(strcpy, '{');
	if (strparts.size() <= 1)
		return true;
	vector<string> inodeDirectory = split2(strparts[1].substr(0, strparts[1].size() - 1), ',');
	for each (string field in inodeDirectory)
	{
		vector<string> subpart = split2(field, ':');
		if (subpart[0] == "d")
		{
			if (subpart[1] == ".")                  //Current Directory
			{
				if (stoi(subpart[2]) != fileno)
					cout << "In fusedata." + to_string(fileno) + ", the Current Directory d:. is incorrect.\n";
				else
					continue;
			}
			if (subpart[1] == "..")                 //Parent Directory
			{
				string strcpy1 = fileopen(subpart[2]);
				//vector<string> strparts;
				//strparts.push_back(split1(strcpy));
				vector<string> strparts1 = split2(strcpy1, '{', false);
				if (strparts1.size() <= 1)
				{
					cout << "In fusedata." + to_string(fileno) + ", the Parent d.. is not a directory.\n";
					return false;
				}
				int pos = strparts1[1].find(to_string(fileno));
				if (pos == -1)
				{
					cout << "In fusedata." + to_string(fileno) + ", the Parent Directory d:.. is incorrect.\n";
					return false;
				}
				return true;
			}
		}
		else
			continue;
	}
	return false;
}

bool LinkCount(string strcpy, vector<string> strparts, int fileno)    //Check Link Count
{//Each directory’s link count matches the number of links in the filename_to_inode_dict
	vector<string> str;
	str = split2(strcpy, '{', false);
	if (str.size() <= 1)
		return true;
	vector<string> str1 = split2(str[1].substr(0, str[1].size() - 1), ',');
	int count = str1.size();
	int linkcount;

	for each(string part in strparts)
	{
		vector<string> subpart = split2(part, ':');
		if (subpart[0] == "linkcount")
		{
			linkcount = stoi(subpart[1]);
			if (linkcount == count)
				return true;
			else
			{
				cout << "Incorrect Link Count.";
				cout << " In fusedata." + to_string(fileno) + ", the Link Count of should be " + to_string(count) + ", instead of " + to_string(linkcount) + ".\n";
				return false;
			}
		}
		else
			continue;
	}
	cout << "could not find a linkcount field in the file" << endl;
	return false;
}

bool Indirect(string strcpy, int &indirect, vector<string> &locationarray, int fileno)  //Check Indirect
{
	vector<string> str;
	str = split2(strcpy, '{', false);
	vector<string> strparts = split2(strcpy, ',');
	for each(string part in strparts)
	{
		vector<string> subpart = split2(part, ':');
		if (subpart[0] == "indirect")
		{
			size_t location = subpart[1].find("location");
			indirect = stoi(subpart[1].substr(0, location));
			if (indirect == 0 && str.size() <= 1)
			{
				return true;
			}
			else if (indirect == 1 && str.size() > 1)                    //If the data contained in a location pointer is an array, indirect is one
			{
				locationarray = split2(str[1].substr(0, str[1].size() - 1), ',');
				return true;
			}
			else
			{
				cout << "In fusedata." + to_string(fileno) + ", the indirect of " + to_string(indirect) + " stored on the file does not have location pointer array." << endl;
				return false;
			}
		}
		else
			continue;
	}
	return false;
}

bool Size(vector<string> strparts, int &indirect, vector<string> &locationarray, int fileno)  //Check Size
{	//size is valid for the number of block pointers in the location array 
	int size = 0;
	for each(string part in strparts)
	{
		vector<string> subpart = split2(part, ':');
		if (subpart[0] != "size")
			continue;
		size = stoi(subpart[1]);
		if (size <= 0 || indirect < 0)
			return false;
		if (indirect == 0 && size <= 4096)                                       //size < blocksize, if  indirect = 0 and size > 0
			return true;
		else if (indirect != 0 && size < 4096 * locationarray.size())            //size < blocksize * length of location array, if indirect != 0
			return true;
		else if (indirect != 0 && size > 4096 * (locationarray.size() - 1))      //size > blocksize * (length of location array - 1), if indirect != 0
			return true;
		else
		{
			cout << "In fusedata." + to_string(fileno) + ", the size of " + to_string(size) + " is invalid for the number of block pointers in the location array.\n";
			return false;
		}
	}
	return false;
}

bool FreeBlockList(vector<string> &freeblocks)                  //Check whether all entries in Free Block List are valid or not
{
	string prev = "0";
	int iprev = stoi(prev);
	for each(string block in freeblocks)
	{
		if (stoi(block) <= total_blocks)                        //Check all the blocks in Free Block List are empty
		{
			string strcpy = fileopen(block);

			if (!strcpy.empty())
			{
				cout << "File fusedata." + block + " in the Free Block List is not empty.\n";
			}
		}
		if (!iprev == 0)
		{
			if (!(iprev + 1 == stoi(block)))                    //Check Free Block List contains all the Free Blocks
			{
				ifstream inputFile;
				inputFile.open(filename + ext);
				if (!inputFile.is_open())
				{
					int freeblock = stoi(block) - iprev;
					for (int i = 0; i < freeblock - 1; i++)
					{
						iprev = iprev + 1;
						cout << "The Free Block List does not contain Free Block " << iprev << ".\n";
					}
				}
			}
		}
		iprev = stoi(block);
	}
	return true;
}

void main()
{
	string strcpy, strcpy1;
	int indirect;
	vector<string> locationarray;
	vector<string> freeblocks;
	vector<int> freeblocklist;
	strcpy = fileopen("0");
	strcpy = split1(strcpy);
	vector<string> strparts = split2(strcpy, ',');
	DeviceID(strparts);                                             //Check Device ID
	strcpy.clear();
	strparts.clear();
	createfreeblocklist(freeblocks, freeblocklist);                 //Create Free Block List
	for (int fileno = 26; fileno < 31; ++fileno)
	{
		strcpy = fileopen(to_string(fileno));            
		strcpy = split1(strcpy);
		vector<string> strparts = split2(strcpy, ',');
		AllTimes(strparts, fileno);                                //Check creation time, atime, ctime and mtime
		FreeBlock(strparts, fileno, freeblocklist, freeblocks);    //Check for Free Blocks not present in Free Block List
		Directory(strcpy, fileno);                                 //Check values for Parent Directory and Current Directory
		LinkCount(strcpy, strparts, fileno);                       //Check value of Link Count 
		Indirect(strcpy, indirect, locationarray, fileno);         //Check if Indirect value is valid 
		Size(strparts, indirect, locationarray, fileno);           //Check if Size is correct
		cout << endl;
		indirect = -1;
		locationarray.clear();
	}
	FreeBlockList(freeblocks);                                     //Check that Free Block List contains all the Free Blocks, only
	cin.get();
}
