#include "SensorManager.h"
#include <cstring>

namespace WeatherStation {
	SensorManager::SensorManager(IBMEDriver& driver, const AppConfig& cfg) : m_driver(driver), m_cfg(cfg)
	{
		std::memset(m_tempWindow,  0, sizeof(m_tempWindow));
		std::memset(m_humWindow,   0, sizeof(m_humWindow));
		std::memset(m_pressWindow, 0, sizeof(m_pressWindow));
	}

	SensorError SensorManager::init()
	{
		SensorError e = m_driver.init();
		if (e != SensorError::None)
			m_errorCount++;
		return e;
	}

	SensorError SensorManager::poll(SensorReading& out)
	{
		SensorReading raw{};
		SensorError e = m_driver.readAll(raw);
		if (e != SensorError::None) {
			m_errorCount++;
			out.valid = false;
			return e;
		}

		if (!validateReading(raw)) {
			m_errorCount++;
			out.valid = false;
			return SensorError::OutOfRange;
		}

		m_errorCount = 0;

		out.temperatureC = applyFilter(m_tempWindow, m_filterHead, raw.temperatureC);
		uint8_t hHead = m_filterHead;
		uint8_t pHead = m_filterHead;
		out.humidityPct = applyFilter(m_humWindow, hHead, raw.humidityPct);
		out.pressureHPa = applyFilter(m_pressWindow, pHead, raw.pressureHPa);

		const uint8_t win = m_cfg.filterWindowSize < kMaxFilterWindow ? m_cfg.filterWindowSize : kMaxFilterWindow;
		if (m_filterFill < win) m_filterFill++;

		out.timestampMs = raw.timestampMs;
		out.valid = true;
		return SensorError::None;
	}

	bool SensorManager::validateReading(const SensorReading& r) const
	{
		return (r.temperatureC >= m_cfg.tempMinC  && r.temperatureC <= m_cfg.tempMaxC) && (r.humidityPct  >= m_cfg.humMinPct && r.humidityPct  <= m_cfg.humMaxPct) && (r.pressureHPa  >= m_cfg.pressMinHPa && r.pressureHPa <= m_cfg.pressMaxHPa);
	}

	float SensorManager::applyFilter(float* window, uint8_t& head, float newVal)
	{
		const uint8_t win = m_cfg.filterWindowSize < kMaxFilterWindow ? m_cfg.filterWindowSize : kMaxFilterWindow;

		window[head % win] = newVal;
		head = (head + 1) % win;

		uint8_t count = m_filterFill > 0 ? m_filterFill : 1;
		float sum = 0.f;
		for (uint8_t i = 0; i < count; ++i)
			sum += window[i];
		return sum / static_cast<float>(count);
	}
}
