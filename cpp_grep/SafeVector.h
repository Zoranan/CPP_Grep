#pragma once
#include <vector>

namespace rex
{
	using namespace std;

	/// <summary>
	/// Deletes all elements when it goes out of scope, unless its ownership is removed before that happens
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template <class T> class SafeVector
	{
	private:
		vector<T> _contents;

	public:
		void add(T t)
		{
			_contents.push_back(t);
		}

		size_t size()
		{
			return _contents.size();
		}

		T at(size_t i)
		{
			return _contents.at(i);
		}

		T back()
		{
			return _contents.back();
		}

		vector<T> release()
		{
			vector<T> temp = _contents;
			_contents.clear();
			return temp;
		}


		virtual ~SafeVector<T>()
		{
			for (size_t i = 0; i < size(); i++)
			{
				delete at(i);
			}
		}
	};
}

