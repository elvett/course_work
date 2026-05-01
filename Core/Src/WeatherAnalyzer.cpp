#include "WeatherAnalyzer.h"
#include <cmath>
#include <algorithm>

namespace WeatherStation {
	WeatherAnalyzer::WeatherAnalyzer(const DataStorage& storage, const AppConfig& cfg)  : m_storage(storage), m_cfg(cfg) {}

	void WeatherAnalyzer::analyze()
	{
		TrendData trend1h = computeTrend(kSamplesPerHour * 1);
		TrendData trend6h = computeTrend(kSamplesPerHour * 6);
		TrendData trend12h = computeTrend(kSamplesPerHour * 12);
		TrendData trend24h = computeTrend(kSamplesPerHour * 24);

		m_forecast.at(ForecastHorizon::H1) = forecastHorizon(trend1h, trend6h);
		m_forecast.at(ForecastHorizon::H6) = forecastHorizon(trend6h, trend12h);
		m_forecast.at(ForecastHorizon::H12) = forecastHorizon(trend12h, trend24h);
		m_forecast.at(ForecastHorizon::H24) = forecastHorizon(trend24h, trend24h);

		bool storm = trend1h.valid && trend1h.pressRateHPaPerHour <= m_cfg.pressDropStormHPaPerHour;
		for (auto& h : m_forecast.horizons)
			h.stormRisk = storm;
	}

	WeatherAnalyzer::computeTrend(uint16_t windowSamples) const
	{
		TrendData td{};
		uint16_t avail = m_storage.sampleCount();
		if (avail < 2) return td;

		uint16_t n = std::min(windowSamples, avail);
		SensorReading buf[AppConfig::kHistoryDepth];
		uint16_t got = m_storage.lastN(buf, n);
		if (got < 2) return td;

		float pressFirst = buf[0].pressureHPa;
		float pressLast = buf[got - 1].pressureHPa;

		uint32_t dtMs = buf[got-1].timestampMs - buf[0].timestampMs;
		float dtHours = (dtMs > 0) ? static_cast<float>(dtMs) / 3600000.f : 1.f;

		td.pressRateHPaPerHour = (pressLast - pressFirst) / dtHours;
		td.currentPressHPa = pressLast;

		float sumHum = 0.f, sumTemp = 0.f;
		for (uint16_t i = 0; i < got; ++i) {
			sumHum += buf[i].humidityPct;
			sumTemp += buf[i].temperatureC;
		}
		td.avgHumidity = sumHum / static_cast<float>(got);
		td.tempRatePerHour = (buf[got-1].temperatureC - buf[0].temperatureC) / dtHours;
		td.valid = true;
		return td;
	}

	HorizonForecast WeatherAnalyzer::forecastHorizon(const TrendData& now, const TrendData& extended) const
	{
		HorizonForecast hf{};
		if (!now.valid) {
			hf.condition = WeatherCondition::Unknown;
			hf.confidence = 0;
			return hf;
		}

		hf.condition = conditionFromPressure(now.pressRateHPaPerHour, now.currentPressHPa, now.avgHumidity);
		hf.pressDeltaHPa = now.pressRateHPaPerHour;
		hf.tempDeltaC = now.tempRatePerHour;

		bool contradiction = extended.valid && ((now.pressRateHPaPerHour > 0.f) != (extended.pressRateHPaPerHour > 0.f));

		hf.confidence = confidenceFromDataQuality(m_storage.sampleCount(), kSamplesPerHour * 6);
		if (contradiction)
			hf.confidence = static_cast<uint8_t>(hf.confidence * 70 / 100);

		return hf;
	}

	WeatherCondition WeatherAnalyzer::conditionFromPressure(float rate, float abs, float humidity) const
	{
		// Rapid fall
		if (rate <= m_cfg.pressDropStormHPaPerHour)
			return WeatherCondition::Thunderstorm;

		// Moderate fall
		if (rate <= m_cfg.pressDropRainHPaPerHour) {
			return (humidity > 75.f) ? WeatherCondition::HeavyRain : WeatherCondition::Rain;
		}

		// Slow fall
		if (rate < -0.5f) {
			return (humidity > 60.f) ? WeatherCondition::LightRain : WeatherCondition::Deteriorating;
		}

		// Rising
		if (rate >= m_cfg.pressRiseImproveHPaPerHour)
			return WeatherCondition::Improving;

		if (rate > 0.5f)
			return (abs > 1013.f) ? WeatherCondition::Clear : WeatherCondition::PartlyCloudy;

		// Stable
		if (abs > 1020.f) return WeatherCondition::Clear;
		if (abs > 1013.f) return WeatherCondition::PartlyCloudy;
		return (humidity > 70.f) ? WeatherCondition::Cloudy : WeatherCondition::PartlyCloudy;
	}

	uint8_t WeatherAnalyzer::confidenceFromDataQuality(uint16_t avail, uint16_t needed) const
	{
		if (avail >= needed) return 90;
		if (avail == 0) return 0;
		uint32_t pct = (static_cast<uint32_t>(avail) * 90) / needed;
		return static_cast<uint8_t>(pct < 10 ? 10 : pct);
	}
}
