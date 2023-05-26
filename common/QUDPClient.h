#pragma once
#include "Package.h"
#include "CSocketInit.h"
#include <vector>
#include <map>
#include <memory>
#include <functional>

#include "SocketChannel.h"

using UpdateUIFunc2 = std::function<void(QUDP_TYPE, size_t, std::vector<char>&)>;
using UpdateUIFunc = std::function<void(QUDP_TYPE, char*)>;
//using UpdateUIFunc = std::function<void(QUDP_TYPE, Package&)>;

class QUDPClient
{
public:
  QUDPClient() : m_nServerConvId(0) {}

  bool ConnectServer(short nPort, const char* szIp); //客户端使用

  //根据流来接收和发送数据
  void Recv(char* buf, int nLen);
  void Send(char* buf, int nLen);

  void SendPackage(const Package& package);
  bool RecvPackage(Package& package);
  void StartWork(UpdateUIFunc func);
  void StopWork() {
    m_bWorking = false;
  }
private:
  //工作线程标志
  bool m_bWorking = false;
  //工作线程函数
  void WorkingThread();

public:
  SocketChannel m_sockChannel;
  uint32_t m_nServerConvId;
  UpdateUIFunc m_uiFunc;
};

