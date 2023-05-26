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
  // 定义一个指向 SocketChannel 对象的智能指针类型
  using SocketChannelPtr = std::shared_ptr<SocketChannel>;

public:
  QUDPServer();
  bool StartServer(short nPort, const char* szIp = "0.0.0.0"); //服务端使用

  //根据流来接收和发送数据
  void Recv(char* buf, int nLen);
  void Send(QUDP_TYPE type, const char* buf, int nLen, const SocketChannelPtr& ptr);

  void SendPackage(const Package& package, const SocketChannelPtr& ptr);
  
  using TupleType = std::tuple<MySQLResult, std::shared_ptr<MySQLConnector>>;
  
  //传给线程池执行发送数据给客户端
  void SendPacketFunc(TupleType tuple, SocketChannelPtr channelPtr); 

  void CheckConvId(size_t convId);

  //定义类型别名，指向 MySQLConnector 类中的
  //返回 MySQLResult 类型的成员函数指针，该函数接受一个 std::string& 类型的参数。
  using QueryFunc = MySQLResult (MySQLConnector::*)(std::string&);
  //指定默认形参为 MySQLConnector::Query 函数
  TupleType GetQueryResult(std::string sql, QueryFunc func = &(MySQLConnector::Query));


  bool RecvPackage(Package& package);
  void StartWork();
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

  std::recursive_mutex m_mtxMap;
  std::map<int, SocketChannelPtr> m_channelMap;

  ThreadPool m_threadPool; //线程池
  ConnectionPool* m_pConnPool; //连接池
  MySQLConnector m_conn;
};




#if 0
template<typename TYPE, typename ...ARGS>
inline int QUDPServer::SendPackage(int nLen, const SocketChannelPtr& ptr, ARGS... args) {
  //int nRet = sendto(ptr->m_sock, (char*)&package,
  //  sizeof(package), 0, (sockaddr*)&(ptr->m_destAddr), sizeof(ptr->m_destAddr));

  //申请缓冲区，nLen 是柔性数组的长度，若没有，则填 0
  std::shared_ptr<char> pBuf(new char[sizeof(TYPE) + nLen]);
  //将缓冲区转为 TYPE 类型
  new (pBuf.get()) TYPE(args...);
  //printf("nBitsCount=%d, type=%d\n", sizeof(TYPE) + nLen, pBuf->m_packetType);
  int nRet = sendto(ptr->m_sock, pBuf.get(), sizeof(TYPE) + nLen, 0);
  if (nRet == SOCKET_ERROR) {
    printf("SendPacket fail\n");
    return -1;
  }
  //返回发送字节数
  return nRet;

}
#endif // 0
