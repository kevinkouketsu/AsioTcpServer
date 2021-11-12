#pragma once

#include <vector>

template<typename Container = std::vector>
class BufferReader
{
private:
    static constexpr auto HEADER_LENGTH = 12;

	std::vector<unsigned char> data;
	int index{ 0 };

public:
	BufferReader(unsigned char* input, int size)
	{
		data.resize(size);
		memcpy(data.data(), input, size);
	}

	void advance(unsigned int size)
	{
		index += size;
	}

	template<typename T>
	T Get()
	{
		T val = *(T*)&data[index];

		index += sizeof T;
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
};
