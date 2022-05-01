#pragma once
#include <string>
#include <vector>
#include <stack>
#include "utils.h"
#include "defines.h"
#include "MatchState.h"

namespace rex 
{
	using namespace std;

	////////////////////
	// Base Atom
	////////////////////

	class Atom
	{

	protected:
		Atom* _next;
		unsigned int _min_length;

		/// <summary>
		/// Try to match on our next atom (if it exists) and reset it if the match fails
		/// </summary>
		/// <param name="current_result">The current distance this atom traveled</param>
		/// <param name="m">The match object we're working with</param>
		/// <param name="str">The string we are matching against</param>
		/// <param name="start_pos">The position this atom started at (not including its current result</param>
		/// <returns>The result of all downstream atoms</returns>
		int try_next(int current_result, const char* str, size_t strSize, size_t start_pos, MatchState& state)
		{
			if (_next == nullptr)
			{
				return current_result;
			}

			int nextRes = _next->try_match(str, strSize, start_pos + current_result, state);
			if (nextRes > -1)
			{
				return current_result + nextRes;
			}

			return -1;
		}

	public:
		Atom(Atom* next = nullptr, unsigned int min_len = 0)
		{
			_next = next;
			_min_length = min_len;
		}

		/// <summary>
		/// Recursively append 'n' to the last atom in the sequence
		/// </summary>
		/// <param name="n">A pointer to the Atom to append</param>
		virtual void append(Atom* n)
		{
			if (_next == nullptr)
			{
				_next = n;
			}

			else
				_next->append(n);
		}

		virtual unsigned int min_length()
		{
			if (_next != nullptr)
			{
				return _next->min_length() + _min_length;
			}
			else
				return _min_length;
		}

		virtual void findGroupNums(vector<unsigned short>& grps)
		{
			if (_next != nullptr)
				_next->findGroupNums(grps);
		}

		/// <summary>
		/// Attempt to match this atom agains the input string at the start_pos
		/// </summary>
		/// <param name="m"></param>
		/// <param name="str">The input string to match against</param>
		/// <param name="start_pos">The position in the string to match against</param>
		/// <returns>The number of characters this and all downstream atoms traveled, or -1 for failures.</returns>
		virtual int try_match(const char* str, size_t strSize, size_t start_pos, MatchState &state) = 0;

		virtual ~Atom()
		{
			if (_next != nullptr)
				delete _next;
		}
	};


	////////////////////
	// Character Atoms
	////////////////////

	/// <summary>
	/// Match a single character
	/// </summary>
	class CharLiteral : public Atom
	{
	private:
		unsigned char _char;
		bool _caseSensitive;

	public:
		CharLiteral()
		{
			_char = '\0';
			_caseSensitive = true;
		}

		CharLiteral(unsigned char c, bool caseSensitive = true, Atom* next = nullptr) : Atom(next, 1)
		{
			_char = !caseSensitive ? tolower_u(c) : c;
			_caseSensitive = caseSensitive;
		}

		int try_match(const char* str, size_t strSize, size_t start_pos, MatchState& state) override
		{
			if (start_pos >= strSize)
				return -1;
			
			unsigned char c = static_cast<unsigned char>(str[start_pos]);
			if (!_caseSensitive && isupper_u(c))
				c = tolower_u(c);

			// This atom succeeded
			if (c == _char)
			{
				return try_next(1, str, strSize, start_pos, state);
			}

			return -1;
		}
	};

	/// <summary>
	/// Match any character within the given range
	/// </summary>
	class CharRange : public Atom
	{
	private:
		unsigned char _min;
		unsigned char _max;

	public:
		CharRange(unsigned char min, unsigned char max, Atom* next = nullptr) : Atom(next, 1)
		{
			_min = min;
			_max = max;
		}

		int try_match(const char* str, size_t strSize, size_t start_pos, MatchState& state) override
		{
			if (start_pos >= strSize)
				return -1;

			unsigned char c = str[start_pos];

			//This Atom succeeded
			if (_min <= c && c <= _max)
			{
				return try_next(1, str, strSize, start_pos, state);
			}

			// All success branches should have returned out by now. If we're here, then this Atom or one of its sub atoms failed
			return -1;
		}
	};

	/// <summary>
	/// Match any single character (Including \r\n until multiline/singleline options are implemented)
	/// </summary>
	class AnyChar : public Atom
	{
		int try_match(const char* str, size_t strSize, size_t start_pos, MatchState& state) override
		{
			if (start_pos < strSize)
			{
				return try_next(1, str, strSize, start_pos, state);
			}

			return -1;
		}
	};

	/// <summary>
	/// Inverts the result of a SINGLE match atom only. Using this for atoms that match variable lengths of chars will cause undefined behavior
	/// Currently this is broken and causes the program to hang... need to research this
	/// </summary>
	class InversionAtom : public Atom
	{
	private:
		Atom* _atom;
		int _step;
	public:
		InversionAtom() : Atom(nullptr, 1) 
		{ 
			_atom = nullptr; 
			_step = 1;
		}
		InversionAtom(Atom* a, int step) : Atom(nullptr, static_cast<unsigned int>(_step))
		{
			_atom = a;
			_step = step;
		}

		int try_match(const char * str, size_t strSize, size_t start_pos, MatchState& state) override
		{
			if (start_pos >= strSize)
				return -1;

			int r = _atom->try_match(str, strSize, start_pos, state);
			if (r > -1)
				return -1;

			int fin = try_next(_step, str, strSize, start_pos, state);

			return fin;
		}

		~InversionAtom()
		{
			delete _atom;
		}
	};

	////////////////////
	// Combiner Atoms
	////////////////////

	/// <summary>
	/// Try each branch until one succeeds, then tries to rest of the atoms downstream until one of the branches and the downstream atoms are able to match.
	/// </summary>
	class OrAtom : public Atom
	{
	private:
		vector<Atom*> _atoms;
		//vector<vector<unsigned short>> _branch_groups;

	protected:
		OrAtom(Atom* next = nullptr) : Atom(next) {}


	public:

		OrAtom(vector<Atom*> a)
		{
			_atoms = a;
			_min_length = UINT32_MAX;
			for (size_t i = 0; i < _atoms.size(); i++)
			{
				unsigned int m = _atoms[i]->min_length();
				if (m < _min_length)
					_min_length = m;

				vector<unsigned short> grps;
				_atoms[i]->findGroupNums(grps);
			}
		}

		void append(Atom* next) override
		{
			// Only append next to each branch if this is the first Atom after the OR.
			if (_next == nullptr)
			{
				for (size_t i = 0; i < _atoms.size(); i++)
				{
					_atoms[i]->append(next);
				}

				_next = next;
			}
			else
			{
				_next->append(next);
			}
			
		}

		int try_match(const char* str, size_t strSize, size_t start_pos, MatchState& state) override
		{
			for (size_t i = 0; i < _atoms.size(); i++)
			{
				int r = _atoms[i]->try_match(str, strSize, start_pos, state);

				// This branch succeeded
				if (r > -1)
				{
					return r;
				}
			}

			// No branches succeeded
			return -1;
		}

		~OrAtom()
		{
			for (size_t i = 0; i < _atoms.size(); i++)
			{
				delete _atoms[i];
			}
		}
	};

	/////////////////////
	// Quantifier Atoms
	/////////////////////

	class QuantifierBase : public Atom
	{
	protected:
		Atom* _atom;
		unsigned int _min;
		unsigned int _max;
		vector<unsigned short> _sub_groups;

		unsigned int min_length() override
		{
			return (_atom->min_length() * _min) + Atom::min_length();
		}

	public:
		QuantifierBase(Atom* a, unsigned int min, unsigned int max) : Atom(nullptr, 0)
		{
			_atom = a;
			_min = min;
			_max = max;
			a->findGroupNums(_sub_groups);
		}

		virtual ~QuantifierBase()
		{
			delete _atom;
		}
	};

	/// <summary>
	/// A quantifier that attempts to match as much as it can, and only stops when it's inner Atom fails, or its maximum match count is reached
	/// </summary>
	class GreedyQuantifier : public QuantifierBase
	{

	public:
		GreedyQuantifier(Atom* a, unsigned int min, unsigned int max) : QuantifierBase(a, min, max) { }

		int try_match(const char* str, size_t strSize, size_t start_pos, MatchState& state) override
		{
			// First, get the maximum start_pos that our inner atom could match, so we can work our way backwards
			stack<size_t> end_positions;
			end_positions.push(start_pos);
			size_t last_end_pos = start_pos;


			while (end_positions.size() <= _max)
			{
				int r = _atom->try_match(str, strSize, last_end_pos, state);
				if (r > -1)
				{
					last_end_pos += r;
					end_positions.push(last_end_pos);
				}
				else
				{
					break;
				}
			}

			// We didn't hit the minimum: failure
			if (end_positions.size() <= _min)	//Less than or equal to account for the starting value on the stack
			{
				for (size_t i = 0; i < _sub_groups.size(); i++)
				{
					state.resetGroup(_sub_groups[i]);
				}

				return -1;
			}

			// We found the most greedy match possible and met the minimum
			// If we don't have a next, then return success
			int fin = try_next(static_cast<int>(end_positions.top() - start_pos), str, strSize, start_pos, state);	//NOTE: Always pass in the unmodified start position, because this method adds the current position to it!
			
			if (_next != nullptr)
				while (fin == -1)
				{
					if (end_positions.size() - 1 <= _min)	//Subtract one to account for starting value on stack. <= to account for the item we will pop off next
					{
						return -1;
					}

					end_positions.pop();

					//Remove any sub captures that we may gave picked up
					for (size_t i = 0; i < _sub_groups.size(); i++)
					{
						state.popCapture(_sub_groups[i]);
					}

					fin = try_next(static_cast<int>(end_positions.top() - start_pos), str, strSize, start_pos, state);
				}

			if (fin == -1)
				for (size_t i = 0; i < _sub_groups.size(); i++)
				{
					state.resetGroup(_sub_groups[i]);
				}

			return fin;
		}
	};

	/// <summary>
	/// A quantifier that only attempts to match it's inner atom until it reaches the minimum required count, and its _next Atom is successful
	/// </summary>
	class LazyQuantifier : public QuantifierBase
	{

	public:
		LazyQuantifier(Atom* a, unsigned int min, unsigned int max) : QuantifierBase(a, min, max) {}

		int try_match(const char* str, size_t strSize, size_t start_pos, MatchState& state) override
		{
			if (_max <= 0)
				return -1;

			int r = 0;	//last match length
			int i = 0;	//Total length matched
			size_t c = 0;	//Total match count

			while (true)
			{
				//If we've done the minimum matching needed, check our next atom and return out if success
				if (c >= _min)
				{
					int fin = try_next(i, str, strSize, start_pos, state);	//_next is auto reset here on failure
					if (fin > -1)
					{
						return fin;
					}
				}

				// We're already maxed out and should have returned by now if the next atom was successful. So return failure
				if (c >= _max)
				{
					return -1;
				}

				// We have not reached max yet, try our inner atom again

				r = _atom->try_match(str, strSize, start_pos + i, state);
				if (r > -1)
				{
					c++;
					i += r;
				}
				//We can't match our inner atom, so we must fail
				else
				{
					return -1;
				}
			}
		}
	};

	////////////////////////
	// Special matches
	////////////////////////

	/// <summary>
	/// Matches the beginning of input
	/// </summary>
	class BeginStringAtom : public Atom
	{
	public:

		int try_match(const char* str, size_t strSize, size_t start_pos, MatchState& state) override
		{
			if (start_pos == 0)
				return try_next(0, str, strSize, start_pos, state);

			return -1;
		}
	};

	/// <summary>
	/// Matches the end of input
	/// </summary>
	class EndStringAtom : public Atom
	{
	public:
		int try_match(const char* str, size_t strSize, size_t start_pos, MatchState& state) override
		{
			if (start_pos == strSize)
				return try_next(0, str, strSize, start_pos, state);

			return -1;
		}
	};

	/// <summary>
	/// Matches the beginning of the current line or the beginning of the input
	/// </summary>
	class BeginLineAtom : public Atom
	{
	public:
		int try_match(const char * str, size_t strSize, size_t start_pos, MatchState& state) override
		{
			if (start_pos == 0
				|| (start_pos - 1 < strSize && str[start_pos - 1] == '\n')
				//|| (start_pos + 1 < str.length() && str[start_pos + 1] == '\r')
				)
				return try_next(0, str, strSize, start_pos, state);

			return -1;
		}
	};

	/// <summary>
	/// Matches the end of the current line or the end of the input
	/// </summary>
	class EndLineAtom : public Atom
	{
	public:
		int try_match(const char* str, size_t strSize, size_t start_pos, MatchState& state) override
		{
			if (start_pos >= strSize || (start_pos + 1 < strSize && (str[start_pos + 1] == '\n' || str[start_pos + 1] == '\r')))
				return try_next(0, str, strSize, start_pos, state);

			return -1;
		}
	};

	class WordBoundary : public Atom
	{
	private:
		bool is_word_char(unsigned char c)
		{
			return c == '-' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
		}

	public:
		int try_match(const char * str, size_t strSize, size_t start_pos, MatchState& state) override
		{
			bool is1aWord = false;
			bool is2aWord = false;

			if (start_pos > 0)
				is1aWord = is_word_char(str[start_pos - 1]);

			if (start_pos < strSize)
				is2aWord = is_word_char(str[start_pos]);

			if (is1aWord != is2aWord)
				return try_next(0, str, strSize, start_pos, state);

			return -1;
		}
	};

	/// <summary>
	/// Stores capture group information at the current position and tries downstream atoms to determine if the capture be kept or thrown away
	/// </summary>
	class GroupStart : public Atom
	{
	private:
		unsigned short _group_num;

		void findGroupNums(vector<unsigned short> &grps) override
		{
			grps.push_back(_group_num);
			Atom::findGroupNums(grps);
		}

	public:
		GroupStart(unsigned short groupNum, Atom* next = nullptr) : Atom(next) 
		{
			_group_num = groupNum;
		}

		unsigned short group_num() const
		{
			return _group_num;
		}

		int try_match(const char* str, size_t strSize, size_t start_pos, MatchState& state) override
		{
			// Place down a starting capture point and try the next atom
			state.startNewCapture(_group_num, start_pos);
			int r = try_next(0, str, strSize, start_pos, state);
			
			// If it failed somewhere down the line, pop that capture back off
			if (r < 0)
				state.popCapture(_group_num);

			return r;
		}
	};

	/// <summary>
	/// Marks the end of a capturing group. If atoms downstream from this succeed, it calls back to the starting atom to update its capture length
	/// </summary>
	class GroupEnd : public Atom
	{
	private:
		unsigned short _group_num;

	public:
		GroupEnd(unsigned short group_num)
		{
			_group_num = group_num;
		}

		int try_match(const char * str, size_t strSize, size_t start_pos, MatchState& state) override
		{
			int r = try_next(0,str, strSize, start_pos, state);

			if (r > -1)
				state.end_capture(_group_num, start_pos);

			return r;
		}
	};
}

