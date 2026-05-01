#pragma once

#include "IBMEDriver.h"
#include "stm32f4xx_hal.h"

namespace WeatherStation {
	class BME280Driver final : public IBMEDriver {
	public:
		explicit BME280Driver(I2C_HandleTypeDef& hi2c, uint8_t devAddr);

		SensorError init() override;
		SensorError reset() override;
		SensorError readAll(SensorReading& reading) override;
		bool isConnected() const override;

	private:
		// Register map
		static constexpr uint8_t kRegCalib00 = 0x88;
		static constexpr uint8_t kRegCalib26 = 0xE1;
		static constexpr uint8_t kRegChipId = 0xD0;
		static constexpr uint8_t kRegReset = 0xE0;
		static constexpr uint8_t kRegCtrlHum = 0xF2;
		static constexpr uint8_t kRegStatus = 0xF3;
		static constexpr uint8_t kRegCtrlMeas = 0xF4;
		static constexpr uint8_t kRegConfig = 0xF5;
		static constexpr uint8_t kRegPressureMSB = 0xF7;

		static constexpr uint8_t kChipId = 0x60;
		static constexpr uint8_t kResetWord = 0xB6;
		static constexpr uint32_t kI2cTimeout = 10; // ms

		struct CalibData {
			uint16_t dig_T1; int16_t dig_T2; int16_t dig_T3;
			uint16_t dig_P1; int16_t dig_P2; int16_t dig_P3;
			int16_t dig_P4; int16_t dig_P5; int16_t dig_P6;
			int16_t dig_P7; int16_t dig_P8; int16_t dig_P9;
			uint8_t dig_H1; int16_t dig_H2; uint8_t dig_H3;
			int16_t dig_H4; int16_t dig_H5; int8_t dig_H6;
		} m_calib {};

		I2C_HandleTypeDef& m_hi2c;
		uint8_t m_addr;
		bool m_initialised = false;
		int32_t m_tFine = 0;

		SensorError readReg(uint8_t reg, uint8_t* buf, uint16_t len) const;
		SensorError writeReg(uint8_t reg, uint8_t val);
		SensorError loadCalibration();

		float compensateTemperature(int32_t adc_T);
		float compensatePressure(int32_t adc_P) const;
		float compensateHumidity(int32_t adc_H) const;
	};
}
