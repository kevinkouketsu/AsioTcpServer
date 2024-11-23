#pragma once

#include <cstring>
#include <string>
#include <vector>
#include <limits>

template<typename T>
concept StringLike = std::same_as<T, std::string> || std::same_as<T, const char*>;

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
    void set(T value, size_t position = std::numeric_limits<size_t>::max()) 
        requires (!StringLike<T>)
    {
        bool moveIndex = false;
        if (position == std::numeric_limits<size_t>::max()) {
            position = index;
            moveIndex = true;
        }

        ensureSize(sizeof(T));

        std::memcpy(&data[position], &value, sizeof(T));

        if (moveIndex) {
            index += sizeof(T);
        }
    }

    template<StringLike T>
    void set(T value) {
        std::string_view strValue{ value };

        set<unsigned int>(static_cast<unsigned int>(strValue.size()));

        ensureSize(strValue.size());
        index += strValue.size();
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

private:
    void ensureSize(size_t size) 
    {
        if (index + size > data.size())
            data.resize(index + size);
    }
};
