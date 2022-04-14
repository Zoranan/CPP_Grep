#pragma once
#include "capture.h"
#include <vector>

namespace rex
{
	using namespace std;

	class Group : public Capture
	{
	private:
		vector<Capture> _captures;
	public:
		Group(size_t s = 0, size_t l = 0) : Capture(s, l)
		{

		}

		Capture get_capture(size_t capnum)
		{
			return _captures[capnum];
		}

		void add_capture(Capture cap)
		{
			if (_captures.size() == 0)
				_start = cap.start();

			_captures.push_back(cap);

			_len = cap.start() + cap.end() - _start;	//Captures should always be added from left to right
		}

		string value() override
		{
			string t;
			for (size_t i = 0; i < _captures.size(); i++)
			{
				t.append(_captures[i].value());
			}
			return t;
		}

		size_t total_caps()
		{
			return _captures.size();
		}

		Capture last_capture()
		{
			if (_captures.size() == 0)
				return Capture(0, 0);

			return _captures[_captures.size() - 1];
		}

		string last_capture_value()
		{
			last_capture().value();
		}
	};
}
