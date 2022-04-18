#pragma once
#include <string>

namespace rex 
{
	using namespace std;

	class CaptureBase
	{
	public:

		virtual size_t start() = 0;

		virtual string value() = 0;

		virtual size_t length() = 0;

		size_t end()
		{
			return start() + length();
		}
	};

	class Capture : public CaptureBase
	{
	protected:
		string _value;
		size_t _start;

	public:
		Capture(size_t s = 0, string val = "")
		{
			_start = s;
			_value = val;
		}

		size_t start() override
		{
			return _start;
		}

		string value() override
		{
			return _value;
		}

		size_t length() override
		{
			return _value.length();
		}
	};
}
