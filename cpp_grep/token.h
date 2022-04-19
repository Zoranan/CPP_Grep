#pragma once
#include "numtostr.h"

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
			type = Type::EMPTY;
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
			return type >= Type::GREEDY_PLUS && type <= Type::LAZY_RANGE_QUAN;
		}

		string toString()
		{
			string val("Type: ");
			switch (type)
			{
			case Type::CARRET:
				val += "CARRET";
				break;
			case Type::CHAR_RANGE:
				val += "CHAR_RANGE";
				break;
			case Type::DOLLAR:
				val += "DOLLAR";
				break;
			case Type::DOT:
				val += "DOT";
				break;
			case Type::EMPTY:
				val += "EMPTY";
				break;
			case Type::END_CHAR_CLASS:
				val += "END_CHAR_CLASS";
				break;
			case Type::END_GROUP:
				val += "END_GROUP";
				break;
			case Type::GREEDY_PLUS:
				val += "GREEDY_PLUS";
				break;
			case Type::GREEDY_Q_MARK:
				val += "GREEDY_Q_MARK";
				break;
			case Type::GREEDY_STAR:
				val += "GREEDY_STAR";
				break;
			case Type::LAZY_PLUS:
				val += "LAZY_PLUS";
				break;
			case Type::LAZY_Q_MARK:
				val += "LAZY_Q_MARK";
				break;
			case Type::LAZY_STAR:
				val += "LAZY_STAR";
				break;
			case Type::LITERAL:
				val += "LITERAL";
				break;
			case Type::GREEDY_MIN_QUAN:
				val += "GREEDY_MIN_QUAN";
				break;
			case Type::LAZY_MIN_QUAN:
				val += "LAZY_MIN_QUAN";
				break;
			case Type::OR_OP:
				val += "OR_OP";
				break;
			case Type::GREEDY_RANGE_QUAN:
				val += "GREEDY_RANGE_QUAN";
				break;
			case Type::LAZY_RANGE_QUAN:
				val += "LAZY_RANGE_QUAN";
				break;
			case Type::SPECIAL:
				val += "SPECIAL";
				break;
			case Type::START_CHAR_CLASS:
				val += "START_CHAR_CLASS";
				break;
			case Type::START_GROUP:
				val += "START_GROUP";
				break;
			case Type::STATIC_QUAN:
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

