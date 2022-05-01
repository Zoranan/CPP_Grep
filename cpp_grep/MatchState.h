#pragma once
#include <vector>
#include "capture.h"
#include "match.h"


namespace rex 
{
	using namespace std;

	class MatchState
	{
	private:
		struct PendingCap
		{
		public:
			size_t start_pos;
			size_t length;

			PendingCap(size_t start = 0, size_t len = 0)
			{
				start_pos = start;
				length = len;
			}

			Capture finish(const string& str)
			{
				return Capture(start_pos, str.substr(start_pos, length));
			}

			void set_end_pos(size_t end_pos)
			{
				length = end_pos - start_pos;
			}
		};

		struct PendingCap;
		vector<vector<PendingCap>> _groupCaps;

	public:
		MatchState(unsigned short groupCount = 1)
		{
			_groupCaps = vector<vector<PendingCap>>(groupCount);
		}

		void startNewCapture(unsigned short group, size_t startPos);
		void resetGroup(unsigned short group);
		void popCapture(unsigned short group);
		void end_capture(unsigned short group, size_t end_pos);
		void commit(Match& m, const string& str);
		void reset();
	};
}

