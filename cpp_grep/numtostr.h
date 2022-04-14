#pragma once
#include <sstream>
#include <string>

namespace rex 
{
	using namespace std;

	template <class T> string toStr(T num)
	{
		ostringstream out_buff;
		out_buff << num;
		return out_buff.str();
	}
}

