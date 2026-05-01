#pragma once

#include "DataStorage.h"
#include "AppConfig.h"
#include "Types.h"

namespace WeatherStation {
	class WeatherAnalyzer {
		public:
			WeatherAnalyzer(const DataStorage& storage, const AppConfig& cfg);

			void analyze();

			const WeatherForecast& forecast() const { return m_forecast; }

		private:
			struct TrendData {
				float pressRateHPaPerHour = 0.f;
				float tempRatePerHour = 0.f;
				float avgHumidity = 0.f;
				float currentPressHPa = 0.f;
				bool valid = false;
			};

			TrendData computeTrend(uint16_t windowSamples) const;

			HorizonForecast forecastHorizon(const TrendData& now, const TrendData& extended) const;

			WeatherCondition conditionFromPressure(float rateHPaPerHour, float absHPa, float humidity) const;

			uint8_t confidenceFromDataQuality(uint16_t samplesAvailable, uint16_t samplesNeeded) const;

			const DataStorage& m_storage;
			const AppConfig& m_cfg;
			WeatherForecast m_forecast{};

			static constexpr uint16_t kSamplesPerHour = 12;
	};
}
