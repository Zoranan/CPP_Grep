#pragma once

#include "token.h"
#include "atom.h"
#include "SafeVector.h"
#include "UniquePtr.h"
#include "RegexException.h"

namespace rex
{
	using namespace std;

	class Parser
	{
	private:
		static Atom* quantify(Atom* base, unsigned int min, unsigned int max, bool greedy)
		{
			// Create a temp pointer that will become the new current.
			Atom* t = nullptr;

			// We de-reference the pointer to base. When it gets passed into the constructor its
			// value is copied to a field of the quantifier object.
			if (greedy)
				t = new GreedyQuantifier(base, min, max);
			else
				t = new LazyQuantifier(base, min, max);

			return t;
		}

		static vector<Token> sub_seq(vector<Token> &master, size_t &i, enum Token::Type endsWith, bool allowNesting)
		{
			size_t startIndex = i;
			vector<Token> sub;
			Token::Type startType = master[i].type;
			int balance = 1;
			
			//Skip opening token
			i++;

			//Make sure this isnt an empty set

			while (i < master.size())
			{	
				if (master[i].type == startType)
				{
					balance++;
				}
				else if (master[i].type == endsWith && --balance == 0)
				{
					// Don't skip the closing paren, because the calling method will increment after this
					break;
				}

				sub.push_back(master[i]);
				i++;

				if (!allowNesting && balance > 1)
					throw RegexSyntaxException("Illegal nested pair", i - 1);
			}

			if (balance != 0)
				throw RegexSyntaxException("Unbalanced pair", startIndex);

			return sub;
		}

		static Atom* get_special(Token tok)
		{
			// I could use my UniquePtr and SafeVector classes here, but the only spot 
			// an exception is throw is in the default case of the switch where the ptr would be null

			bool invert = tok.value[1] >= 'A' && tok.value[1] <= 'Z';
			Atom* special;
			int inv_step = 1;

			switch (tok.value[1])
			{
			case 'd':
			case 'D':
				special = new CharRange('0', '9');
				break;

			case 'w':
			case 'W':
			{
				vector<Atom*> atoms;
				atoms.push_back(new CharRange('a', 'z'));
				atoms.push_back(new CharRange('A', 'Z'));
				atoms.push_back(new CharRange('0', '9'));
				atoms.push_back(new CharLiteral('_', true));
				special = new OrAtom(atoms);
				break;
			}

			case 's':
			case 'S':
			{
				vector<Atom*> atoms;
				atoms.push_back(new CharLiteral(' ', true));
				atoms.push_back(new CharLiteral('\n', true));
				atoms.push_back(new CharLiteral('\r', true));
				atoms.push_back(new CharLiteral('\t', true));
				atoms.push_back(new CharLiteral('\f', true));
				special = new OrAtom(atoms);
				break;
			}

			case 'b':
			case 'B':
				special = new WordBoundary();
				inv_step = 0;
				break;

			default:
				throw RegexSyntaxException("Invalid escape sequence '" + tok.originalText + "'", tok.location);
				break;
			}

			if (invert)
				return new InversionAtom(special, inv_step);
			else
				return special;
		}

		static Atom* parse_inner(vector<Token> toks, bool caseSensitive, bool in_char_class, unsigned short &gNum)
		{
			SafeVector<Atom*> ors;	// Deletes any held pointers when it goes out of scope
			bool lastWasOr = false;

			for (size_t i = 0; i < toks.size(); i++)
			{
				UniquePtr<Atom*> next;
				Token tok = toks[i];

				switch (tok.type)
				{
				case Token::LITERAL:
					next.replace(new CharLiteral(tok.value[0], caseSensitive));
					break;
				case Token::CHAR_RANGE:
				{
					unsigned char min = tok.value[0];
					unsigned char max = tok.value[2];
					next.replace(new CharRange(min, max));
					if (!caseSensitive)
					{
						if (min <= 'A' && max >= 'z' || max < 'A' || min > 'z' || (max > 'Z' && min < 'a'))
							//This range already covers both cases or has no alpha characters
							break;

						bool containsUpper = max > 'A' && min < 'Z';
						bool containsLower = max > 'a' && min < 'z';
						SafeVector<Atom*> parts;
						parts.add(next.release());

						if (containsUpper)
						{
							//Get the alpha char bounds of the character range, to lower
							unsigned char lowerMin = min >= 'A' ? min + 32 : 'a';
							unsigned char lowerMax = max <= 'Z' ? max + 32 : 'z';

							if (lowerMin < lowerMax)
								parts.add(new CharRange(lowerMin, lowerMax));
						}
						else if (containsLower)
						{
							//Get the alpha char bounds of the character range, to upper
							unsigned char upperMin = min >= 'a' ? min - 32 : 'A';
							unsigned char upperMax = max <= 'z' ? max - 32 : 'Z';

							if (upperMin < upperMax)
								parts.add(new CharRange(upperMin, upperMax));
						}

						if (parts.size() > 1)
							next.replace(new OrAtom(parts.release()));

						else
						{
							next.replace(parts.at(0));
							parts.release();
						}
					}
					break;
				}
				case Token::CARRET:
					next.replace(new BeginLineAtom());
					break;
				case Token::DOLLAR:
					next.replace(new EndLineAtom());
					break;
				case Token::DOT:
					next.replace(new AnyChar());
					break;
				case Token::SPECIAL:
					next.replace(get_special(tok));
					break;

				case Token::START_CHAR_CLASS:
				{
					vector<Token> t = sub_seq(toks, i, Token::END_CHAR_CLASS, false);
					if (t.empty())
						continue;	//Ignore empty char classes

					next.replace(parse_inner(t, caseSensitive, true, gNum));

					//Check if this is inverted
					if (tok.originalText.size() > 1 && tok.originalText[1] == '^')
						next.replace(new InversionAtom(next.release(), 1));

					break;
				}

				case Token::START_GROUP:
				{
					vector<Token> t = sub_seq(toks, i, Token::END_GROUP, true);

					if (t.empty())
					{
						throw RegexSyntaxException("Empty group", tok.location);
					}

					//Check if this is a non-capturing group
					else if (tok.originalText.length() == 3 && tok.originalText[1] == '?' && tok.originalText[2] == ':')
					{
						next.replace(parse_inner(t, caseSensitive, false, gNum));
					}

					//This is a capturing group
					else
					{
						unsigned short captureGroup = gNum;	//Store off our capture group before it gets incremented
						gNum++;

						// Create the group start with the inner part of the group attached
						UniquePtr<GroupStart*> start(new GroupStart(captureGroup, parse_inner(t, caseSensitive, false, gNum)));
						//Create an end group atom with a reference to the start atom, the attach the end to the start;
						start.get()->append(new GroupEnd(start.get()));

						next.replace(start.release());
					}
					break;
				}

				case Token::OR_OP:
					lastWasOr = true;
					continue;	//Nothing to do

				default:
					throw RegexSyntaxException("Unsupported token '" + tok.originalText + "'", tok.location);
					break;
				}

				// If we are in a char class we OR everything instead of "next" ing it.
				if (in_char_class)
				{
					// Add our pointer to the or list
					ors.add(next.release());
					continue;
				}

				// We aren't in a char class, so...
				// Check for a quantifier

				if (i + 1 < toks.size())
				{
					bool greedy = false;
					Token t2 = toks[i + 1];

					switch (t2.type)
					{
					case Token::GREEDY_Q_MARK:
						greedy = true;
					case Token::LAZY_Q_MARK:
						next.replace(quantify(next.release(), 0, 1, greedy));
						i++;
						break;

					case Token::GREEDY_STAR:
						greedy = true;
					case Token::LAZY_STAR:
						next.replace(quantify(next.release(), 0, UINT32_MAX, greedy));
						i++;
						break;

					case Token::GREEDY_PLUS:
						greedy = true;
					case Token::LAZY_PLUS:
						next.replace(quantify(next.release(), 1, UINT32_MAX, greedy));
						i++;
						break;

					case Token::STATIC_QUAN:
					{
						unsigned int x;
						istringstream(t2.value) >> x;	//TODO: Get a better way of parsing numbers
						next.replace(quantify(next.release(), x, x, false));
						i++;
						break;
					}

					case Token::GREEDY_MIN_QUAN:
						greedy = true;
					case Token::LAZY_MIN_QUAN:
					{
						unsigned int x;
						istringstream(t2.value) >> x;	//TODO: Get a better way of parsing numbers
						next.replace(quantify(next.release(), x, UINT32_MAX, greedy));	//TODO: Make no max a thing, and make lazy version
						i++;
						break;
					}

					case Token::GREEDY_RANGE_QUAN:
						greedy = true;
					case Token::LAZY_RANGE_QUAN:
					{
						unsigned int x, y;
						size_t com = t2.value.find(',');
						istringstream(t2.value.substr(0, com)) >> x;	//TODO: Get a better way of parsing numbers
						istringstream(t2.value.substr(com + 1)) >> y;	//TODO: Get a better way of parsing numbers
						next.replace(quantify(next.release(), x, y, greedy));
						i++;
						break;
					}

					default:
						//This is not a quantifier
						break;
					}
				}

				// Decide what to do with our completed atom
				if (ors.size() == 0 || lastWasOr)
				{
					ors.add(next.release());
					lastWasOr = false;
				}
				else
				{
					//This is not an OR operation, so append this atom to the very end of the last root atom in the list
					ors.back()->append(next.release());
				}


			}// End for

			// Only one atom, just return it
			if (ors.size() == 1)
			{
				Atom* temp = ors.at(0);
				ors.release();
				return temp;
			}

			else if (ors.size() > 1)
				return new OrAtom(ors.release());

			else
				throw RegexException("Something went horribly wrong");

		}//End parse_inner
		
	public:
		static Atom* parse(vector<Token> toks, bool caseSensistive)
		{
			unsigned short gNum = 1;	// The starting group number. Must be a var so it can be passed by ref
			Atom* inner = parse_inner(toks, caseSensistive, false, gNum);
			return new RootAtom(inner);
		}

};
}

