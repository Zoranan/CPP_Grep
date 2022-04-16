// tgrep.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "regex.h"

#define ARGC_OFFSET 1

using namespace std;
using namespace rex;

void supress_output(string& line, Match &m)
{
	// Do nothing
}

void print_output(string& line, Match &m)
{
	cout << line << endl;
}

void print_full_match_info(string& line, Match& m)
{
	m.print_all_info(cout);
	cout << endl;
}

int process_matches(string& pattern, istream& stream, void (*func)(string&, Match&), unsigned int max = 0)
{
	unsigned int count = 0;
	size_t lineNum = 0;
	string line;

	try
	{
		Regex reg(pattern);

		while (std::getline(stream, line) && (!max || count < max))
		{
			//Line was read, now see if it matches our pattern
			Match m;
			if (reg.try_match(line, m))
			{
				count++;
				lineNum++;
				cout << lineNum << ": " << line << endl;
				func(line, m);	//TODO: Accept multiple funcs
			}
		}
	}
	catch (exception e)
	{
		cerr << e.what() << endl;
	}
	catch (string s)
	{
		cerr << s << endl;
	}
	return count;
}

int main(int argc, char* argv[])
{
	// No args entered
	if (argc == ARGC_OFFSET)
	{
		cerr << "Help Screen HERE" << endl;
		//Print help here
		return 0;
	}
	else
	{			
		// Grab our pattern to match with
		// TODO: This should be a regex
		string pattern = string(argv[ARGC_OFFSET]);


		// Only one arg was specified, so we must be reading from cin
		if (argc == ARGC_OFFSET + 1)
		{
			process_matches(pattern, cin, &print_full_match_info);
		}

		// At least one file arg was specified after the pattern arg
		// Loop through the files, open them, and pass the streams to our matching function
		else
		{
			// Start with our second arg and loop though them all
			for (int i = ARGC_OFFSET + 1; i < argc; i++)
			{
				//TODO: Try using file patterns to match multiple files?
				//		On guardian we might want to throw a warning if /in/ or /inv/ was specified

				ifstream fstr(argv[i]);	// Default open mode is 1 (in)
				process_matches(pattern, fstr, &print_full_match_info);
				fstr.close();
			}
		}
	}

	return 0;
}
