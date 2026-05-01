#include "DisplayManager.h"
#include <cstdio>
#include <cstring>

namespace WeatherStation {
	DisplayManager::DisplayManager(ISSD1306Driver& oled, const AppConfig& cfg) : m_oled(oled), m_cfg(cfg) {}

	bool DisplayManager::init()
	{
		return m_oled.init();
	}

	void DisplayManager::setReading(const SensorReading& r) { m_reading  = r; }
	void DisplayManager::setForecast(const WeatherForecast& f) { m_forecast = f; }

	void DisplayManager::update()
	{
		uint32_t now = HAL_GetTick();
		if (now - m_lastAutoRotateMs >= m_cfg.screenCyclePeriodMs) {
			m_lastAutoRotateMs = now;
			nextScreen();
			return;
		}

		m_oled.clear();
		renderStatusBar();

		switch (m_currentScreen) {
			case DisplayScreen::CurrentConditions:
				renderCurrentConditions();
				break;
			case DisplayScreen::Forecast1h:
				renderForecastScreen(ForecastHorizon::H1, "1h Forecast");
				break;
			case DisplayScreen::Forecast6h:
				renderForecastScreen(ForecastHorizon::H6, "6h Forecast");
				break;
			case DisplayScreen::Forecast24h:
				renderForecastScreen(ForecastHorizon::H24, "24h Forecast");
				break;
			default:
				break;
		}

		m_oled.flush();
	}

	void DisplayManager::nextScreen()
	{
		uint8_t next = (static_cast<uint8_t>(m_currentScreen) + 1) % static_cast<uint8_t>(DisplayScreen::kCount);
		m_currentScreen = static_cast<DisplayScreen>(next);
	}

	void DisplayManager::setScreen(DisplayScreen s)
	{
		m_currentScreen = s;
	}

	void DisplayManager::renderStatusBar()
	{
		m_oled.drawString(0, 0, "WeatherStn", 1);
		m_oled.drawHLine(0, 9, ISSD1306Driver::kWidth);
	}

	void DisplayManager::renderCurrentConditions()
	{
		char buf[24];

		snprintf(buf, sizeof(buf), "T: %.1fC", static_cast<double>(m_reading.temperatureC));
		m_oled.drawString(0, 12, buf, 2);

		snprintf(buf, sizeof(buf), "H: %.0f%%", static_cast<double>(m_reading.humidityPct));
		m_oled.drawString(0, 30, buf, 1);

		snprintf(buf, sizeof(buf), "P: %.1fhPa", static_cast<double>(m_reading.pressureHPa));
		m_oled.drawString(0, 42, buf, 1);

		const HorizonForecast& hf1 = m_forecast.at(ForecastHorizon::H1);
		const char* icon = conditionIcon(hf1.condition);
		m_oled.drawString(100, 30, icon, 1);

		if (hf1.stormRisk) {
			m_oled.drawString(0, 54, "!! STORM RISK !!", 1);
		}
	}

	void DisplayManager::renderForecastScreen(ForecastHorizon h, const char* label)
	{
		const HorizonForecast& hf = m_forecast.at(h);
		char buf[24];

		m_oled.drawString(0, 12, label, 1);
		m_oled.drawHLine(0, 21, ISSD1306Driver::kWidth);

		snprintf(buf, sizeof(buf), "%s", conditionLabel(hf.condition));
		m_oled.drawString(0, 24, buf, 1);

		snprintf(buf, sizeof(buf), "Conf: %d%%", hf.confidence);
		m_oled.drawString(0, 34, buf, 1);

		snprintf(buf, sizeof(buf), "dP:%.1f hPa/h", static_cast<double>(hf.pressDeltaHPa));
		m_oled.drawString(0, 44, buf, 1);

		snprintf(buf, sizeof(buf), "dT:%.1f C/h", static_cast<double>(hf.tempDeltaC));
		m_oled.drawString(0, 54, buf, 1);
	}

	const char* DisplayManager::conditionLabel(WeatherCondition c) const
	{
		switch (c) {
			case WeatherCondition::Clear: return "Clear";
			case WeatherCondition::PartlyCloudy: return "Part.Cloudy";
			case WeatherCondition::Cloudy: return "Cloudy";
			case WeatherCondition::LightRain: return "Light Rain";
			case WeatherCondition::Rain: return "Rain";
			case WeatherCondition::HeavyRain: return "Heavy Rain";
			case WeatherCondition::Thunderstorm: return "Thunderstorm";
			case WeatherCondition::Improving: return "Improving";
			case WeatherCondition::Deteriorating: return "Deterior.";
			default: return "Unknown";
		}
	}

	const char* DisplayManager::conditionIcon(WeatherCondition c) const
	{
		switch (c) {
			case WeatherCondition::Clear: return "SUN";
			case WeatherCondition::PartlyCloudy: return "P/C";
			case WeatherCondition::Cloudy: return "CLD";
			case WeatherCondition::LightRain: return "DRP";
			case WeatherCondition::Rain: return "RAN";
			case WeatherCondition::HeavyRain: return "HVY";
			case WeatherCondition::Thunderstorm: return "!T!";
			case WeatherCondition::Improving: return " ^ ";
			case WeatherCondition::Deteriorating: return " v ";
			default: return "???";
		}
	}
}
