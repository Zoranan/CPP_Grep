#pragma once
#include <sstream>
#include <string>

namespace rex 
{
	using namespace std;

	bool isdigit_u(unsigned char c)
	{
		return c >= '0' && c <= '9';
	}

	bool islower_u(unsigned char c)
	{
		return c >= 'a' && c <= 'z';
	}

	bool isupper_u(unsigned char c)
	{
		return c >= 'A' && c <= 'Z';
	}

	bool isalpha_u(unsigned char c)
	{
		return isupper_u(c) || islower_u(c);
	}

	unsigned char tolower_u(unsigned char c)
	{
		return isupper_u(c) ? static_cast<unsigned char>(c + 32) : c;
	}

	unsigned char tolower_u(char c)
	{
		unsigned char c2 = static_cast<unsigned char>(c);
		return tolower_u(c2);
	}

	template <class T> string toStr(T num)
	{
		ostringstream out_buff;
		out_buff << num;
		return out_buff.str();
	}
}

