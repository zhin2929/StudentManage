#pragma once
#include <ws2tcpip.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

class CSocketInit {
  CSocketInit() {
    WSADATA wsaData;
    int nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (nRet != 0) {
      //cout << ("WSAStartup failed\n");
    }
  }

  ~CSocketInit() {
    WSACleanup();
  }
  static CSocketInit m_instance;
};
