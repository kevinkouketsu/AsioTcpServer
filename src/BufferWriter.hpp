#pragma once

#include <vector>
#include <string>

class BufferWriter
{
	size_t index{ 0 };
	std::vector<unsigned char> data;
public:
    BufferWriter() = default;
	BufferWriter(int size)
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
	void set(T value, int position = -1)
	{
		bool moveIndex{ false };
		if (position == -1)
		{
			position = index;
			moveIndex = true;
		}

		if (position + sizeof T > data.size())
			data.resize(position + sizeof T);

        std::memcpy(&data[position], (void*)&value, sizeof T);

		if(moveIndex)
			index += sizeof(T);
	}

	template<typename T = std::string>
	void set(const char* value, size_t size)
	{
		Set<uint32_t>(size);

		if (index + size > data.size())
			data.resize(index + size);

		memcpy_s(&data[index], data.size() - index, value, size);

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
