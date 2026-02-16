#pragma once

#include <Arduino.h>

class SmartLogger {
  public:
    explicit SmartLogger();

    inline void takeAccess() { xSemaphoreTakeRecursive(logMutex, portMAX_DELAY); }
    inline void giveAccess() { xSemaphoreGiveRecursive(logMutex); }

    bool Log(const char* msg, bool newLine = false);
    bool LogF(const char* fmt, bool newLine, ...);
    bool LogBuff(const uint8_t* buffer, size_t len, bool newLine = false);

    inline bool LogDone(bool newLine = false) 
      { return Log(msg_Done, newLine); }
    inline bool LogFail(bool newLine = false) 
      { return Log(msg_Fail, newLine); }
    inline bool LogRReason(const char* fmt, esp_reset_reason_t reason) 
      { return LogF(fmt, true, RReasonToStr(reason)); }

    bool LogFirmInfo(const char* head, const char* ver);

    #ifdef USE_SERIAL_LOGGER
      void startSerial(unsigned long baud);
    #endif

    #ifdef USE_WIFI_LOGGER
      bool startWifi(const char* serverIp, uint16_t serverPort);
      void LogIP(const char* fmt, IPAddress ip);
    #endif

  private:
    static constexpr char msg_Done[]   = " done !";
    static constexpr char msg_Fail[]   = " failed !";
    static constexpr char hexChars[]   = "0123456789ABCDEF";

    #ifdef USE_SERIAL_LOGGER
      bool serialStarted = false;
    #endif

    #ifdef USE_WIFI_LOGGER
      const char* servIp;
      uint16_t servPort;
      bool wifiStarted = false;
    #endif
    
    StaticSemaphore_t mutexBuffer;
    SemaphoreHandle_t logMutex;

    const char* RReasonToStr(esp_reset_reason_t reason);

    #ifdef USE_SERIAL_LOGGER
      bool serialSend(const char* msg, size_t len, bool newLine);
      bool serialSendBuff(const uint8_t* buff, size_t len, bool newLine);
    #endif

    #ifdef USE_WIFI_LOGGER
      bool tcpSend(const char* data, size_t len, bool newLine);
      bool tcpSendBuff(const uint8_t* buff, size_t len, bool newLine);
    #endif
};

extern SmartLogger Debug;
