#pragma once

#include "ISSD1306Driver.h"
#include "AppConfig.h"
#include "Types.h"

namespace WeatherStation {
	class DisplayManager {
		public:
			DisplayManager(ISSD1306Driver& oled, const AppConfig& cfg);

			bool init();

			void setReading(const SensorReading& r);
			void setForecast(const WeatherForecast& f);

			void update();

			void nextScreen();
			void setScreen(DisplayScreen s);

		private:
			void renderCurrentConditions();
			void renderForecastScreen(ForecastHorizon h, const char* label);
			void renderStatusBar();

			const char* conditionLabel(WeatherCondition c) const;
			const char* conditionIcon (WeatherCondition c) const;

			ISSD1306Driver& m_oled;
			const AppConfig& m_cfg;

			SensorReading m_reading{};
			WeatherForecast m_forecast{};
			DisplayScreen m_currentScreen = DisplayScreen::CurrentConditions;

			uint32_t m_lastAutoRotateMs = 0;
	};
}
