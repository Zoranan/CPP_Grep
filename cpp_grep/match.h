#pragma once
#include <string>
#include <vector>
#include "capture.h"
#include "group.h"

namespace rex
{
	using namespace std;

	class Match : public Capture
	{
	private:
		vector<Group> _groups;

	public:
		Group get_group(unsigned int groupnum)
		{
			if (groupnum < 1)
			{
				throw "Group number must be > 1";
			}

			else if (groupnum > _groups.size())
			{
				throw "Group number is too high";
			}

			return _groups[groupnum - 1];
		}

		/// <summary>
		/// Returns the string value of the specified group number. Group 0 returns the whole match value
		/// </summary>
		/// <param name="groupnum"></param>
		/// <returns></returns>
		string get_group_value(unsigned int groupnum)
		{
			if (groupnum == 0)
				return value();

			else if (groupnum > _groups.size())
				return string();

			else
				return _groups[groupnum - 1].value();
		}

		void add_group_capture(unsigned int groupNum, Capture cap)
		{
			if (groupNum == 0)
				throw "Invalid group number: 0";

			//Group num is one higher than the actual max index since they start at 1 instead of 0

			while (groupNum > _groups.size())
				_groups.push_back(Group());

			_groups[groupNum - 1].add_capture(cap);

			//Recalculate the match length
			size_t end = cap.start() + cap.length();
			if (end < _start + _len)
				_len = end - _start;
		}

		void print_all_info(ostream &o)
		{
			o << "Whole Match (0): '" << _value << "'" << endl;
			o << "      Starts At: " << _start << endl;
			for (size_t i = 0; i < _groups.size(); i++)
			{
				o << "           Group " << i + 1 << ": " << endl;
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
