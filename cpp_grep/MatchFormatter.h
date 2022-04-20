#pragma once
#include "match.h"
#include "utils.h"
#include "defines.h"
#include <sstream>

namespace rex
{
	using namespace std;

	class MatchFormatter
	{
	private:

		//////////////////////////////////////////////////
		// Internal classes to represent the formatting
		//////////////////////////////////////////////////

		class FormatPartBase
		{
		public:
			FormatPartBase() {}

			virtual string get(const Match& m) = 0;

			virtual ~FormatPartBase() {}
		};

		class LiteralPart : public FormatPartBase
		{
		private:
			string _text;
		public:
			LiteralPart(string text)
			{
				_text = text;
			}

			string get(const Match&) override
			{
				return _text;
			}
		};

		class DynamicPart : public FormatPartBase
		{
		private:
			unsigned short _group;
		public:
			DynamicPart(unsigned short group = 0)
			{
				_group = group;
			}

			string get(const Match& m) override
			{
				return m.get_group_value(_group);
			}
		};

		///////////////////////////
		// Private members
		///////////////////////////

		vector<FormatPartBase*> _parts;

	public:
		MatchFormatter(){}

		MatchFormatter(const string format)
		{
			string lit;
			for (size_t i = 0; i < format.length(); i++)
			{
				char c = format[i];
				if (c == '<')
				{
					// This is escaped, or doesn't have a digit afterward. just add a single < and move on
					if (i + 1 >= format.length() || !isdigit(format[i + 1]))
					{
						lit.push_back(c);	//Push the character like its normal

						if (format[i + 1] == '<')
						{
							i++;
							lit.push_back(format[i]);
						}
					}

					// This is not escaped, so it must be a reference to a group value
					else 
					{
						i++;	// Skip the opening char

						// Add our current literal string
						if (!lit.empty())
						{
							_parts.push_back(new LiteralPart(lit));
							lit.clear();
						}

						//Read our group number
						string gnum;
						while (i < format.length() && isdigit(format[i]))
						{
							gnum.push_back(format[i]);
							i++;
						}

						//Make sure there is a closing brace at the end
						if (!gnum.empty() && i < format.length() && format[i] == '>')
						{
							//Dont skip the closing char since the for loop will increment once

							// Parse the group num
							unsigned short g;
							istringstream(gnum) >> g;

							_parts.push_back(new DynamicPart(g));
						}
						else
						{
							throw FormatException("Invalid group reference. Provide a group number followed by a closing '>', or escape the opening '<' with another one right before it", i);
						}

					}
				}

				//This is a literal char. Add it
				else
				{
					lit.push_back(c);
				}
			} // End for

			//Make sure to check the literal at the end
			if (!lit.empty())
			{
				_parts.push_back(new LiteralPart(lit));
				lit.clear();
			}
		}

		string format(const Match& m)
		{
			string out;
			for (size_t i = 0; i < _parts.size(); i++)
			{
				out.append(_parts[i]->get(m));
			}
			return out;
		}

		~MatchFormatter()
		{
			for (size_t i = 0; i < _parts.size(); i++)
			{
				delete _parts[i];
			}
		}

	};
}

