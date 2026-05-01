#pragma once

#include "IBMEDriver.h"
#include "AppConfig.h"
#include "Types.h"
#include <array>

namespace WeatherStation {
	class SensorManager {
		public:
			SensorManager(IBMEDriver& driver, const AppConfig& cfg);

			SensorError init();

			SensorError poll(SensorReading& out);

			uint32_t consecutiveErrors() const { return m_errorCount; }

		private:
			static constexpr uint8_t kMaxFilterWindow = 16;

			bool validateReading(const SensorReading& r) const;
			float applyFilter(float* window, uint8_t& head, float newVal);

			IBMEDriver& m_driver;
			const AppConfig& m_cfg;
			uint32_t m_errorCount = 0;

			float m_tempWindow [kMaxFilterWindow] = {};
			float m_humWindow [kMaxFilterWindow] = {};
			float m_pressWindow [kMaxFilterWindow] = {};
			uint8_t m_filterHead = 0;
			uint8_t m_filterFill = 0;
	};
}
