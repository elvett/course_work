#pragma once

#include <cstdint>

namespace WeatherStation {
	struct AppConfig {
		// Timing
		uint32_t sensorPollIntervalMs   = 2000; // BME280 read period
		uint32_t displayUpdateIntervalMs = 1000; // OLED refresh period
		uint32_t analyzerRunIntervalMs  = 60000; // Weather analysis period

		// Storage
		static constexpr uint16_t kHistoryDepth = 288; // 288 samples * 5 min = 24h

		// Sensor validation bounds
		float tempMinC    = -40.0f;
		float tempMaxC    =  85.0f;
		float humMinPct   =   0.0f;
		float humMaxPct   = 100.0f;
		float pressMinHPa = 300.0f;
		float pressMaxHPa = 1100.0f;

		// Noise filter
		uint8_t filterWindowSize = 5; // Moving-average window for sensor data

		// Forecast thresholds
		float pressDropStormHPaPerHour  = -3.0f; // Rapid fall to storm likely
		float pressDropRainHPaPerHour   = -1.5f; // Moderate fall to rain likely
		float pressRiseImproveHPaPerHour =  1.5f; // Rise to improving

		// I2C addresses
		uint8_t bme280I2cAddr  = 0x76 << 1;
		uint8_t ssd1306I2cAddr = 0x3C << 1;

		// Display
		uint8_t displayBrightness = 0xCF; // SSD1306 contrast register value
		uint32_t screenCyclePeriodMs = 4000; // Auto-rotate screens every N ms
	};
}
