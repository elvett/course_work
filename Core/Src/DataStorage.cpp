#include "DataStorage.h"
#include <cstring>
#include <algorithm>

namespace WeatherStation {
	DataStorage::DataStorage(const AppConfig& cfg)
	{
		std::memset(m_buf, 0, sizeof(m_buf));
	}

	void DataStorage::push(const SensorReading& reading)
	{
		m_buf[m_head] = reading;
		m_head = (m_head + 1) % kCap;
		if (m_count < kCap) m_count++;
		persist(reading);
	}

	bool DataStorage::latest(SensorReading& out) const
	{
		if (m_count == 0) return false;
		out = m_buf[indexOf(0)];
		return true;
	}

	uint16_t DataStorage::lastN(SensorReading* buf, uint16_t count) const
	{
		uint16_t n = std::min(count, m_count);
		for (uint16_t i = 0; i < n; ++i)
			buf[i] = m_buf[indexOf(n - 1 - i)];
		return n;
	}

	void DataStorage::clear()
	{
		m_head = 0;
		m_count = 0;
		std::memset(m_buf, 0, sizeof(m_buf));
	}

	uint16_t DataStorage::indexOf(uint16_t age) const
	{
		return static_cast<uint16_t>((static_cast<int32_t>(m_head) - 1 - age + kCap * 2) % kCap);
	}
}
