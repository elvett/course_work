#pragma once

#include "ISSD1306Driver.h"
#include "stm32f4xx_hal.h"
#include <array>

namespace WeatherStation {
	class SSD1306Driver final : public ISSD1306Driver {
		public:
			SSD1306Driver(I2C_HandleTypeDef& hi2c, uint8_t devAddr, uint8_t contrast);

			bool init() override;
			void clear() override;
			void flush() override;

			void drawPixel(uint8_t x, uint8_t y, bool on) override;
			void drawChar(uint8_t x, uint8_t y, char c, uint8_t scale) override;
			void drawString(uint8_t x, uint8_t y, const char* str, uint8_t scale) override;
			void drawHLine(uint8_t x, uint8_t y, uint8_t len) override;
			void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool filled) override;

		private:
			static constexpr uint8_t kPages = kHeight / 8;
			static constexpr uint8_t kBufSize = kWidth * kPages;
			static constexpr uint32_t kTimeout  = 10;

			void sendCmd(uint8_t cmd);
			void sendCmdList(const uint8_t* cmds, uint8_t len);

			I2C_HandleTypeDef& m_hi2c;
			uint8_t m_addr;
			uint8_t m_contrast;
			uint8_t m_fb[kBufSize] = {};

			static const uint8_t kFont5x7[][5];
	};
}
