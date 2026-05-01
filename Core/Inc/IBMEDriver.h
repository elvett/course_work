#pragma once

#include "Types.h"

namespace WeatherStation {
	class IBMEDriver {
	public:
		virtual ~IBMEDriver() = default;

		virtual SensorError init() = 0;
		virtual SensorError reset() = 0;

		virtual SensorError readAll(SensorReading& reading) = 0;

		virtual bool isConnected() const = 0;
	};
}
