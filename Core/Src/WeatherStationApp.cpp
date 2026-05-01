#include "WeatherStationApp.h"
#include "stm32f4xx_hal.h"

namespace WeatherStation {
	WeatherStationApp::WeatherStationApp(AppConfig& cfg, SensorManager& sensor, DataStorage& storage, WeatherAnalyzer& analyzer, DisplayManager& display) : m_cfg(cfg), m_sensor(sensor), m_storage(storage), m_analyzer(analyzer), m_display(display) {}

	bool WeatherStationApp::init()
	{
		if (!m_display.init()) {
			return false;
		}

		if (SensorError e = m_sensor.init(); e != SensorError::None) {
			return false;
		}

		m_lastSensorMs = HAL_GetTick();
		m_lastAnalyzerMs = HAL_GetTick();
		m_lastDisplayMs = HAL_GetTick();
		m_initialised = true;
		return true;
	}

	void WeatherStationApp::run()
	{
		if (!m_initialised) return;

		uint32_t now = HAL_GetTick();

		if (now - m_lastSensorMs >= m_cfg.sensorPollIntervalMs) {
			m_lastSensorMs = now;
			tickSensor();
		}

		if (now - m_lastAnalyzerMs >= m_cfg.analyzerRunIntervalMs) {
			m_lastAnalyzerMs = now;
			tickAnalyzer();
		}

		if (now - m_lastDisplayMs >= m_cfg.displayUpdateIntervalMs) {
			m_lastDisplayMs = now;
			tickDisplay();
		}
	}

	void WeatherStationApp::tickSensor()
	{
		SensorReading reading{};
		SensorError e = m_sensor.poll(reading);

		if (e == SensorError::None && reading.valid) {
			m_storage.push(reading);
			m_display.setReading(reading);
		}

		if (m_sensor.consecutiveErrors() >= 5) {
			m_sensor.init();
		}
	}

	void WeatherStationApp::tickAnalyzer()
	{
		m_analyzer.analyze();
		m_display.setForecast(m_analyzer.forecast());
	}

	void WeatherStationApp::tickDisplay()
	{
		m_display.update();
	}
}
