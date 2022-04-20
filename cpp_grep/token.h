#pragma once
#include "utils.h"

namespace rex
{

	class Token
	{
	public:
		// Enclosing this enum in the token class to give it a scope, since enum class isn't supported in older C++ versions
		enum Type
		{
			EMPTY,
			LITERAL,
			START_CHAR_CLASS,		// [
			END_CHAR_CLASS,			// ]
			CHAR_RANGE,
			SPECIAL,
			CARRET,					// ^
			DOLLAR,					// $
			DOT,					// .
			OR_OP,					// |

			GREEDY_PLUS,			// +
			GREEDY_STAR,			// *
			GREEDY_Q_MARK,			// ?
			LAZY_PLUS,				// +?
			LAZY_STAR,				// *?
			LAZY_Q_MARK,			// ??
			STATIC_QUAN,
			GREEDY_MIN_QUAN,
			LAZY_MIN_QUAN,
			GREEDY_RANGE_QUAN,
			LAZY_RANGE_QUAN,

			START_GROUP,			// ( or (?:
			END_GROUP				// )
		};

		enum Type type;
		size_t location;
		string originalText;
		string value;

		Token()
		{
			type = EMPTY;
			location = 0;
			originalText = "";
			value = "";
		}

		void set_text(string val)
		{
			originalText = val;
			value = val;
		}

		bool isQuan()
		{
			return type >= GREEDY_PLUS && type <= LAZY_RANGE_QUAN;
		}

		string toString()
		{
			string val("Type: ");
			switch (type)
			{
			case CARRET:
				val += "CARRET";
				break;
			case CHAR_RANGE:
				val += "CHAR_RANGE";
				break;
			case DOLLAR:
				val += "DOLLAR";
				break;
			case DOT:
				val += "DOT";
				break;
			case EMPTY:
				val += "EMPTY";
				break;
			case END_CHAR_CLASS:
				val += "END_CHAR_CLASS";
				break;
			case END_GROUP:
				val += "END_GROUP";
				break;
			case GREEDY_PLUS:
				val += "GREEDY_PLUS";
				break;
			case GREEDY_Q_MARK:
				val += "GREEDY_Q_MARK";
				break;
			case GREEDY_STAR:
				val += "GREEDY_STAR";
				break;
			case LAZY_PLUS:
				val += "LAZY_PLUS";
				break;
			case LAZY_Q_MARK:
				val += "LAZY_Q_MARK";
				break;
			case LAZY_STAR:
				val += "LAZY_STAR";
				break;
			case LITERAL:
				val += "LITERAL";
				break;
			case GREEDY_MIN_QUAN:
				val += "GREEDY_MIN_QUAN";
				break;
			case LAZY_MIN_QUAN:
				val += "LAZY_MIN_QUAN";
				break;
			case OR_OP:
				val += "OR_OP";
				break;
			case GREEDY_RANGE_QUAN:
				val += "GREEDY_RANGE_QUAN";
				break;
			case LAZY_RANGE_QUAN:
				val += "LAZY_RANGE_QUAN";
				break;
			case SPECIAL:
				val += "SPECIAL";
				break;
			case START_CHAR_CLASS:
				val += "START_CHAR_CLASS";
				break;
			case START_GROUP:
				val += "START_GROUP";
				break;
			case STATIC_QUAN:
				val += "STATIC_QUAN";
				break;

			default:
				break;
			}

			val += " Position: " + toStr(location) + " Original: '" + originalText + "' Value: '" + value + "'";
			return val;
		}
	};
}

