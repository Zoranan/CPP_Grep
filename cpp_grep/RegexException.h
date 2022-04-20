#pragma once
#include <exception>
#include <string>

namespace rex
{
	using namespace std;

	class RegexException : public exception
	{
	private:
		string _message;
	public:
		RegexException(string message)
		{
			_message = message;
		}

		string whatStr() const
		{
			return _message;
		}

		virtual const char* what() const throw()
		{
			return _message.c_str();
		}
	};

	class RegexSyntaxException : public RegexException
	{
	private:
		size_t _err_pos;

	public:
		RegexSyntaxException(string message, size_t err_pos) : RegexException(message)
		{
			_err_pos = err_pos;
		}

		size_t at() const
		{
			return _err_pos;
		}

		string get_indicator() const
		{
			string ind;
			ind.reserve(_err_pos);
			for (unsigned short i = 0; i < _err_pos; i++)
			{
				ind.push_back(' ');
			}
			ind.push_back('^');
			return ind;
		}
	};

	class FormatException : public RegexSyntaxException
	{
	public:
		FormatException(string message, size_t err_pos) : RegexSyntaxException(message, err_pos) { }
	};
}

