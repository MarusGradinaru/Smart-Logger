#include "SmartLoggerCode.h"
#include <esp_ota_ops.h>
#ifdef USE_WIFI_LOGGER
  #include <WiFi.h>
#endif

//-------------- Common Public -------------------

SmartLogger::SmartLogger() {
  logMutex = xSemaphoreCreateRecursiveMutexStatic(&mutexBuffer);
}

bool SmartLogger::Log(const char* msg, bool newLine) {
  takeAccess();
  bool result = true; 
  size_t len = msg != nullptr ? strlen(msg) : 0;
  #ifdef USE_SERIAL_LOGGER
    if (serialStarted)
      if (!serialSend(msg, len, newLine)) result = false;
  #endif
  #ifdef USE_WIFI_LOGGER
    if (wifiStarted) 
      if (!tcpSend(msg, len, newLine)) result = false;
  #endif
  giveAccess();
  return result;
}

bool SmartLogger::LogF(const char* fmt, bool newLine, ...) {
  takeAccess();
  char buffer[256]; char* buff = nullptr; size_t len = 0;
  if (fmt != nullptr && strlen(fmt) > 0) {
    va_list args;
    va_start(args, newLine);
    int count = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    if (count > 0) {
      buff = buffer;
      len = count > sizeof(buffer) - 1 ? sizeof(buffer) - 1 : count; }
  }
  bool result = true;
  #ifdef USE_SERIAL_LOGGER
    if (serialStarted)
      if (!serialSend(buff, len, newLine)) result = false;  
  #endif
  #ifdef USE_WIFI_LOGGER
    if (wifiStarted) 
      if(!tcpSend(buff, len, newLine)) result = false;
  #endif  
  giveAccess();
  return result;
}

bool SmartLogger::LogBuff(const uint8_t* buffer, size_t len, bool newLine) {
  takeAccess();
  bool result = true;
  #ifdef USE_SERIAL_LOGGER
    if (serialStarted)
      if (!serialSendBuff(buffer, len, newLine)) result = false;
  #endif
  #ifdef USE_WIFI_LOGGER
    if (wifiStarted)
      if (!tcpSendBuff(buffer, len, newLine)) result = false;
  #endif
  giveAccess();
  return result;
}

bool SmartLogger::LogFirmInfo(const char* head, const char* ver) {
  const esp_partition_t* run_part = esp_ota_get_running_partition();
  return LogF("%s Running firmware version %s from \"%s\" partition.", true, head, ver, run_part->label);
}


//-------------- Common Private ----------------

const char* SmartLogger::RReasonToStr(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_POWERON:    return "Power On";
    case ESP_RST_EXT:        return "External pin";
    case ESP_RST_SW:         return "Soft reset";
    case ESP_RST_PANIC:      return "Panic !";
    case ESP_RST_INT_WDT:    return "Interrupt watchdog";
    case ESP_RST_TASK_WDT:   return "Task watchdog";
    case ESP_RST_WDT:        return "Other watchdogs";
    case ESP_RST_DEEPSLEEP:  return "Deep sleep exit";
    case ESP_RST_BROWNOUT:   return "Brownout reset";
    case ESP_RST_SDIO:       return "SDIO reset";
    case ESP_RST_USB:        return "USB peripheral reset";
    case ESP_RST_JTAG:       return "JTAG reset";
    case ESP_RST_EFUSE:      return "eFuse error";
    case ESP_RST_PWR_GLITCH: return "Power glitch";
    case ESP_RST_CPU_LOCKUP: return "CPU lock up";
    default:                 return "Unknown";
  }
}

#ifdef USE_SERIAL_LOGGER

//-------------- Serial Public -----------------

void SmartLogger::startSerial(unsigned long baud) {
  takeAccess();
  Serial.begin(baud); delay(100);
  serialStarted = true;
  giveAccess();
}

//-------------- Serial Private ----------------

bool SmartLogger::serialSend(const char* msg, size_t len, bool newLine) {
  if (msg != nullptr && len > 0)
    if (Serial.print(msg) != len) return false;
  return !newLine || (Serial.println() == 2);
}

bool SmartLogger::serialSendBuff(const uint8_t* buff, size_t len, bool newLine) {
  if (Serial.print("[") != 1) return false;
  if (buff != nullptr && len > 0) {
    char bout[5] = "00, ", bsize = 4;
    for (int i = 0; i < len; i++) {
      bout[0] = hexChars[buff[i] >> 4]; 
      bout[1] = hexChars[buff[i] & 0x0F];
      if (i == len-1) { bout[2] = '\0'; bsize = 2; }
      if (Serial.print(bout) != bsize) return false; }
  }
  if (Serial.print("]") != 1) return false;
  return !newLine || (Serial.println() == 2);
}

#endif

#ifdef USE_WIFI_LOGGER

//-------------- WiFi Public -------------------

bool SmartLogger::startWifi(const char* serverIp, uint16_t serverPort) {
  takeAccess();
  servIp = serverIp; servPort = serverPort;
  wifiStarted = (servIp != nullptr) && (strlen(servIp) > 0) && (servPort != 0);
  giveAccess();
  return wifiStarted;
}

void SmartLogger::LogIP(const char* fmt, IPAddress ip) {
  char ip_buff[16];
  snprintf(ip_buff, sizeof(ip_buff), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  LogF(fmt, true, ip_buff);
}

//-------------- WiFi Private -------------------

bool SmartLogger::tcpSend(const char* data, size_t len, bool newLine) {
  if (!newLine && (data == nullptr || len == 0)) return true;
  if (WiFi.status() != WL_CONNECTED) return false;
  WiFiClient client;
  if (!client.connect(servIp, servPort, 300)) return false;
  bool result = true;
  if (data != nullptr && len > 0)
    result = result && (client.write(data, len) == len); 
  result = result && (!newLine || (client.write("\r\n", 2) == 2));
  delay(1); client.stop();
  return result;
}

bool SmartLogger::tcpSendBuff(const uint8_t* buff, size_t len, bool newLine) {
  if (WiFi.status() != WL_CONNECTED) return false;
  WiFiClient client;
  if (!client.connect(servIp, servPort, 300)) return false;
  bool result = (client.write("[") == 1);
  if (result && buff != nullptr && len > 0) {
    uint8_t bout[5] = "00, ", bsize = 4;
    for (int i = 0; i < len; i++) {
      bout[0] = hexChars[buff[i] >> 4]; 
      bout[1] = hexChars[buff[i] & 0x0F];
      if (i == len-1) bsize = 2;
      if (client.write(bout, bsize) != bsize) { result = false; break; } } 
  }
  result = result && (client.write("]") == 1);
  result = result && (!newLine || (client.write("\r\n", 2) == 2));
  delay(1); client.stop();
  return result;
}

#endif

SmartLogger Debug;
