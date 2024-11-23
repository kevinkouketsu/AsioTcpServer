#pragma once

#include <cstdint>
#include <string>
#include <cstring>
#include <type_traits>
#include <sstream>
#include "BufferReader.hpp"

static constexpr int32_t NETWORKMESSAGE_MAXSIZE = 24590;

class NetworkMessage
{
public:
	using MsgSize_t = uint16_t;
	static constexpr MsgSize_t INITIAL_BUFFER_POSITION = 0;
	static constexpr auto SIZE_LENGTH = 2;
	static constexpr auto HEADER_LENGTH = SIZE_LENGTH + 10;
	static constexpr auto CHECKSUM_LENGTH = 1;
	static constexpr auto MAX_BODY_LENGTH = NETWORKMESSAGE_MAXSIZE - HEADER_LENGTH;

	NetworkMessage() = default;
	NetworkMessage(uint8_t* data, size_t size)
	{
        buffer = BufferReader{ data, size };
	}
	
    void reset()
    {
        buffer = {};
	}

	template<typename T>
	T get()
	{
        return buffer.Get<T>();
	}

	MsgSize_t getLength() const
	{
		return buffer.getBufferSize();
	}

	void setLength(MsgSize_t newLength)
	{
        if (newLength > buffer.getBufferSize())
            buffer.setBufferSize(newLength);
	}

	NetworkMessage& operator+=(size_t value)
	{
        buffer.advance(value);

		return *this;
	}

	MsgSize_t getBufferPosition() const
	{
		return position;
	}

	bool setBufferPosition(MsgSize_t pos)
	{
		if (pos < NETWORKMESSAGE_MAXSIZE - INITIAL_BUFFER_POSITION)
		{
			position = pos;
			return true;
		}
		return false;
	}

	uint16_t getLengthHeader() const
	{
        auto rawBuffer = buffer.getRawBuffer();
		return static_cast<uint16_t>(rawBuffer[0] | rawBuffer[1] << 8);
	}

	uint8_t* getBuffer()
	{
		return buffer.getRawBuffer();
	}

	const uint8_t* getBuffer() const
	{
		return buffer.getRawBuffer();
	}

	uint8_t* getBodyBuffer()
	{
		position = 2;

		return buffer.getRawBuffer() + SIZE_LENGTH;
	}

protected:
    MsgSize_t position = INITIAL_BUFFER_POSITION;
    BufferReader buffer;

private:
	bool canAdd(size_t size) const
	{
		return (size + position) < MAX_BODY_LENGTH;
	}

	bool canRead(int32_t size) const
	{
		return !((position + size) > (getLength() + 8) || size >= (NETWORKMESSAGE_MAXSIZE - position));
	}
};
