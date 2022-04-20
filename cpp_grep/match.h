#pragma once
#include <string>
#include <vector>
#include "capture.h"
#include "group.h"

namespace rex
{
	using namespace std;

	class Match : public CaptureBase
	{
	private:
		vector<Group> _groups;

	public:
		Group get_group(unsigned short groupnum) const
		{
			if (groupnum >= _groups.size())
			{
				throw "Group number is too high";
			}

			return _groups[groupnum];
		}

		size_t start() const override
		{
			return _groups.empty() ? 0 : _groups[0].start();
		}

		size_t length() const override
		{
			return get_group(0).length();
		}

		string value() const override
		{
			return get_group(0).value();
		}

		/// <summary>
		/// Returns the string value of the specified group number. Group 0 returns the whole match value
		/// </summary>
		/// <param name="groupnum"></param>
		/// <returns></returns>
		string get_group_value(const unsigned short groupnum) const
		{

			if (groupnum >= _groups.size())
				return string();

			else
				return _groups[groupnum].value();
		}

		void add_group_capture(const unsigned short groupNum, const Capture cap)
		{
			while (groupNum >= _groups.size())
				_groups.push_back(Group());

			_groups[groupNum].add_capture(cap);

		}

		void print_all_info(ostream &o) const
		{
			for (size_t i = 0; i < _groups.size(); i++)
			{
				o << "           Group " << i << ": " << endl;
				for (size_t j = 0; j < _groups[i].total_caps(); j++)
				{
					Capture cap = _groups[i].get_capture(j);
					o << "            Capture " << j << ": " << endl;
					o << "                 Value: '" << cap.value() << "'" << endl;
					o << "             Starts At: " << cap.start() << endl;
				}
			}
		}
	};
}
