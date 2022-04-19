#pragma once

#include "token.h"
#include "atom.h"

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
					throw "Illegal nested pair at " + toStr(i - 1);
			}

			if (balance != 0)
				throw "Unbalanced pair at " + toStr(startIndex);

			return sub;
		}

		static Atom* get_special(Token tok)
		{
			bool invert = tok.value[1] >= 'A' && tok.value[1] <= 'Z';
			Atom* special;

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
				break;

			default:
				throw "Invalid escape sequence: '" + tok.originalText + "' at " + toStr(tok.location);
				break;
			}

			if (invert)
				return new InversionAtom(special);
			else
				return special;
		}

		static Atom* parse_inner(vector<Token> toks, bool caseSensitive, bool in_char_class, unsigned short &gNum)
		{
			vector<Atom*> ors;
			Atom* next = nullptr;
			bool lastWasOr = false;

			try 
			{
				for (size_t i = 0; i < toks.size(); i++)
				{
					Token tok = toks[i];

					switch (tok.type)
					{
					case Token::Type::LITERAL:
						next = new CharLiteral(tok.value[0], caseSensitive);
						break;
					case Token::Type::CHAR_RANGE:
					{
						unsigned char min = tok.value[0];
						unsigned char max = tok.value[2];
						next = new CharRange(min, max);
						if (!caseSensitive)
						{
							if (min <= 'A' && max >= 'z' || max < 'A' || min > 'z' || (max > 'Z' && min < 'a'))
								//This range already covers both cases or has no alpha characters
								break;

							bool containsUpper = max > 'A' && min < 'Z';
							bool containsLower = max > 'a' && min < 'z';
							vector<Atom*> parts;
							parts.push_back(next);

							if (containsUpper)
							{
								//Get the alpha char bounds of the character range, to lower
								unsigned char lowerMin = min >= 'A' ? min + 32 : 'a';
								unsigned char lowerMax = max <= 'Z' ? max + 32 : 'z';

								if (lowerMin < lowerMax)
									ors.push_back(new CharRange(lowerMin, lowerMax));
							}
							else if (containsLower)
							{
								//Get the alpha char bounds of the character range, to upper
								unsigned char upperMin = min >= 'a' ? min - 32 : 'A';
								unsigned char upperMax = max <= 'z' ? max - 32 : 'Z';

								if (upperMin < upperMax)
									ors.push_back(new CharRange(upperMin, upperMax));
							}

							if (parts.size() > 1)
								next = new OrAtom(parts);

						}
						break;
					}
					case Token::Type::CARRET:
						next = new BeginLineAtom();
						break;
					case Token::Type::DOLLAR:
						next = new EndLineAtom();
						break;
					case Token::Type::DOT:
						next = new AnyChar();
						break;
					case Token::Type::SPECIAL:
						next = get_special(tok);
						break;

					case Token::Type::START_CHAR_CLASS:
					{
						vector<Token> t = sub_seq(toks, i, Token::Type::END_CHAR_CLASS, false);
						if (t.empty())
							continue;	//Ignore empty char classes

						next = parse_inner(t, caseSensitive, true, gNum);

						//Check if this is inverted
						if (tok.originalText.size() > 1 && tok.originalText[1] == '^')
							next = new InversionAtom(next);

						break;
					}

					case Token::Type::START_GROUP:
					{
						vector<Token> t = sub_seq(toks, i, Token::Type::END_GROUP, true);
						
						if (t.empty())
						{
							throw "Empty group at " + toStr(tok.location);
						}

						//Check if this is a non-capturing group
						else if (tok.originalText.length() == 3 && tok.originalText[1] == '?' && tok.originalText[2] == ':')
						{
							next = parse_inner(t, caseSensitive, false, gNum);
						}

						//This is a capturing group
						else 
						{
							unsigned short captureGroup = gNum;	//Store off our capture group before it gets incremented
							gNum++;

							next = parse_inner(t, caseSensitive, false, gNum);

							//Wrap next in a start and end atom
							GroupStart* start = new GroupStart(captureGroup, next);
							GroupEnd* end = new GroupEnd(start);
							start->append(end);
							next = start;
						}
						break;
					}

					case Token::Type::OR_OP:
						lastWasOr = true;
						continue;	//Nothing to do

					default:
						throw "Unsupported token ::: " + tok.toString();
						break;
					}

					// If we are in a char class we OR everything instead of "next" ing it.
					if (in_char_class)
					{
						// Add a copy of our pointer to the or list
						ors.push_back(next);
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
						case Token::Type::GREEDY_Q_MARK:
							greedy = true;
						case Token::Type::LAZY_Q_MARK:
							next = quantify(next, 0, 1, greedy);
							i++;
							break;

						case Token::Type::GREEDY_STAR:
							greedy = true;
						case Token::Type::LAZY_STAR:
							next = quantify(next, 0, UINT32_MAX, greedy);	//TODO: Make no max a thing
							i++;
							break;

						case Token::Type::GREEDY_PLUS:
							greedy = true;
						case Token::Type::LAZY_PLUS:
							next = quantify(next, 1, UINT32_MAX, greedy);	//TODO: Make no max a thing
							i++;
							break;

						case Token::Type::STATIC_QUAN:
						{
							unsigned int x;
							istringstream(t2.value) >> x;	//TODO: Get a better way of parsing numbers
							next = quantify(next, x, x, false);
							i++;
							break;
						}

						case Token::Type::GREEDY_MIN_QUAN:
							greedy = true;
						case Token::Type::LAZY_MIN_QUAN:
						{
							unsigned int x;
							istringstream(t2.value) >> x;	//TODO: Get a better way of parsing numbers
							next = quantify(next, x, UINT32_MAX, greedy);	//TODO: Make no max a thing, and make lazy version
							i++;
							break;
						}

						case Token::Type::GREEDY_RANGE_QUAN:
							greedy = true;
						case Token::Type::LAZY_RANGE_QUAN:
						{
							unsigned int x, y;
							size_t com = t2.value.find(',');
							istringstream(t2.value.substr(0, com)) >> x;	//TODO: Get a better way of parsing numbers
							istringstream(t2.value.substr(com + 1)) >> y;	//TODO: Get a better way of parsing numbers
							next = quantify(next, x, y, greedy);	//TODO: Make lazy version
							i++;
							break;
						}

						default:
							break;
						}
					}

					// Decide what to do with our completed atom
					if (ors.empty() || lastWasOr)
					{
						ors.push_back(next);
					}
					else
					{
						//This is not an OR operation, so append this atom to the very end of the last root atom in the list
						ors.back()->append(next);
					}


				}// End for

				// Only one atom, just return it
				if (ors.size() == 1)
					return ors[0];

				else if (ors.size() > 1)
					return new OrAtom(ors);

				else
					throw "Something went horribly wrong";
			
			} // End try

			catch (string x)
			{
				//Clean up memory
				for (size_t i = 0; i < ors.size(); i++)
					delete ors[i];
				
				if (next != nullptr)
					delete next;

				//Throw again
				throw x;
			}

		}//End parse_inner
		
	public:
		static Atom* parse(vector<Token> toks, bool caseSensistive)
		{
			unsigned short gNum = 1;	// The starting group number. Must be a var so it can be passed by ref
			try 
			{
				Atom* inner = parse_inner(toks, caseSensistive, false, gNum);
				return new RootAtom(inner);
			}
			catch (string err)
			{
				throw err;
			}

		}

};
}

