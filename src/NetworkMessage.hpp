#pragma once

#include <cstdint>
#include <string>
#include <cstring>
#include <type_traits>
#include <sstream>

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

		void reset()
        {
			info = {};
		}
		template<typename T>
		typename std::enable_if<!(std::is_same<T, std::string>::value), T>::type get()
		{
			if (!canRead(sizeof(T)))
			{
				return static_cast<T>(4);
			}

			T v;
			std::memcpy(&v, buffer + info.position, sizeof(T));
			info.position += sizeof(T);
			return v;
		}
		template<typename T>
		typename std::enable_if<std::is_same<T, std::string>::value, T>::type get()
		{
			auto size = get<unsigned int>();
			if (!canRead(size))
			{
				std::stringstream str;
				str << "Invalid data. String size " << size << std::endl;
				str << "Actual index " << info.position << std::endl;
				str << "Data size: " << size << std::endl;

				return str.str();
			}

			std::string val (reinterpret_cast<char*>(buffer) + info.position, size);
			info.position += size;
			return std::move(val);
		}
		MsgSize_t getLength() const
		{
			return info.length;
		}

		void setLength(MsgSize_t newLength)
		{
			info.length = newLength;
		}
		NetworkMessage& operator+=(unsigned int value)
		{
			info.position += value;

			return *this;
		}

		MsgSize_t getBufferPosition() const
		{
			return info.position;
		}

		bool setBufferPosition(MsgSize_t pos)
		{
			if (pos < NETWORKMESSAGE_MAXSIZE - INITIAL_BUFFER_POSITION)
			{
				info.position = pos;
				return true;
			}
			return false;
		}

		uint16_t getLengthHeader() const
		{
			return static_cast<uint16_t>(buffer[0] | buffer[1] << 8);
		}

		uint8_t* getBuffer()
		{
			return buffer;
		}

		const uint8_t* getBuffer() const
		{
			return buffer;
		}

		uint8_t* getBodyBuffer()
		{
			info.position = 2;
			return buffer + 2;
		}

	protected:
		struct NetworkMessageInfo
		{
			MsgSize_t length = 0;
			MsgSize_t position = INITIAL_BUFFER_POSITION;
		};

		NetworkMessageInfo info;
		uint8_t buffer[NETWORKMESSAGE_MAXSIZE];

	private:
		bool canAdd(size_t size) const
		{
			return (size + info.position) < MAX_BODY_LENGTH;
		}

		bool canRead(int32_t size)
		{
			return !((info.position + size) > (info.length + 8) || size >= (NETWORKMESSAGE_MAXSIZE - info.position));
		}
};
