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
	BufferWriter(std::vector<unsigned char> data)
		: data { std::move(data) }
	{}

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

    BufferWriter& operator<<(const std::string& lhs)
    {
        this->set(lhs);

        return *this;
    }

    template<typename T>
    BufferWriter& operator<<(const T& lhs)
    {
        this->set<T>(lhs);

        return *this;
    }

    template<typename T>
    std::enable_if_t<!std::is_same_v<T, std::string>> set(T value, size_t position = std::numeric_limits<size_t>::max())
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

    void set(const std::string& input)
    {
        set<unsigned int>(input.size());

        if (index + input.size() > data.size())
            data.resize(index + input.size());

        memcpy_s(&data[index], data.size() - index, input.data(), input.size());

        index += input.size();
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
