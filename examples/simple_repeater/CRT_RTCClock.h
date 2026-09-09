#pragma once

#ifdef CRT_RTC_ENABLED

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>

#include <Mesh.h>

#ifndef CRT_RTC_I2C_ADDR
  #define CRT_RTC_I2C_ADDR 0x68   // DS3231
#endif

#ifndef CRT_RTC_MIN_VALID_TIME
  #define CRT_RTC_MIN_VALID_TIME 1704067200   // 2024-01-01 00:00:00 UTC
#endif

/**
 * \brief  External I2C RTC (DS3231) clock, with fallback on the board's clock.
 *         Active only when CRT_RTC_ENABLED is defined at build time.
 */
class CRT_RTCClock : public mesh::RTCClock {
  mesh::RTCClock* _fallback;
  RTC_DS3231 _rtc;
  bool _present;

public:
  CRT_RTCClock(mesh::RTCClock& fallback) : _fallback(&fallback), _present(false) {}

  bool begin(TwoWire& wire);
  bool isPresent() const { return _present; }

  uint32_t getCurrentTime() override;
  void setCurrentTime(uint32_t time) override;

  void tick() override {
    _fallback->tick();   // keeps the fallback (eg. VolatileRTCClock) running
  }
};

#endif // CRT_RTC_ENABLED