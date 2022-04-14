#pragma once
#include <string>

namespace rex 
{
	using namespace std;

	class Capture
	{
	protected:
		string _value;
		size_t _start;
		size_t _len;

	public:
		Capture()
		{
			_value = "";
			_start = 0;
			_len = 0;
		}

		Capture(size_t s, size_t l = 0)
		{
			_start = s;
			set_length(l);
		}

		void take_capture(string& str)
		{
			_value = str.substr(_start, _len);
		}

		void done(string& source)
		{
			_value = source.substr(_start, _len);
		}

		void set_length(size_t l)
		{
			_len = l;
		}

		void set_start(size_t s)
		{
			_start = s;
		}

		virtual string value()
		{
			return _value;
		}

		size_t start()
		{
			return _start;
		}

		size_t length()
		{
			return _len;
		}

		size_t end()
		{
			return start() + length();
		}
	};
}
