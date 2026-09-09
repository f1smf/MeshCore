#include "CRT_RTCClock.h"

#ifdef CRT_RTC_ENABLED

bool CRT_RTCClock::begin(TwoWire& wire) {
  _present = false;
  wire.begin();
  _present = _rtc.begin(&wire);
#if MESH_DEBUG
  if (_present) {
    MESH_DEBUG_PRINTLN("CRT_RTC: DS3231 found at 0x%02X", CRT_RTC_I2C_ADDR);
  } else {
    MESH_DEBUG_PRINTLN("CRT_RTC: DS3231 not found, using fallback clock");
  }
#endif
  return _present;
}

uint32_t CRT_RTCClock::getCurrentTime() {
  if (_present) {
    uint32_t t = _rtc.now().unixtime();
    if (t >= CRT_RTC_MIN_VALID_TIME) {
      return t;
    }
#if MESH_DEBUG
    MESH_DEBUG_PRINTLN("CRT_RTC: DS3231 time not plausible (%lu), using fallback clock", (unsigned long)t);
#endif
  }
  if (_fallback) {
    return _fallback->getCurrentTime();
  }
  return 0;
}

void CRT_RTCClock::setCurrentTime(uint32_t time) {
  if (_present) {
    _rtc.adjust(DateTime(time));
  }
  if (_fallback) {
    _fallback->setCurrentTime(time);
  }
}

#endif // CRT_RTC_ENABLED