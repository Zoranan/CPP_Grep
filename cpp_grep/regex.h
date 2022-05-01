#pragma once
#include <string>
#include "match.h"
#include "atom.h"
#include "utils.h"
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
		unsigned int _minLen;

	public:
		Regex() 
		{
			_pattern_str = "";
			_pattern = nullptr;
		}

		Regex(string pattern, bool caseSensitive = true)
		{
			_pattern_str = pattern;
			if (pattern.size() > 0)
			{
				vector<Token> toks = Lexer::lex(pattern);
				_pattern = Parser::parse(toks, caseSensitive);
				_minLen = _pattern->min_length();
				if (_minLen > 0)
					_minLen--;
			}
			else
				throw RegexException("Regex pattern cannot be empty");
		}

		string get_pattern()
		{
			return _pattern_str;
		}

		/// <summary>
		/// Attempt to match the regex at the specified start location only, not counting 0 length matches as success
		/// </summary>
		/// <param name="str">The string to match</param>
		/// <param name="out_match">The Match object to hold the results</param>
		/// <param name="pos">The position to match at</param>
		/// <returns>True if the match succeeds, false otherwise</returns>
		bool matchAt(const char * str, size_t strSize, Match &out_match, size_t pos)
		{
			int r = _pattern->try_match(str, strSize, pos);
			if (r > 0)
			{
				_pattern->commit(out_match, str);
				return true;
			}
			
			return false;
		}

		/// <summary>
		/// Attempt to match the regex anywhere in the string, starting at the specified start location and working right until a match is found
		/// </summary>
		/// <param name="str">The string to match</param>
		/// <param name="out_match">The Match object to hold the results</param>
		/// <param name="The position in the string to start checking at"></param>
		/// <returns></returns>
		bool match(const char *str, size_t strSize, Match& out_match, size_t start_pos = 0)
		{
			for (; start_pos + _minLen < strSize; start_pos++)
			{
				_pattern->reset();
				if (matchAt(str, strSize, out_match, start_pos))
					return true;
			}
			
			return false;
		}

		/// <summary>
		/// Attempts to match the regex as many times as possible in the string
		/// </summary>
		/// <param name="str">The string to match</param>
		/// <param name="The position in the string to start checking at"></param>
		/// <returns>A vector of matches made</returns>
		vector<Match> matches(char *str, size_t strSize, size_t start_pos = 0)
		{
			vector<Match> res;
			while (start_pos + _minLen < strSize)
			{
				Match m;
				if (matchAt(str, strSize, m, start_pos))
				{
					res.push_back(m);
					size_t len = m.get_group(0).length();
					start_pos += len == 0 ? 1 : len;	//Always move forward by at least 1
				}
				else
					start_pos++;
			}

			return res;
		}

		~Regex()
		{
			delete _pattern;
		}
	};
}
