#pragma once

#include <cstdint>

namespace WeatherStation {
	class ISSD1306Driver {
		public:
			virtual ~ISSD1306Driver() = default;

			virtual bool init() = 0;
			virtual void clear() = 0;
			virtual void flush() = 0;

			virtual void drawPixel(uint8_t x, uint8_t y, bool on) = 0;
			virtual void drawChar(uint8_t x, uint8_t y, char c, uint8_t scale = 1) = 0;
			virtual void drawString(uint8_t x, uint8_t y, const char* str, uint8_t scale = 1) = 0;
			virtual void drawHLine(uint8_t x, uint8_t y,uint8_t len) = 0;
			virtual void drawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, bool filled = false) = 0;

			static constexpr uint8_t kWidth = 128;
			static constexpr uint8_t kHeight = 64;
	};
}
