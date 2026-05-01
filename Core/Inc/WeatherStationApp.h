#pragma once

#include "AppConfig.h"
#include "SensorManager.h"
#include "DataStorage.h"
#include "WeatherAnalyzer.h"
#include "DisplayManager.h"

namespace WeatherStation {
	class WeatherStationApp {
		public:
			WeatherStationApp(AppConfig& cfg, SensorManager& sensor, DataStorage& storage, WeatherAnalyzer& analyzer, DisplayManager& display);

			bool init();

			void run();

		private:
			void tickSensor();
			void tickAnalyzer();
			void tickDisplay();

			AppConfig& m_cfg;
			SensorManager& m_sensor;
			DataStorage& m_storage;
			WeatherAnalyzer& m_analyzer;
			DisplayManager& m_display;

			uint32_t m_lastSensorMs = 0;
			uint32_t m_lastAnalyzerMs = 0;
			uint32_t m_lastDisplayMs = 0;

			bool m_initialised = false;
	};
}
