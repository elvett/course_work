#pragma once

#include "Types.h"
#include "AppConfig.h"
#include <cstdint>

namespace WeatherStation {
	class DataStorage {
		public:
			explicit DataStorage(const AppConfig& cfg);

			void push(const SensorReading& reading);

			bool latest(SensorReading& out) const;

			uint16_t lastN(SensorReading* buf, uint16_t count) const;

			uint16_t sampleCount() const { return m_count; }
			uint16_t capacity() const { return kCap; }

			void clear();

		protected:
			virtual void persist(const SensorReading& r) {}
			virtual void restore() {}

		private:
			static constexpr uint16_t kCap = AppConfig::kHistoryDepth;

			SensorReading m_buf[kCap];
			uint16_t m_head = 0;
			uint16_t m_count = 0;

			uint16_t indexOf(uint16_t age) const;
	};
}
