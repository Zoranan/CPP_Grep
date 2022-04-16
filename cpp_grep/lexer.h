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
			if (current.type != TokenType::EMPTY)
			{
				if (current.isQuan() && (toks.size() == 0 || toks[toks.size() - 1].isQuan()))
					throw "Invalid quantifier at " + toStr(current.location + 1);

				toks.push_back(current);
			}
			else
				throw "Attempted to add an empty token! " + current.toString();

			current.originalText.erase();
			current.value.erase();
			current.type = TokenType::EMPTY;
		}

		static size_t lex_quantifier(string& pattern, size_t start, vector<Token>& toks, Token& tok)
		{
			// Start should be the character right after a { was encountered
			size_t i = start;

			//Look for first number
			for (; i < pattern.length(); i++)
			{
				char c = pattern[i];
				if (c < '0' || c > '9')
					break;
			}

			// If a number was found, then this could be a quantifier
			if (i > start)
			{
				tok.type = TokenType::STATIC_QUAN;

				//Check for a comma next. If it's there, then this might either be a range or min quan
				if (pattern[i] == ',')
				{
					i++;
					tok.type = TokenType::GREEDY_MIN_QUAN;

					//Look for a second number
					bool foundSec = false;
					for (; i < pattern.length(); i++)
					{
						char c = pattern[i];
						if (c < '0' || c > '9')
							break;

						else
							foundSec = true;
					}

					if (foundSec)
						tok.type = TokenType::GREEDY_RANGE_QUAN;
				}

				//The last char should be a closing }
				if (pattern[i] == '}')
				{
					i++;
					size_t len = i - start;
					string val = pattern.substr(start, len - 1);
					tok.originalText = "{" + val + "}";
					if (tok.type == TokenType::GREEDY_MIN_QUAN)
						val.pop_back();	// Remove training comma from value
					
					// Check for the lazy modifier
					if (tok.type != TokenType::STATIC_QUAN && i < pattern.size() && pattern[i] == '?')
					{
						tok.originalText.push_back('?');
						i++;
						len++;

						if (tok.type == TokenType::GREEDY_MIN_QUAN)
							tok.type = TokenType::LAZY_MIN_QUAN;
						else
							tok.type = TokenType::LAZY_RANGE_QUAN;
					}

					tok.value = val;

					end_token(toks, tok);
					return len;
				}
			}

			//If we made it here then something failed and this is not a quantifier. Add it as a literal
			tok.set_text("{");
			tok.type = TokenType::LITERAL;
			end_token(toks, tok);
			return 1;
		}

		static bool get_escaped_token(char c, Token& tokOut)
		{
			switch (c)
			{
			case 'r':
				tokOut.originalText = "\\r";
				tokOut.value = "\r";
				tokOut.type = TokenType::LITERAL;
				return false;

			case 'n':
				tokOut.originalText = "\\n";
				tokOut.value = "\n";
				tokOut.type = TokenType::LITERAL;
				return false;

			case 't':
				tokOut.originalText = "\\t";
				tokOut.value = "\t";
				tokOut.type = TokenType::LITERAL;
				return false;

			case 'f':
				tokOut.originalText = "\\f";
				tokOut.value = "\f";
				tokOut.type = TokenType::LITERAL;
				return false;

				//META

			case 'd':
				tokOut.set_text("\\d");
				tokOut.type = TokenType::SPECIAL;
				return true;

			case 'D':
				tokOut.set_text("\\D");
				tokOut.type = TokenType::SPECIAL;
				return true;

			case 'w':
				tokOut.set_text("\\w");
				tokOut.type = TokenType::SPECIAL;
				return true;

			case 'W':
				tokOut.set_text("\\W");
				tokOut.type = TokenType::SPECIAL;
				return true;

			case 's':
				tokOut.set_text("\\w");
				tokOut.type = TokenType::SPECIAL;
				return true;

			case 'S':
				tokOut.set_text("\\W");
				tokOut.type = TokenType::SPECIAL;
				return true;

			default:
				tokOut.set_text(string(1, c));
				tokOut.type = TokenType::LITERAL;
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
				//tok.type = TokenType::START_NEG_CHAR_CLASS;
				i++;
				isNeg = true;
			}
			else
			{
				tok.set_text(OPEN_CLASS_STR);
				//tok.type = TokenType::START_CHAR_CLASS;
			}

			end_token(toks, tok);

			//Check for a leading dash
			if (pattern[i] == '-')
			{
				tok.set_text("-");
				tok.type = TokenType::LITERAL;
				end_token(toks, tok);
				i++;
			}

			// Loop through remaining chars to tokenize each one
			for (; i < pattern.length(); i++)
			{
				char c = pattern[i];

				if (c == CLOSE_CLASS_C)
				{
					if (i == start)
					{
						throw "Invalid character class at: " + toStr(i);
					}

					tok.location = i;
					tok.set_text(CLOSE_CLASS_STR);
					tok.type = TokenType::END_CHAR_CLASS;

					end_token(toks, tok);

					return i - start + 1;
				}
				else if (c == '\\')
				{
					if (i + 1 < pattern.length())
					{
						i++;
						get_escaped_token(pattern[i], tok);
						end_token(toks, tok);
					}
					else
					{
						throw "Incomplete escape sequence at: " + toStr(i);
					}
				}
				else if (i + 2 < pattern.length() && pattern[i + 1] == '-' && pattern[i + 2] != CLOSE_CLASS_C)	//At least 2 chars remain, the next one is a dash, and second is not end of class
				{
					char second = pattern[i + 2];

					if (second != '\\' || (i + 3 < pattern.length() && !get_escaped_token(pattern[i + 3], tok)))
					{
						if (second == '\\')
						{
							second = pattern[i + 3];
							tok.originalText = pattern.substr(i, 4);
							tok.value = pattern.substr(i, 2) + second;
							i++;
						}
						else
							tok.set_text(pattern.substr(i, 3));

						if (second <= c)
							throw "Invalid character range: " + tok.originalText;

						tok.location = i;
						tok.type = TokenType::CHAR_RANGE;
						end_token(toks, tok);
						i += 2;
					}
					else
					{
						throw "Invalid character range at: " + toStr(i) + ". Cannot use token " + tok.originalText;
					}
				}
				else
				{
					tok.location = i;
					tok.set_text(pattern.substr(i, 1));
					tok.type = TokenType::LITERAL;
					end_token(toks, tok);
				}
			}

			// We should have returned out by now
			throw "Character class ended unexpectedly at: " + toStr(i);
		}

	public:
		static vector<Token> lex(string pattern, bool caseSensitive = true, bool multiline = true, bool singleline = false)
		{
			vector<Token> tokens;
			//bool inClass = false;

			for (size_t i = 0; i < pattern.length(); i++)
			{
				char c = pattern[i];
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
					tok.type = TokenType::START_GROUP;
					// Look ahead
					if (i + 2 < pattern.length() && pattern[i + 1] == '?' && pattern[i + 2] == ':')
					{
						tok.set_text("(?:");
						//tok.type = TokenType::START_GROUP_NO_CAP;
						i += 2;
					}
					else
					{
						tok.set_text("(");
						//tok.type = TokenType::START_GROUP;
					}

					end_token(tokens, tok);
					break;

				case ')':					
					tok.location = i;
					tok.set_text(")");
					tok.type = TokenType::END_GROUP;
					end_token(tokens, tok);
					break;

				case '^':
					tok.location = i;
					tok.set_text("^");
					tok.type = TokenType::CARRET;
					end_token(tokens, tok);
					break;

				case '$':
					tok.location = i;
					tok.set_text("$");
					tok.type = TokenType::DOLLAR;
					end_token(tokens, tok);
					break;

				case '|':
					if (i == 0 || i == pattern.length() - 1 || (i < pattern.length() - 1 && pattern[i + 1] == '|'))
						throw "Invalid location for OR (|) token: " + toStr(i);

					tok.location = i;
					tok.set_text("|");
					tok.type = TokenType::OR_OP;
					end_token(tokens, tok);
					break;

				case '.':
					tok.location = i;
					tok.set_text(".");
					tok.type = TokenType::DOT;
					end_token(tokens, tok);
					break;

				case OPEN_CLASS_C:
					tok.location = i;
					tok.type = TokenType::START_CHAR_CLASS;
					i += lex_char_class(pattern, i + 1, tokens, tok);
					break;

				case '+':
					tok.location = i;

					if (i + 1 < pattern.length() && pattern[i + 1] == '?')
					{
						tok.set_text("+?");
						tok.type = TokenType::LAZY_PLUS;
						i++;
					}
					else
					{
						tok.set_text("+");
						tok.type = TokenType::GREEDY_PLUS;
					}

					end_token(tokens, tok);
					break;

				case '*':
					tok.location = i;

					if (i + 1 < pattern.length() && pattern[i + 1] == '?')
					{
						tok.set_text("*?");
						tok.type = TokenType::LAZY_STAR;
						i++;
					}
					else
					{
						tok.set_text("*");
						tok.type = TokenType::GREEDY_STAR;
					}

					end_token(tokens, tok);
					break;

				case '?':
					tok.location = i;

					if (i + 1 < pattern.length() && pattern[i + 1] == '?')
					{
						tok.set_text("??");
						tok.type = TokenType::LAZY_Q_MARK;
						i++;
					}
					else
					{
						tok.set_text("?");
						tok.type = TokenType::GREEDY_Q_MARK;
					}

					end_token(tokens, tok);
					break;

				case '\\':
					if (i + 1 < pattern.length())
					{
						char next = pattern[i + 1];
						get_escaped_token(next, tok);
						end_token(tokens, tok);
						i++;
					}
					break;

				default:
					tok.location = i;
					tok.set_text(string(1, c));
					tok.type = TokenType::LITERAL;
					end_token(tokens, tok);
					break;
				}
			}

			return tokens;
		}
	};
}

