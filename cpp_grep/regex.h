#pragma once
#include <string>
#include "match.h"
#include "atom.h"
#include "numtostr.h"
#include "lexer.h"
#include "parser.h"

namespace rex
{
	using namespace std;

	class Regex
	{
	private:
		string _pattern_str;
		Atom* _pattern;

	public:

		Regex(string pattern, bool caseSensitive = true, bool multiline = true, bool singleline = false)
		{
			_pattern_str = pattern;
			if (pattern.size() > 0)
			{
				vector<Token> toks = Lexer::lex(pattern, caseSensitive, multiline, singleline);
				_pattern = Parser::parse(toks, caseSensitive);
			}
			else
				throw "Regex pattern cannot be empty";
		}

		string get_pattern()
		{
			return _pattern_str;
		}

		bool try_match(string& str, Match& out_match)
		{
			if (_pattern == NULLPTR)
				throw "Regex was not initialized";

			for (unsigned int i = 0; i < str.length(); i++)
			{
				_pattern->reset();

				int r = _pattern->try_match(out_match, str, i);
				if (r > -1)
				{
					out_match.set_start(i);
					out_match.set_length(r);
					out_match.take_capture(str);
					_pattern->commit(out_match, str);	//Commit the rest of the cap groups
					return true;
				}
			}
			return false;
		}

		~Regex()
		{
			delete _pattern;
		}
	};
}
