#pragma once

#include <cstring>
#include <string>
#include <vector>
#include <limits>

class BufferWriter
{
	size_t index{ 0 };
	std::vector<unsigned char> data;
public:
    BufferWriter() = default;
	BufferWriter(size_t size)
	{
		data.resize(size);
	}

	void advance(unsigned int size)
	{
		index += size;
	}

	BufferWriter& operator+=(unsigned int value)
	{
		index += value;
		return *this;
	}

	template<typename T>
	BufferWriter& operator<<(const T& lhs)
	{
		this->set<T>(lhs);

		return *this;
	}

	template<typename T>
	void set(T value, size_t position = std::numeric_limits<size_t>::max())
	{
		bool moveIndex{ false };
		if (position == -1)
		{
			position = index;
			moveIndex = true;
		}

		if (position + sizeof(T) > data.size())
			data.resize(position + sizeof (T));

        std::memcpy(&data[position], (void*)&value, sizeof(T));

		if(moveIndex)
			index += sizeof(T);
	}

	template<typename T = std::string>
	void set(const char* value, size_t size)
	{
		set<uint32_t>(size);

		if (index + size > data.size())
			data.resize(index + size);

		std::memcpy((void*)&data[index], (void*)value, size);

		index += size;
	}

	std::vector<unsigned char>& getBuffer()
	{
		return data;
	}

    size_t getSize() const
    {
        return data.size();
    }

	template<typename T>
	T* getAs()
	{
		return reinterpret_cast<T*>(data.data());
	}
};
