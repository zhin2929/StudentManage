#pragma once
#include "Package.h"
#include "CSocketInit.h"
#include "SocketChannel.h"
#include "ThreadPool.hpp"
#include "MySQLConnector.h"
#include "ConnectionPool.h"

#include <vector>
#include <map>
#include <memory>
#include <string_view>
#include <format>




class QUDPServer {
  // ����һ��ָ�� SocketChannel ���������ָ������
  using SocketChannelPtr = std::shared_ptr<SocketChannel>;

public:
  QUDPServer();
  bool StartServer(short nPort, const char* szIp = "0.0.0.0"); //�����ʹ��

  //�����������պͷ�������
  void Recv(char* buf, int nLen);
  void Send(QUDP_TYPE type, const char* buf, int nLen, const SocketChannelPtr& ptr);

  void SendPackage(const Package& package, const SocketChannelPtr& ptr);
  
  using TupleType = std::tuple<MySQLResult, std::shared_ptr<MySQLConnector>>;
  
  //�����̳߳�ִ�з������ݸ��ͻ���
  void SendPacketFunc(TupleType tuple, SocketChannelPtr channelPtr); 

  void CheckConvId(size_t convId);

  //�������ͱ�����ָ�� MySQLConnector ���е�
  //���� MySQLResult ���͵ĳ�Ա����ָ�룬�ú�������һ�� std::string& ���͵Ĳ�����
  using QueryFunc = MySQLResult (MySQLConnector::*)(std::string&);
  //ָ��Ĭ���β�Ϊ MySQLConnector::Query ����
  TupleType GetQueryResult(std::string sql, QueryFunc func = &(MySQLConnector::Query));


  bool RecvPackage(Package& package);
  void StartWork();
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

  std::recursive_mutex m_mtxMap;
  std::map<int, SocketChannelPtr> m_channelMap;

  ThreadPool m_threadPool; //�̳߳�
  ConnectionPool* m_pConnPool; //���ӳ�
  MySQLConnector m_conn;
};




#if 0
template<typename TYPE, typename ...ARGS>
inline int QUDPServer::SendPackage(int nLen, const SocketChannelPtr& ptr, ARGS... args) {
  //int nRet = sendto(ptr->m_sock, (char*)&package,
  //  sizeof(package), 0, (sockaddr*)&(ptr->m_destAddr), sizeof(ptr->m_destAddr));

  //���뻺������nLen ����������ĳ��ȣ���û�У����� 0
  std::shared_ptr<char> pBuf(new char[sizeof(TYPE) + nLen]);
  //��������תΪ TYPE ����
  new (pBuf.get()) TYPE(args...);
  //printf("nBitsCount=%d, type=%d\n", sizeof(TYPE) + nLen, pBuf->m_packetType);
  int nRet = sendto(ptr->m_sock, pBuf.get(), sizeof(TYPE) + nLen, 0);
  if (nRet == SOCKET_ERROR) {
    printf("SendPacket fail\n");
    return -1;
  }
  //���ط����ֽ���
  return nRet;

}
#endif // 0
