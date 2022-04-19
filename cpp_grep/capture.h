#pragma once
#include <string>

namespace rex 
{
	using namespace std;

	class CaptureBase
	{
	public:

		virtual size_t start() const = 0;

		virtual string value() const = 0;

		virtual size_t length() const = 0;

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
		Capture(const size_t s = 0, const string val = "")
		{
			_start = s;
			_value = val;
		}

		size_t start() const override
		{
			return _start;
		}

		string value() const override
		{
			return _value;
		}

		size_t length() const override
		{
			return _value.length();
		}
	};
}
