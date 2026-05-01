#include "BME280Driver.h"
#include <cstring>

namespace WeatherStation {
	BME280Driver::BME280Driver(I2C_HandleTypeDef& hi2c, uint8_t devAddr) : m_hi2c(hi2c), m_addr(devAddr) {}

	SensorError BME280Driver::init()
	{
		m_initialised = false;

		// Verify ID
		uint8_t chipId = 0;
		if (auto e = readReg(kRegChipId, &chipId, 1); e != SensorError::None)
			return e;
		if (chipId != kChipId)
			return SensorError::NotDetected;

		// Soft-reset before configuration
		if (auto e = reset(); e != SensorError::None)
			return e;
		HAL_Delay(10);

		if (auto e = loadCalibration(); e != SensorError::None)
			return e;

		if (auto e = writeReg(kRegCtrlHum, 0x01); e != SensorError::None)
			return e;

		if (auto e = writeReg(kRegConfig, 0x10); e != SensorError::None)
			return e;

		if (auto e = writeReg(kRegCtrlMeas, 0x57); e != SensorError::None)
			return e;

		m_initialised = true;
		return SensorError::None;
	}

	SensorError BME280Driver::reset()
	{
		return writeReg(kRegReset, kResetWord);
	}

	SensorError BME280Driver::readAll(SensorReading& reading)
	{
		if (!m_initialised)
			return SensorError::NotCalibrated;

		uint8_t status = 0;
		uint8_t retries = 10;
		do {
			if (auto e = readReg(kRegStatus, &status, 1); e != SensorError::None)
				return e;
			if (--retries == 0)
				return SensorError::ReadTimeout;
			HAL_Delay(3);
		} while (status & 0x08);

		uint8_t raw[8] = {};
		if (auto e = readReg(kRegPressureMSB, raw, 8); e != SensorError::None)
			return e;

		int32_t adc_P = static_cast<int32_t>((static_cast<uint32_t>(raw[0]) << 12) | (static_cast<uint32_t>(raw[1]) << 4) | (static_cast<uint32_t>(raw[2]) >> 4));

		int32_t adc_T = static_cast<int32_t>((static_cast<uint32_t>(raw[3]) << 12) | (static_cast<uint32_t>(raw[4]) << 4) | (static_cast<uint32_t>(raw[5]) >> 4));

		int32_t adc_H = static_cast<int32_t>((static_cast<uint32_t>(raw[6]) << 8) | static_cast<uint32_t>(raw[7]));

		reading.temperatureC = compensateTemperature(adc_T);
		reading.pressureHPa = compensatePressure(adc_P);
		reading.humidityPct = compensateHumidity(adc_H);
		reading.timestampMs = HAL_GetTick();
		reading.valid = true;

		return SensorError::None;
	}

	bool BME280Driver::isConnected() const
	{
		return HAL_I2C_IsDeviceReady(&m_hi2c, m_addr, 3, 10) == HAL_OK;
	}

	SensorError BME280Driver::readReg(uint8_t reg, uint8_t* buf, uint16_t len) const
	{
		HAL_StatusTypeDef s = HAL_I2C_Mem_Read(const_cast<I2C_HandleTypeDef*>(&m_hi2c), m_addr, reg, I2C_MEMADD_SIZE_8BIT, buf, len, kI2cTimeout);
		if (s == HAL_TIMEOUT) return SensorError::ReadTimeout;
		if (s != HAL_OK) return SensorError::I2cBusError;
		return SensorError::None;
	}

	SensorError BME280Driver::writeReg(uint8_t reg, uint8_t val)
	{
		uint8_t buf[2] = { reg, val };
		HAL_StatusTypeDef s = HAL_I2C_Master_Transmit(&m_hi2c, m_addr, buf, 2, kI2cTimeout);
		if (s == HAL_TIMEOUT) return SensorError::ReadTimeout;
		if (s != HAL_OK) return SensorError::I2cBusError;
		return SensorError::None;
	}

	SensorError BME280Driver::loadCalibration()
	{
		uint8_t calib00[26] = {};
		uint8_t calib26[7] = {};

		if (auto e = readReg(kRegCalib00, calib00, 26); e != SensorError::None)
			return e;
		if (auto e = readReg(kRegCalib26, calib26, 7);  e != SensorError::None)
			return e;

		auto u16 = [](uint8_t lsb, uint8_t msb) -> uint16_t {
			return static_cast<uint16_t>(lsb) | (static_cast<uint16_t>(msb) << 8);
		};
		auto s16 = [&](uint8_t lsb, uint8_t msb) -> int16_t {
			return static_cast<int16_t>(u16(lsb, msb));
		};

		m_calib.dig_T1 = u16(calib00[0], calib00[1]);
		m_calib.dig_T2 = s16(calib00[2], calib00[3]);
		m_calib.dig_T3 = s16(calib00[4], calib00[5]);
		m_calib.dig_P1 = u16(calib00[6], calib00[7]);
		m_calib.dig_P2 = s16(calib00[8], calib00[9]);
		m_calib.dig_P3 = s16(calib00[10], calib00[11]);
		m_calib.dig_P4 = s16(calib00[12], calib00[13]);
		m_calib.dig_P5 = s16(calib00[14], calib00[15]);
		m_calib.dig_P6 = s16(calib00[16], calib00[17]);
		m_calib.dig_P7 = s16(calib00[18], calib00[19]);
		m_calib.dig_P8 = s16(calib00[20], calib00[21]);
		m_calib.dig_P9 = s16(calib00[22], calib00[23]);
		m_calib.dig_H1 = calib00[25];

		m_calib.dig_H2 = s16(calib26[0], calib26[1]);
		m_calib.dig_H3 = calib26[2];
		m_calib.dig_H4 = static_cast<int16_t>((static_cast<int16_t>(calib26[3]) << 4) | (calib26[4] & 0x0F));
		m_calib.dig_H5 = static_cast<int16_t>((static_cast<int16_t>(calib26[5]) << 4) | (calib26[4] >> 4));
		m_calib.dig_H6 = static_cast<int8_t>(calib26[6]);

		return SensorError::None;
	}

	float BME280Driver::compensateTemperature(int32_t adc_T)
	{
		int32_t var1 = (((adc_T >> 3) - (static_cast<int32_t>(m_calib.dig_T1) << 1)) * static_cast<int32_t>(m_calib.dig_T2)) >> 11;

		int32_t var2 = (((((adc_T >> 4) - static_cast<int32_t>(m_calib.dig_T1)) * ((adc_T >> 4) - static_cast<int32_t>(m_calib.dig_T1))) >> 12) * static_cast<int32_t>(m_calib.dig_T3)) >> 14;

		m_tFine = var1 + var2;
		return static_cast<float>((m_tFine * 5 + 128) >> 8) / 100.0f;
	}

	float BME280Driver::compensatePressure(int32_t adc_P) const
	{
		int64_t var1 = static_cast<int64_t>(m_tFine) - 128000;
		int64_t var2 = var1 * var1 * static_cast<int64_t>(m_calib.dig_P6);
		var2 += (var1 * static_cast<int64_t>(m_calib.dig_P5)) << 17;
		var2 += static_cast<int64_t>(m_calib.dig_P4) << 35;
		var1 = ((var1 * var1 * static_cast<int64_t>(m_calib.dig_P3)) >> 8) + ((var1 * static_cast<int64_t>(m_calib.dig_P2)) << 12);
		var1 = ((static_cast<int64_t>(1) << 47) + var1) * static_cast<int64_t>(m_calib.dig_P1) >> 33;

		if (var1 == 0) return 0.f;

		int64_t p = 1048576 - adc_P;
		p = (((p << 31) - var2) * 3125) / var1;
		var1 = (static_cast<int64_t>(m_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
		var2 = (static_cast<int64_t>(m_calib.dig_P8) * p) >> 19;
		p = ((p + var1 + var2) >> 8) + (static_cast<int64_t>(m_calib.dig_P7) << 4);

		return static_cast<float>(p) / 25600.0f;
	}

	float BME280Driver::compensateHumidity(int32_t adc_H) const
	{
		int32_t x = m_tFine - 76800;
		x = (((adc_H << 14) - (static_cast<int32_t>(m_calib.dig_H4) << 20) - (static_cast<int32_t>(m_calib.dig_H5) * x)) + 16384) >> 15;
		x = x * (((((( x * static_cast<int32_t>(m_calib.dig_H6)) >> 10) * (((x * static_cast<int32_t>(m_calib.dig_H3)) >> 11) + 32768)) >> 10) + 2097152) * static_cast<int32_t>(m_calib.dig_H2) + 8192) >> 14;
		x -= (((x >> 15) * (x >> 15)) >> 7) * static_cast<int32_t>(m_calib.dig_H1);
		x  = x < 0 ? 0 : x;
		x  = x > 419430400 ? 419430400 : x;
		return static_cast<float>(x >> 12) / 1024.0f;
	}
}
