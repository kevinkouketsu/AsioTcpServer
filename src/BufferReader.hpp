#pragma once

#include <vector>
#include <string>
#include <sstream>

class BufferReader
{
private:
	std::vector<unsigned char> data;
	size_t index{ 0 };

public:
    BufferReader() = default;
	BufferReader(unsigned char* input, size_t size)
	{
		data.resize(size);
		memcpy(data.data(), input, size);
	}

	void advance(size_t size)
	{
		index += size;
	}

    template<typename T>
    BufferReader& operator>>(T& obj)
    {
        obj = this->Get<T>();

        return *this;
    }

	template<typename T>
	T Get()
	{
		T val = *(T*)&data[index];

		index += sizeof(T);
		return val;
	}

	BufferReader& operator+=(unsigned int value)
	{
		index += value;

		return *this;
	}

	template<>
	std::string Get()
	{
		auto size = Get<unsigned int>();
		if (size + index > data.size())
		{
			std::stringstream str;
			str << "Invalid data. String size " << size << std::endl;
			str << "Actual index " << index << std::endl;
			str << "Data size: " << data.size();

			return str.str();
		}

        if (size == 0)
            return {};

		std::vector<unsigned char> temporaryData(size);

		memcpy_s(temporaryData.data(), size, &data[index], size);

		std::string val (temporaryData.begin(), temporaryData.end());
		index += size;
		return val;
	}

	template<typename T>
	T* GetAs()
	{
		return reinterpret_cast<T*>(data.data());
	}

    void setBufferSize(size_t size)
    {
        data.resize(size);
    }

    size_t getBufferSize() const
    {
        return data.size();
    }

    const std::vector<unsigned char>& getBuffer() const
    {
        return data;
    }

    const unsigned char* getRawBuffer() const
    {
        return data.data();
    }

    unsigned char* getRawBuffer()
    {
        return data.data();
    }
};
