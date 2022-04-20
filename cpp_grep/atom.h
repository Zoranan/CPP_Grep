#pragma once
#include <string>
#include <vector>
#include <stack>
#include "match.h"
#include "utils.h"
#include "defines.h"

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

		/// <summary>
		/// Try to match on our next atom (if it exists) and reset it if the match fails
		/// </summary>
		/// <param name="current_result">The current distance this atom traveled</param>
		/// <param name="m">The match object we're working with</param>
		/// <param name="str">The string we are matching against</param>
		/// <param name="start_pos">The position this atom started at (not including its current result</param>
		/// <returns>The result of all downstream atoms</returns>
		int try_next(int current_result, const string& str, size_t start_pos)
		{
			if (_next == nullptr)
			{
				return current_result;
			}

			int nextRes = _next->try_match(str, start_pos + current_result);
			if (nextRes > -1)
			{
				return current_result + nextRes;
			}

			_next->reset();	//Reset our next if it exists, but fails
			return -1;
		}

	public:
		Atom(Atom* next = nullptr)
		{
			_next = next;
		}

		/// <summary>
		/// Recursively append 'n' to the last atom in the sequence
		/// </summary>
		/// <param name="n">A pointer to the Atom to append</param>
		void append(Atom* n)
		{
			if (_next == nullptr)
				_next = n;

			else
				_next->append(n);
		}

		/// <summary>
		/// Needed for quantifiers to be able to remove a previous capture from GroupStart atoms
		/// </summary>
		virtual void pop_state()
		{
			
		}

		/// <summary>
		/// Clears any held state data from this atom, and all downstream atoms
		/// </summary>
		virtual void reset()
		{
			if (_next != nullptr)
				_next->reset();
		}

		/// <summary>
		/// Adds all group captures to the match object
		/// </summary>
		/// <param name="m"></param>
		/// <param name="str"></param>
		virtual void commit(Match& m, const string& str)
		{
			if (_next != nullptr)
				_next->commit(m, str);
		}

		/// <summary>
		/// Attempt to match this atom agains the input string at the start_pos
		/// </summary>
		/// <param name="m"></param>
		/// <param name="str">The input string to match against</param>
		/// <param name="start_pos">The position in the string to match against</param>
		/// <returns>The number of characters this and all downstream atoms traveled, or -1 for failures.</returns>
		virtual int try_match(const string& str, size_t start_pos) = 0;

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

		CharLiteral(unsigned char c, bool caseSensitive = true, Atom* next = nullptr) : Atom(next)
		{
			_char = !caseSensitive ? tolower_u(c) : c;
			_caseSensitive = caseSensitive;
		}

		int try_match(const string& str, size_t start_pos) override
		{
			if (start_pos >= str.size())
				return -1;
			
			unsigned char c = static_cast<unsigned char>(str[start_pos]);
			if (!_caseSensitive && isupper_u(c))
				c = tolower_u(c);

			// This atom succeeded
			if (c == _char)
			{
				return try_next(1, str, start_pos);
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
		CharRange(unsigned char min, unsigned char max, Atom* next = nullptr) : Atom(next)
		{
			_min = min;
			_max = max;
		}

		int try_match(const string& str, size_t start_pos) override
		{
			if (start_pos >= str.size())
				return -1;

			unsigned char c = str[start_pos];

			//This Atom succeeded
			if (_min <= c && c <= _max)
			{
				return try_next(1, str, start_pos);
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
		int try_match(const string& str, size_t start_pos) override
		{
			if (start_pos < str.size())
			{
				return try_next(1, str, start_pos);
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
		InversionAtom(Atom* a, int step)
		{
			_atom = a;
			_step = step;
		}

		int try_match(const string& str, size_t start_pos) override
		{
			if (start_pos >= str.size())
				return -1;

			int r = _atom->try_match(str, start_pos);
			if (r > -1)
				return -1;

			int fin = try_next(_step, str, start_pos);

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

	protected:
		OrAtom(Atom* next = nullptr) : Atom(next) {}


	public:

		OrAtom(vector<Atom*> a)
		{
			_atoms = a;
		}

		void reset() override
		{
			for (size_t i = 0; i < _atoms.size(); i++)
			{
				_atoms[i]->reset();
			}

			Atom::reset();
		}

		void commit(Match& m, const string& str) override
		{
			//TODO: Keep track of which branch succeeded and only commit that one
			for (size_t i = 0; i < _atoms.size(); i++)
			{
				_atoms[i]->commit(m, str);
			}

			Atom::commit(m, str);
		}

		int try_match(const string& str, size_t start_pos) override
		{
			for (size_t i = 0; i < _atoms.size(); i++)
			{
				int r = _atoms[i]->try_match(str, start_pos);

				// This branch of the or succeeded
				if (r > -1)
				{
					int fin = try_next(r, str, start_pos);	//_next is auto reset on failure with this call
					if (fin > -1)
					{
						return fin;
					}
					else
						_atoms[i]->reset();	//The part after this branch failed, so reset this branch
				}

				//This branch failed, so reset it
				else
				{
					_atoms[i]->reset();
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

	public:
		QuantifierBase(Atom* a, unsigned int min, unsigned int max)
		{
			_atom = a;
			_min = min;
			_max = max;
		}

		virtual ~QuantifierBase()
		{
			delete _atom;
		}

		void reset() override
		{
			_atom->reset();
			Atom::reset();
		}

		virtual void commit(Match& m, const string& str) override
		{
			_atom->commit(m, str);
			Atom::commit(m, str);
		}
	};

	/// <summary>
	/// A quantifier that attempts to match as much as it can, and only stops when it's inner Atom fails, or its maximum match count is reached
	/// </summary>
	class GreedyQuantifier : public QuantifierBase
	{

	public:
		GreedyQuantifier(Atom* a, unsigned int min, unsigned int max) : QuantifierBase(a, min, max) { }

		int try_match(const string& str, size_t start_pos) override
		{
			// First, get the maximum start_pos that our inner atom could match, so we can work our way backwards
			stack<size_t> end_positions;
			end_positions.push(start_pos);
			size_t last_end_pos = start_pos;


			while (end_positions.size() <= _max)
			{
				int r = _atom->try_match(str, last_end_pos);
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
				reset();
				return -1;
			}

			// We found the most greedy match possible and met the minimum
			// If we don't have a next, then return success
			int fin = try_next(static_cast<int>(end_positions.top() - start_pos), str, start_pos);	//NOTE: Always pass in the unmodified start position, because this method adds the current position to it!
			
			if (_next != nullptr)
				while (fin == -1)
				{
					if (end_positions.size() - 1 <= _min)	//Subtract one to account for starting value on stack. <= to account for the item we will pop off next
					{
						return -1;
					}

					end_positions.pop();
					_atom->pop_state();	//Step this atom's memory back once (only apply's if its a capture group)

					fin = try_next(static_cast<int>(end_positions.top() - start_pos), str, start_pos);
				}

			if (fin == -1)
				reset();

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

		int try_match(const string& str, size_t start_pos) override
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
					int fin = try_next(i, str, start_pos);	//_next is auto reset here on failure
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

				r = _atom->try_match(str, start_pos + i);
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
		int try_match(const string& str, size_t start_pos) override
		{
			if (start_pos == 0)
				return try_next(0, str, start_pos);

			return -1;
		}
	};

	/// <summary>
	/// Matches the end of input
	/// </summary>
	class EndStringAtom : public Atom
	{
	public:
		int try_match(const string& str, size_t start_pos) override
		{
			if (start_pos == str.size())
				return try_next(0, str, start_pos);

			return -1;
		}
	};

	/// <summary>
	/// Matches the beginning of the current line or the beginning of the input
	/// </summary>
	class BeginLineAtom : public Atom
	{
	public:
		int try_match(const string& str, size_t start_pos) override
		{
			if (start_pos == 0
				|| (start_pos - 1 < str.length() && str[start_pos - 1] == '\n')
				//|| (start_pos + 1 < str.length() && str[start_pos + 1] == '\r')
				)
				return try_next(0, str, start_pos);

			return -1;
		}
	};

	/// <summary>
	/// Matches the end of the current line or the end of the input
	/// </summary>
	class EndLineAtom : public Atom
	{
	public:
		int try_match(const string& str, size_t start_pos) override
		{
			size_t s = str.size();

			if (start_pos >= s || (start_pos + 1 < s && (str[start_pos + 1] == '\n' || str[start_pos + 1] == '\r')))
				return try_next(0, str, start_pos);

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
		int try_match(const string& str, size_t start_pos) override
		{
			bool is1aWord = false;
			bool is2aWord = false;

			if (start_pos > 0)
				is1aWord = is_word_char(str[start_pos - 1]);

			if (start_pos < str.length())
				is2aWord = is_word_char(str[start_pos]);

			if (is1aWord != is2aWord)
				return try_next(0, str, start_pos);

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

		vector<PendingCap> _pendingCaps;

	public:
		GroupStart(unsigned short groupNum, Atom* next = nullptr) : Atom(next) 
		{
			_group_num = groupNum;
		}

		unsigned short group_num() const
		{
			return _group_num;
		}

		void end_capture(size_t end_pos)	//NOT length
		{
			_pendingCaps[_pendingCaps.size() - 1].set_end_pos(end_pos);
		}

		string last_capture(const string &str)
		{
			if (_pendingCaps.empty())
				return string("");

			else
			{
				PendingCap cap = _pendingCaps[_pendingCaps.size() - 1];
				return str.substr(cap.start_pos, cap.length);
			}
		}

		void pop_state() override
		{
			_pendingCaps.resize(_pendingCaps.size() - 1);
		}

		void reset() override
		{
			_pendingCaps.clear();	//TODO: Uneeded?
			Atom::reset();
		}

		void commit(Match& m, const string& str) override
		{
			for (size_t i = 0; i < _pendingCaps.size(); i++)
				m.add_group_capture(_group_num, _pendingCaps[i].finish(str));

			Atom::commit(m, str);
		}

		int try_match(const string& str, size_t start_pos) override
		{
			// Place down a starting capture point and try the next atom
			_pendingCaps.push_back(PendingCap(start_pos, 0));
			int r = try_next(0, str, start_pos);
			
			// If it failed somewhere down the line, pop that capture back off
			if (r < 0)
				pop_state();

			return r;
		}
	};

	/// <summary>
	/// Marks the end of a capturing group. If atoms downstream from this succeed, it calls back to the starting atom to update its capture length
	/// </summary>
	class GroupEnd : public Atom
	{
	private:
		/// <summary>
		/// This pointer is not owned by this object. It is just a back reference, so it should not be deleted from here
		/// </summary>
		GroupStart* _start;

	public:
		GroupEnd(GroupStart* start)
		{
			_start = start;
		}

		int try_match(const string& str, size_t start_pos) override
		{
			int r = try_next(0,str, start_pos);

			if (r > -1)
				_start->end_capture(start_pos);

			return r;
		}
	};

	//////////////////////
	//	ROOT Atom
	//////////////////////

	/// <summary>
	/// The root nod of the regex.
	/// </summary>
	class RootAtom : public Atom
	{

	public:
		RootAtom(Atom* innerRoot)
		{
			// Create a start that will track the total match
			GroupStart* start = new GroupStart(0, innerRoot);
			
			// Create our end, and give it a pointer to where our start is
			GroupEnd* end = new GroupEnd(start);

			// Place our end atom at the end of the start
			start->append(end);

			// Set the start as our next
			_next = start;
		}

		int try_match(const string& str, size_t start_pos) override
		{
			reset();
			return try_next(0, str, start_pos);
		}
	};
}

