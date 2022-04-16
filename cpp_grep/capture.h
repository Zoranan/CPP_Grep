#pragma once
#include <string>

namespace rex 
{
	using namespace std;

	class CaptureBase
	{
	protected:
		size_t _start;

	public:
		CaptureBase(size_t s = 0)
		{
			_start = s;
		}

		void set_start(size_t s)
		{
			_start = s;
		}

		size_t start()
		{
			return _start;
		}

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

	public:
		Capture(size_t s = 0, string val = "")
		{
			_value = val;
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
