#pragma once

#include <cstdint>
#include <cstring>

namespace WeatherStation {

	// Sensor
	enum class SensorError : uint8_t {
		None = 0,
		NotDetected = 1,
		ReadTimeout = 2,
		CrcFail = 3,
		OutOfRange = 4,
		NotCalibrated = 5,
		I2cBusError = 6,
	};

	struct SensorReading {
		float temperatureC;
		float humidityPct;
		float pressureHPa;
		uint32_t timestampMs;
		bool valid;

		SensorReading() : temperatureC(0.f), humidityPct(0.f), pressureHPa(0.f), timestampMs(0), valid(false) {}
	};

	// Weather forecast
	enum class WeatherCondition : uint8_t {
		Unknown = 0,
		Clear = 1,
		PartlyCloudy = 2,
		Cloudy = 3,
		LightRain = 4,
		Rain = 5,
		HeavyRain = 6,
		Thunderstorm = 7,
		Improving = 8,
		Deteriorating = 9,
	};

	enum class ForecastHorizon : uint8_t {
		H1 = 0,
		H6 = 1,
		H12 = 2,
		H24 = 3,
		kCount = 4,
	};

	struct HorizonForecast {
		WeatherCondition condition = WeatherCondition::Unknown;
		uint8_t confidence = 0;
		float tempDeltaC = 0.f;
		float pressDeltaHPa = 0.f;
		bool stormRisk = false;
	};

	struct WeatherForecast {
		HorizonForecast horizons[static_cast<uint8_t>(ForecastHorizon::kCount)];

		HorizonForecast& at(ForecastHorizon h) {
			return horizons[static_cast<uint8_t>(h)];
		}
		const HorizonForecast& at(ForecastHorizon h) const {
			return horizons[static_cast<uint8_t>(h)];
		}
	};

	// Display
	enum class DisplayScreen : uint8_t {
		CurrentConditions = 0,
		Forecast1h = 1,
		Forecast6h = 2,
		Forecast24h = 3,
		kCount = 4,
	};
}
