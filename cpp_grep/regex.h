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

			int r = _pattern->try_match(str, 0);

			// Make sure to commit all of our capture groups if it was successful
			if (r > -1)
				_pattern->commit(out_match, str);

			// We don't need to reset here since the root will call reset before each match attempt

			return r > -1;
		}

		~Regex()
		{
			delete _pattern;
		}
	};
}
