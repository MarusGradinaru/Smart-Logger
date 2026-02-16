#pragma once

#define Halt()  while(true) delay(1000)

#if defined(USE_SERIAL_LOGGER) || defined(USE_WIFI_LOGGER)

  //------------------- USING SMART LOGGER ------------------------

  #include <SmartLoggerCode.h>

  #ifdef USE_SERIAL_LOGGER
    #define DebugSBlock(block)          block
    #define StartSerialLogger(baud)     Debug.startSerial(baud)
  #else
    #define DebugSBlock(block)          ((void)0)
    #define StartSerialLogger(baud)     ((void)0)
  #endif

  #ifdef USE_WIFI_LOGGER
    #define DebugWBlock(block)          block
    #define StartWifiLogger(ip, port)   Debug.startWifi(ip, port)
    #define PrintIp(fmt, ip)            Debug.LogIP(fmt, ip)
  #else
    #define DebugWBlock(block)          ((void)0)
    #define StartWifiLogger(ip, port)   ((void)0)
    #define PrintIp(fmt, ip)            ((void)0)
  #endif

  #define DebugBlock(block)             block
  #define TakeLogger()                  Debug.takeAccess()
  #define GiveLogger()                  Debug.giveAccess()
  
  #define Print(msg)                    Debug.Log(msg, false)
  #define PrintLn(msg)                  Debug.Log(msg, true)
  #define PrintF(fmt, ...)              Debug.LogF(fmt, false, ##__VA_ARGS__)
  #define PrintFLn(fmt, ...)            Debug.LogF(fmt, true, ##__VA_ARGS__)
  #define PrintDone()                   Debug.LogDone(false)
  #define PrintDoneLn()                 Debug.LogDone(true)
  #define PrintFail()                   Debug.LogFail(false)
  #define PrintFailLn()                 Debug.LogFail(true)
  #define PrintResult(res)              if (res) Debug.LogDone(true); else Debug.LogFail(true)
  #define PrintBuff(buff, len)          Debug.LogBuff(buff, len, false) 
  #define PrintBuffLn(buff, len)        Debug.LogBuff(buff, len, true) 
  #define PrintRReason(fmt, reason)     Debug.LogRReason(fmt, reason)
  #define PrintFirmInfo(head, ver)      Debug.LogFirmInfo(head, ver)

  extern SmartLogger Debug;

#else 

  //---------------- SMART LOGGER DISABLED ------------------------

  #define StartWifiLogger(ip, port)     ((void)0)
  #define PrintIp(fmt, ip)              ((void)0)

  #define DebugBlock(block)             ((void)0)
  #define TakeLogger()                  ((void)0)
  #define GiveLogger()                  ((void)0)

  #define Print(msg)                    ((void)0)
  #define PrintLn(msg)                  ((void)0)
  #define PrintF(fmt, ...)              ((void)0)
  #define PrintFLn(fmt, ...)            ((void)0)
  #define PrintDone()                   ((void)0)
  #define PrintDoneLn()                 ((void)0)
  #define PrintFail()                   ((void)0)
  #define PrintFailLn()                 ((void)0)
  #define PrintResult(res)              ((void)0)
  #define PrintBuff(buff, len)          ((void)0)
  #define PrintBuffLn(buff, len)        ((void)0)
  #define PrintRReason(fmt, reason)     ((void)0) 
  #define PrintFirmInfo(head, ver)      ((void)0)

  #define ListPartitions()              ((void)0)
  #define ListDir(fs, dir, lvl)         ((void)0) 

#endif  

// TO DO:
//  - make WiFi logger deactivate itself after 3 unsuccessful attempts and add a method to reactivate it manually