#pragma once
#include "numtostr.h"

namespace rex
{
	enum class TokenType : int
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

	class Token
	{
	public:
		enum TokenType type;
		size_t location;
		string originalText;
		string value;

		Token()
		{
			type = TokenType::EMPTY;
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
			return type >= TokenType::GREEDY_PLUS && type <= TokenType::LAZY_RANGE_QUAN;
		}

		string toString()
		{
			string val("Type: ");
			switch (type)
			{
			case TokenType::CARRET:
				val += "CARRET";
				break;
			case TokenType::CHAR_RANGE:
				val += "CHAR_RANGE";
				break;
			case TokenType::DOLLAR:
				val += "DOLLAR";
				break;
			case TokenType::DOT:
				val += "DOT";
				break;
			case TokenType::EMPTY:
				val += "EMPTY";
				break;
			case TokenType::END_CHAR_CLASS:
				val += "END_CHAR_CLASS";
				break;
			case TokenType::END_GROUP:
				val += "END_GROUP";
				break;
			case TokenType::GREEDY_PLUS:
				val += "GREEDY_PLUS";
				break;
			case TokenType::GREEDY_Q_MARK:
				val += "GREEDY_Q_MARK";
				break;
			case TokenType::GREEDY_STAR:
				val += "GREEDY_STAR";
				break;
			case TokenType::LAZY_PLUS:
				val += "LAZY_PLUS";
				break;
			case TokenType::LAZY_Q_MARK:
				val += "LAZY_Q_MARK";
				break;
			case TokenType::LAZY_STAR:
				val += "LAZY_STAR";
				break;
			case TokenType::LITERAL:
				val += "LITERAL";
				break;
			case TokenType::GREEDY_MIN_QUAN:
				val += "GREEDY_MIN_QUAN";
				break;
			case TokenType::LAZY_MIN_QUAN:
				val += "LAZY_MIN_QUAN";
				break;
			case TokenType::OR_OP:
				val += "OR_OP";
				break;
			case TokenType::GREEDY_RANGE_QUAN:
				val += "GREEDY_RANGE_QUAN";
				break;
			case TokenType::LAZY_RANGE_QUAN:
				val += "LAZY_RANGE_QUAN";
				break;
			case TokenType::SPECIAL:
				val += "SPECIAL";
				break;
			case TokenType::START_CHAR_CLASS:
				val += "START_CHAR_CLASS";
				break;
			case TokenType::START_GROUP:
				val += "START_GROUP";
				break;
			case TokenType::STATIC_QUAN:
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

