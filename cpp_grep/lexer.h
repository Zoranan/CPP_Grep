#pragma once
#include <string>
#include <vector>
#include "token.h"

#include "defines.h"

namespace rex
{
	class Lexer
	{
	private:
		static void end_token(vector<Token>& toks, Token& current)
		{
			if (current.type != Token::Type::EMPTY)
			{
				if (current.isQuan() && (toks.size() == 0 || toks[toks.size() - 1].isQuan()))
					throw "Invalid quantifier at " + toStr(current.location + 1);

				toks.push_back(current);
			}
			else
				throw "Attempted to add an empty token! " + current.toString();

			current.originalText.erase();
			current.value.erase();
			current.type = Token::Type::EMPTY;
		}

		static size_t lex_quantifier(string& pattern, size_t start, vector<Token>& toks, Token& tok)
		{
			// Start should be the character right after a { was encountered
			size_t i = start;

			//Look for first number
			for (; i < pattern.length(); i++)
			{
				unsigned char c = pattern[i];
				if (c < '0' || c > '9')
					break;
			}

			// If a number was found, then this could be a quantifier
			if (i > start)
			{
				tok.type = Token::Type::STATIC_QUAN;

				//Check for a comma next. If it's there, then this might either be a range or min quan
				if (pattern[i] == ',')
				{
					i++;
					tok.type = Token::Type::GREEDY_MIN_QUAN;

					//Look for a second number
					bool foundSec = false;
					for (; i < pattern.length(); i++)
					{
						unsigned char c = pattern[i];
						if (c < '0' || c > '9')
							break;

						else
							foundSec = true;
					}

					if (foundSec)
						tok.type = Token::Type::GREEDY_RANGE_QUAN;
				}

				//The last char should be a closing }
				if (pattern[i] == '}')
				{
					i++;
					size_t len = i - start;
					string val = pattern.substr(start, len - 1);
					tok.originalText = "{" + val + "}";
					if (tok.type == Token::Type::GREEDY_MIN_QUAN)
						val.pop_back();	// Remove training comma from value
					
					// Check for the lazy modifier
					if (tok.type != Token::Type::STATIC_QUAN && i < pattern.size() && pattern[i] == '?')
					{
						tok.originalText.push_back('?');
						i++;
						len++;

						if (tok.type == Token::Type::GREEDY_MIN_QUAN)
							tok.type = Token::Type::LAZY_MIN_QUAN;
						else
							tok.type = Token::Type::LAZY_RANGE_QUAN;
					}

					tok.value = val;

					end_token(toks, tok);
					return len;
				}
			}

			//If we made it here then something failed and this is not a quantifier. Add it as a literal
			tok.set_text("{");
			tok.type = Token::Type::LITERAL;
			end_token(toks, tok);
			return 1;
		}

		static bool read_hex_char(unsigned char in, unsigned char& out)
		{
			if (in >= '0' && in <= '9')
			{
				out = in - '0';
				return true;
			}

			if (in >= 'A' && in <= 'F')
			{
				out = in - 'A' + 10;
				return true;
			}

			if (in >= 'a' && in <= 'f')
			{
				out = in - 'a' + 10;
				return true;
			}
			return false;
		}

		/// <summary>
		/// Get an escape token as a char literal or special
		/// </summary>
		/// <param name="pattern">The regex pattern</param>
		/// <param name="start">The starting location of the escape sequence (after the slash)</param>
		/// <param name="tokOut"></param>
		/// <returns></returns>
		static bool get_escaped_token(string& pattern, size_t start, Token& tokOut)
		{
			if (start >= pattern.length())
				throw "Incomplete escape sequence at " + toStr(start);

			tokOut.location = start;

			switch (pattern[start])
			{
			case 'r':
				tokOut.originalText = "\\r";
				tokOut.value = "\r";
				tokOut.type = Token::Type::LITERAL;
				return false;

			case 'n':
				tokOut.originalText = "\\n";
				tokOut.value = "\n";
				tokOut.type = Token::Type::LITERAL;
				return false;

			case 't':
				tokOut.originalText = "\\t";
				tokOut.value = "\t";
				tokOut.type = Token::Type::LITERAL;
				return false;

			case 'f':
				tokOut.originalText = "\\f";
				tokOut.value = "\f";
				tokOut.type = Token::Type::LITERAL;
				return false;

			// Hexadecimal char codes (ascii)
			case 'x':
			{
				unsigned char a;
				unsigned char b;

				if (start + 2 >= pattern.length() 
					|| !read_hex_char(pattern[start + 1], a) 
					|| !read_hex_char(pattern[start + 2], b))
					throw "Invalid escape sequence at " + toStr(start) + ". Expected a 2 digit hex value";

				tokOut.originalText = pattern.substr(start - 1, 4);
				tokOut.value = string(1, a * 16 + b);
				tokOut.type = Token::Type::LITERAL;
				return false;
			}

				//META

			case 'd':
				tokOut.set_text("\\d");
				tokOut.type = Token::Type::SPECIAL;
				return true;

			case 'D':
				tokOut.set_text("\\D");
				tokOut.type = Token::Type::SPECIAL;
				return true;

			case 'w':
				tokOut.set_text("\\w");
				tokOut.type = Token::Type::SPECIAL;
				return true;

			case 'W':
				tokOut.set_text("\\W");
				tokOut.type = Token::Type::SPECIAL;
				return true;

			case 's':
				tokOut.set_text("\\s");
				tokOut.type = Token::Type::SPECIAL;
				return true;

			case 'S':
				tokOut.set_text("\\S");
				tokOut.type = Token::Type::SPECIAL;
				return true;

			case 'b':
				tokOut.set_text("\\b");
				tokOut.type = Token::Type::SPECIAL;
				return true;

			case 'B':
				tokOut.set_text("\\B");
				tokOut.type = Token::Type::SPECIAL;
				return true;

			default:
				tokOut.set_text(string(1, pattern[start]));
				tokOut.type = Token::Type::LITERAL;
				return false;
			}
		}

		static size_t lex_char_class(string& pattern, size_t start, vector<Token>& toks, Token& tok)
		{
			bool isNeg = false;
			size_t i = start;
			if (i < pattern.length() && pattern[i] == '^')
			{
				string t(OPEN_CLASS_STR);
				t.push_back('^');
				tok.set_text(t);
				i++;
				isNeg = true;
			}
			else
			{
				tok.set_text(OPEN_CLASS_STR);
			}

			end_token(toks, tok);	//Add the opening token

			//Check for a leading dash
			if (pattern[i] == '-')
			{
				tok.set_text("-");
				tok.type = Token::Type::LITERAL;
				end_token(toks, tok);
				i++;
			}

			// Loop through remaining chars to tokenize each one
			while (i < pattern.length())
			{
				unsigned char c = pattern[i];

				if (c == CLOSE_CLASS_C)
				{
					if (i == start)
					{
						throw "Invalid character class at: " + toStr(i);
					}

					tok.location = i;
					tok.set_text(CLOSE_CLASS_STR);
					tok.type = Token::Type::END_CHAR_CLASS;

					end_token(toks, tok);

					return i - start + 1;
				}

				// Look for an escape seqence, set the token to it, and move I forward but do not end the token yet
				if (c == '\\')
				{
					get_escaped_token(pattern, i + 1, tok);
					i += tok.originalText.length();
				}

				//This wasn't an escape sequence. Just grab the token literal and move forward once
				else
				{
					tok.location = i;
					tok.type = Token::Type::LITERAL;
					tok.set_text(string(1, c));
					i++;
				}

				// Now check how many chars are left to decide what we want to do
				// If there are at least 2 characters left in the string, the next is a dash, and the one after is not a closing char class, 
				// then we are about to enter a character range
				if (i + 2 < pattern.size() && pattern[i] == '-' && pattern[i + 1] != CLOSE_CLASS_C)
				{
					// Check if the token is currently set to special. Character ranges can't be set to special
					if (tok.type == Token::Type::SPECIAL)
						throw "Invalid character range at " + toStr(tok.location) + ". The special sequence '" + tok.originalText + "' cannot be used in a character range";

					unsigned char minRange = tok.value[0];
					unsigned char maxRange;
					i++;
					if (pattern[i] == '\\')
					{
						if (get_escaped_token(pattern, i + 1, tok))
							throw "Invalid character range at " + toStr(i) + ". The special sequence '" + tok.originalText + "' cannot be used in a character range";

						maxRange = tok.value[0];
						i += tok.originalText.length();
					}
					else
					{
						maxRange = pattern[i];
						i++;
					}

					if (minRange >= maxRange)
						throw "Invalid character range at " + toStr(start) + ". Minimum value must be less than the maximum value";

					tok.originalText = pattern.substr(start, i - start - 1);
					tok.value = string(1, minRange) + "-";
					tok.value.push_back(maxRange);
					tok.type = Token::Type::CHAR_RANGE;
					tok.location = start;
				}

				end_token(toks, tok);
			}

			// We should have returned out by now
			throw "Character class ended unexpectedly at: " + toStr(i);
		}

	public:
		static vector<Token> lex(string pattern, bool multiline = true, bool singleline = false)
		{
			vector<Token> tokens;

			for (size_t i = 0; i < pattern.length(); i++)
			{
				unsigned char c = pattern[i];
				Token tok;

				switch (c)
				{
				case '{':
					tok.location = i;
					i += lex_quantifier(pattern, i + 1, tokens, tok);	// This will handle parsing as either a quantifier or a literal
					break;

					// Close brace and comma will be handled in the parse quantifer method if needed

				case '(':
					tok.location = i;
					tok.type = Token::Type::START_GROUP;
					// Look ahead
					if (i + 2 < pattern.length() && pattern[i + 1] == '?' && pattern[i + 2] == ':')
					{
						tok.set_text("(?:");
						i += 2;
					}
					else
					{
						tok.set_text("(");
					}

					end_token(tokens, tok);
					break;

				case ')':					
					tok.location = i;
					tok.set_text(")");
					tok.type = Token::Type::END_GROUP;
					end_token(tokens, tok);
					break;

				case '^':
					tok.location = i;
					tok.set_text("^");
					tok.type = Token::Type::CARRET;
					end_token(tokens, tok);
					break;

				case '$':
					tok.location = i;
					tok.set_text("$");
					tok.type = Token::Type::DOLLAR;
					end_token(tokens, tok);
					break;

				case '|':
					if (i == 0 || i == pattern.length() - 1 || (i < pattern.length() - 1 && pattern[i + 1] == '|'))
						throw "Invalid location for OR (|) token: " + toStr(i);

					tok.location = i;
					tok.set_text("|");
					tok.type = Token::Type::OR_OP;
					end_token(tokens, tok);
					break;

				case '.':
					tok.location = i;
					tok.set_text(".");
					tok.type = Token::Type::DOT;
					end_token(tokens, tok);
					break;

				case OPEN_CLASS_C:
					tok.location = i;
					tok.type = Token::Type::START_CHAR_CLASS;
					i += lex_char_class(pattern, i + 1, tokens, tok);
					break;

				case '+':
					tok.location = i;

					if (i + 1 < pattern.length() && pattern[i + 1] == '?')
					{
						tok.set_text("+?");
						tok.type = Token::Type::LAZY_PLUS;
						i++;
					}
					else
					{
						tok.set_text("+");
						tok.type = Token::Type::GREEDY_PLUS;
					}

					end_token(tokens, tok);
					break;

				case '*':
					tok.location = i;

					if (i + 1 < pattern.length() && pattern[i + 1] == '?')
					{
						tok.set_text("*?");
						tok.type = Token::Type::LAZY_STAR;
						i++;
					}
					else
					{
						tok.set_text("*");
						tok.type = Token::Type::GREEDY_STAR;
					}

					end_token(tokens, tok);
					break;

				case '?':
					tok.location = i;

					if (i + 1 < pattern.length() && pattern[i + 1] == '?')
					{
						tok.set_text("??");
						tok.type = Token::Type::LAZY_Q_MARK;
						i++;
					}
					else
					{
						tok.set_text("?");
						tok.type = Token::Type::GREEDY_Q_MARK;
					}

					end_token(tokens, tok);
					break;

				case '\\':
					get_escaped_token(pattern, i + 1, tok);
					i += tok.originalText.length() - 1;
					end_token(tokens, tok);
					
					break;

				default:
					tok.location = i;
					tok.set_text(string(1, c));
					tok.type = Token::Type::LITERAL;
					end_token(tokens, tok);
					break;
				}
			}

			return tokens;
		}
	};
}

