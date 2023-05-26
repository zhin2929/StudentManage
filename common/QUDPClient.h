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

  bool ConnectServer(short nPort, const char* szIp); //�ͻ���ʹ��

  //�����������պͷ�������
  void Recv(char* buf, int nLen);
  void Send(char* buf, int nLen);

  void SendPackage(const Package& package);
  bool RecvPackage(Package& package);
  void StartWork(UpdateUIFunc func);
  void StopWork() {
    m_bWorking = false;
  }
private:
  //�����̱߳�־
  bool m_bWorking = false;
  //�����̺߳���
  void WorkingThread();

public:
  SocketChannel m_sockChannel;
  uint32_t m_nServerConvId;
  UpdateUIFunc m_uiFunc;
};

