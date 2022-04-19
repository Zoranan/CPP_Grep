#pragma once
#include "capture.h"
#include "defines.h"
#include <vector>

namespace rex
{
	using namespace std;

	class Group : public CaptureBase
	{
	private:
		vector<Capture> _captures;

	public:
		Capture get_capture(const size_t capnum) const
		{
			return _captures[capnum];
		}

		void add_capture(const Capture cap)
		{
			_captures.push_back(cap);
		}

		size_t start() const override
		{
			return _captures.empty() ? 0 : _captures[0].start();
		}

		string value() const override
		{
			string t;
			for (size_t i = 0; i < _captures.size(); i++)
			{
				t.append(_captures[i].value());
			}
			return t;
		}

		size_t length() const override
		{
			size_t l = 0;
			for (size_t i = 0; i < _captures.size(); i++)
			{
				l += _captures[i].length();
			}
			return l;
		}

		size_t total_caps() const
		{
			return _captures.size();
		}

		Capture last_capture() const
		{
			if (_captures.size() == 0)
				return Capture(0, "");

			return _captures[_captures.size() - 1];
		}

		string last_capture_value() const
		{
			last_capture().value();
		}
	};
}
