#pragma once

template <class T> class UniquePtr
{
private:
	T _content;
public:
	UniquePtr<T>(T content = nullptr)
	{
		_content = content;
	}

	T replace(T newContent)
	{
		T temp = _content;
		_content = newContent;
		return temp;
	}

	bool isNull()
	{
		return _content == nullptr;
	}

	T get()
	{
		return _content;
	}

	/// <summary>
	/// Removes the raw pointer from this unique pointer and returns it
	/// </summary>
	/// <returns></returns>
	T release()
	{
		return replace(nullptr);
	}

	virtual ~UniquePtr<T>()
	{
		if (!isNull())
		{
			delete _content;
		}
	}
};

