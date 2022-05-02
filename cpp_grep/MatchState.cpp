#include "MatchState.h"

namespace rex
{
	void MatchState::startNewCapture(unsigned short group, size_t startPos)
	{
		_groupCaps[group].push_back(PendingCap(startPos));
	}

	void MatchState::resetGroup(unsigned short group)
	{
		_groupCaps[group].clear();
	}

	void MatchState::popCapture(unsigned short group)
	{
		_groupCaps[group].resize(_groupCaps[group].size() - 1);
	}

	void MatchState::end_capture(unsigned short group, size_t end_pos)
	{
		_groupCaps[group].back().set_end_pos(end_pos);
	}

	void MatchState::commit(Match& m, const string& str)
	{
		for (unsigned short i = 0; i < _groupCaps.size(); i++)
		{
			for (unsigned short j = 0; j < _groupCaps[i].size(); j++)
			{
				m.add_group_capture(i, _groupCaps[i][j].finish(str));
			}
		}
		reset();
	}

	void MatchState::reset()
	{
		for (size_t i = 0; i < _groupCaps.size(); i++)
		{
			_groupCaps[i].clear();
		}
	}
}
