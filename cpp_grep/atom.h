#pragma once
#include <string>
#include <vector>
#include <stack>
#include "match.h"
#include "defines.h"	//Contains define for NULLPTR

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

		int try_next(int current_result, Match& m, string& str, unsigned int start_pos)
		{
			if (_next == NULLPTR)
				return current_result;

			int nextRes = _next->try_match(m, str, start_pos + current_result);
			if (nextRes > -1)
				return current_result + nextRes;

			_next->reset();	//Reset our next if it exists, but fails
			return -1;
		}

	public:
		Atom(Atom* next = NULLPTR)
		{
			_next = next;
		}

		void set_next(Atom* n)
		{
			if (_next != NULLPTR)
				delete _next;

			_next = n;
		}

		/// <summary>
		/// Recursively append 'n' to the last atom in the sequence
		/// </summary>
		/// <param name="n">A pointer to the Atom to append</param>
		void append(Atom* n)
		{
			if (_next == NULLPTR)
				_next = n;

			else
				_next->append(n);
		}

		virtual void pop_state()
		{

		}

		virtual void reset()
		{
			if (_next != NULLPTR)
				_next->reset();
		}

		virtual void commit(Match& m, string& str)
		{
			if (_next != NULLPTR)
				_next->commit(m, str);
		}

		virtual int try_match(Match& m, string& str, unsigned int start_pos)
		{
			return 0;
		};

		virtual ~Atom()
		{
			if (_next != NULLPTR)
				delete _next;
		}
	};


	////////////////////
	// Character Atoms
	////////////////////

	class CharLiteral : public Atom
	{
	private:
		char _char;
		bool _caseSensitive;

	public:
		CharLiteral()
		{
			_char = '\0';
			_caseSensitive = true;
		}

		CharLiteral(char c, bool caseSensitive = true, Atom* next = NULLPTR) : Atom(next)
		{
			_char = isalpha(c) && !caseSensitive ? tolower(c) : c;
			_caseSensitive = caseSensitive;
		}

		int try_match(Match& m, string& str, unsigned int start_pos) override
		{
			if (start_pos >= str.size())
				return -1;

			char c = str[start_pos];
			if (isalpha(c) && !_caseSensitive)
				c = tolower(c);

			// This atom succeeded
			if (c == _char)
			{
				return try_next(1, m, str, start_pos);
			}

			return -1;
		}
	};

	class CharRange : public Atom
	{
	private:
		char _min;
		char _max;
		bool _caseSensitive;

	public:
		CharRange(char min, char max, bool caseSensitive = true, Atom* next = NULLPTR) : Atom(next)
		{
			_min = min;
			_max = max;
			_caseSensitive = caseSensitive;
		}

		int try_match(Match& m, string& str, unsigned int start_pos) override
		{
			if (start_pos >= str.size())
				return -1;

			char c = str[start_pos];

			//TODO: Case insensitive mode

			//This Atom succeeded
			if (_min <= c && c <= _max)
			{
				return try_next(1, m, str, start_pos);
			}

			// All success branches should have returned out by now. If we're here, then this Atom or one of its sub atoms failed
			return -1;
		}
	};

	/// <summary>
	/// Inverts the result of a SINGLE match atom only. Using this for atoms that match multiple chars will cause undefined behavior
	/// </summary>
	class InversionAtom : public Atom
	{
	private:
		Atom* _atom;
	public:
		InversionAtom(Atom* a)
		{
			_atom = a;
		}

		int try_match(Match& m, string& str, unsigned int start_pos) override
		{
			int r = _atom->try_match(m, str, start_pos);
			if (r > -1)
				return -1;
			
			return 1;
		}

		~InversionAtom()
		{
			delete _atom;
		}
	};

	////////////////////
	// Combiner Atoms
	////////////////////

	class OrAtom : public Atom
	{
	private:
		vector<Atom*> _atoms;

	protected:
		OrAtom(Atom* next = NULLPTR) : Atom(next) {}


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

		void commit(Match& m, string& str) override
		{
			for (size_t i = 0; i < _atoms.size(); i++)
			{
				_atoms[i]->commit(m, str);
			}

			Atom::commit(m, str);
		}

		int try_match(Match& m, string& str, unsigned int start_pos) override
		{
			for (size_t i = 0; i < _atoms.size(); i++)
			{
				int r = _atoms[i]->try_match(m, str, start_pos);

				// This branch of the or succeeded
				if (r > -1)
				{
					int fin = try_next(r, m, str, start_pos);	//_next is auto reset on failure with this call
					if (fin > -1)
						return fin;
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

	// No need for "and" since this is a recusrsive tree structure

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

		virtual void commit(Match& m, string& str) override
		{
			_atom->commit(m, str);
			Atom::commit(m, str);
		}
	};

	/// <summary>
	/// A quantifier that attempts to match as much as it can and only stops when it's inner Atom failes, or its maximum match count is reached
	/// </summary>
	class GreedyQuantifier : public QuantifierBase
	{

	public:
		GreedyQuantifier(Atom* a, unsigned int min, unsigned int max) : QuantifierBase(a, min, max) { }

		int try_match(Match& m, string& str, unsigned int start_pos) override
		{
			// First, get the maximum start_pos that our inner atom could match, so we can work our way backwards
			stack<unsigned int> end_positions;
			end_positions.push(start_pos);
			unsigned int last_end_pos = start_pos;

			while (end_positions.size() <= _max)
			{
				int r = _atom->try_match(m, str, last_end_pos);
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
			int fin = try_next(end_positions.top() - start_pos, m, str, end_positions.top());
			
			if (_next != NULLPTR)
				while (fin == -1)
				{
					if (end_positions.size() - 1 <= _min)	//Subtract one to account for starting value on stack. <= to account for the item we will pop off next
					{
						reset();
						return -1;
					}

					end_positions.pop();
					_atom->pop_state();	//Step this atom's memory back once

					int nr = _next->try_match(m, str, end_positions.top());
					if (nr > -1)
					{
						fin = end_positions.top() - start_pos + nr;
					}
					else
					{
						fin = -1;
						_next->reset();
					}

					//fin = try_next(end_positions.top() - start_pos, m, str, end_positions.top());
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
		// _followedBy is a base level Atom by default, which always returns a 0 length match.
		LazyQuantifier(Atom* a, unsigned int min, unsigned int max) : QuantifierBase(a, min, max) {}

		int try_match(Match& m, string& str, unsigned int start_pos) override
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
					int fin = try_next(i, m, str, start_pos);	//_next is auto reset here on failure
					if (fin > -1)
						return fin;
				}

				// We're already maxed out and should have returned by now if the next atom was successful. So return failure
				if (c >= _max)
				{
					reset();
					return -1;
				}

				// We have not reached max yet, try our inner atom again
				r = _atom->try_match(m, str, start_pos + i);
				if (r > -1)
				{
					c++;
					i += r;
				}
				//We can't match our inner atom, so we must fail
				else
				{
					reset();
					return -1;
				}
			}
		}
	};


	class GroupAtom : public Atom
	{
	private:
		Atom* _atom;
		unsigned int _g_num;
		vector<Capture> _pending_caps;

	public:
		GroupAtom(Atom* inner, unsigned int g_num)
		{
			_atom = inner;
			_g_num = g_num;
		}

		void pop_state() override
		{
			_pending_caps.pop_back();
		}

		void reset() override
		{
			_pending_caps.clear();
			_atom->reset();

			Atom::reset();
		}

		void commit(Match& m, string& str) override
		{
			for (size_t i = 0; i < _pending_caps.size(); i++)
			{
				_pending_caps[i].take_capture(str);	//Store the captured value from our string
				m.add_group_capture(_g_num, _pending_caps[i]);
			}
			_atom->commit(m, str);
			Atom::commit(m, str);
		}

		int try_match(Match& m, string& str, unsigned int start_pos) override
		{
			int r = _atom->try_match(m, str, start_pos);
			if (r > -1)
			{
				_pending_caps.push_back(Capture(start_pos, r));

				r = try_next(r, m, str, start_pos);
			}
			
			return r;
		}

		~GroupAtom()
		{
			delete _atom;
		}
	};

	////////////////////////
	// Special matches
	////////////////////////

	class BeginStringAtom : public Atom
	{
		int try_match(Match& m, string& str, unsigned int start_pos) override
		{
			if (start_pos == 0)
				return try_next(0, m, str, start_pos);

			return -1;
		}
	};

	class EndStringAtom : public Atom
	{
		int try_match(Match& m, string& str, unsigned int start_pos) override
		{
			if (start_pos == str.size())
				return try_next(0, m, str, start_pos);

			return -1;
		}
	};

	class BeginLineAtom : public Atom
	{
		int try_match(Match& m, string& str, unsigned int start_pos) override
		{
			if (start_pos == 0
				|| (start_pos - 1 < str.length() && str[start_pos - 1] == '\n')
				//|| (start_pos + 1 < str.length() && str[start_pos + 1] == '\r')
				)
				return try_next(0, m, str, start_pos);

			return -1;
		}
	};

	class EndLineAtom : public Atom
	{
		int try_match(Match& m, string& str, unsigned int start_pos) override
		{
			size_t s = str.size();

			if (start_pos >= s || (start_pos + 1 < s && (str[start_pos + 1] == '\n' || str[start_pos + 1] == '\r')))
				return try_next(0, m, str, start_pos);

			return -1;
		}
	};

	class AnyChar : public Atom
	{
		int try_match(Match& m, string& str, unsigned int start_pos) override
		{
			if (start_pos >= 0 && start_pos < str.size())
				return try_next(1, m, str, start_pos);

			return -1;
		}
	};

	//TODO: Create word boundry match
}

