#pragma once

#include <cstdint>
#include <string>

static constexpr int32_t NETWORKMESSAGE_MAXSIZE = 24590;

class NetworkMessage
{
	public:
		using MsgSize_t = uint16_t;
		static constexpr MsgSize_t INITIAL_BUFFER_POSITION = 0;
		static constexpr auto HEADER_LENGTH = 2;
		static constexpr auto CHECKSUM_LENGTH = 1;
		static constexpr auto MAX_BODY_LENGTH = NETWORKMESSAGE_MAXSIZE - HEADER_LENGTH;

		NetworkMessage() = default;

		void reset()
        {
			info = {};
		}

		uint8_t getByte()
        {
			if (!canRead(1)) {
				return 0;
			}

			return buffer[info.position++];
		}

		uint8_t getPreviousByte()
        {
			return buffer[--info.position];
		}

		template<typename T>
		T get() {
			if (!canRead(sizeof(T))) {
				return 0;
			}

			T v;
			memcpy(&v, buffer + info.position, sizeof(T));
			info.position += sizeof(T);
			return v;
		}

		std::string getString(uint16_t stringLen = 0);

		// skips count unknown/unused bytes in an incoming message
		void skipBytes(int16_t count)
        {
			info.position += count;
		}

		// simply write functions for outgoing message
		void addByte(uint8_t value) {
			if (!canAdd(1)) {
				return;
			}

			buffer[info.position++] = value;
			info.length++;
		}

		template<typename T>
		void add(T value) {
			if (!canAdd(sizeof(T))) {
				return;
			}

			memcpy(buffer + info.position, &value, sizeof(T));
			info.position += sizeof(T);
			info.length += sizeof(T);
		}

		void addBytes(const char* bytes, size_t size);
		void addPaddingBytes(size_t n);

		void addString(const std::string& value);

		void addDouble(double value, uint8_t precision = 2);

		// write functions for complex types
		MsgSize_t getLength() const {
			return info.length;
		}

		void setLength(MsgSize_t newLength) {
			info.length = newLength;
		}

		MsgSize_t getBufferPosition() const {
			return info.position;
		}

		bool setBufferPosition(MsgSize_t pos) {
			if (pos < NETWORKMESSAGE_MAXSIZE - INITIAL_BUFFER_POSITION) {
				info.position = pos + INITIAL_BUFFER_POSITION;
				return true;
			}
			return false;
		}

		uint16_t getLengthHeader() const {
			return static_cast<uint16_t>(buffer[0] | buffer[1] << 8);
		}

		bool isOverrun() const {
			return info.overrun;
		}

		uint8_t* getBuffer() {
			return buffer;
		}

		const uint8_t* getBuffer() const {
			return buffer;
		}

		uint8_t* getBodyBuffer() {
			info.position = 2;
			return buffer + HEADER_LENGTH;
		}

	protected:
		struct NetworkMessageInfo {
			MsgSize_t length = 0;
			MsgSize_t position = INITIAL_BUFFER_POSITION;
			bool overrun = false;
		};

		NetworkMessageInfo info;
		uint8_t buffer[NETWORKMESSAGE_MAXSIZE];

	private:
		bool canAdd(size_t size) const {
			return (size + info.position) < MAX_BODY_LENGTH;
		}

		bool canRead(int32_t size) {
			if ((info.position + size) > (info.length + 8) || size >= (NETWORKMESSAGE_MAXSIZE - info.position)) {
				info.overrun = true;
				return false;
			}
			return true;
		}
};
