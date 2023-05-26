#include "QUDPServer.h"
#include "Log.hpp"
using namespace std;
using namespace std::chrono_literals;
using namespace std::chrono;
using enum QUDP_TYPE;

#if 1

QUDPServer::QUDPServer()
{
#if 0  //单连接
  m_conn.Connect("127.0.0.1", 3306, "root", "toor", "student_info");
#endif //单连接
  //初始化连接池
  m_pConnPool = ConnectionPool::GetConnectionPool();
}

bool QUDPServer::StartServer(short nPort, const char* szIp) {
  m_sockChannel.m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (m_sockChannel.m_sock == INVALID_SOCKET) { return false; };
  sockaddr_in si = {};
  si.sin_family = AF_INET;
  si.sin_port = htons(nPort);
  si.sin_addr.s_addr = inet_addr(szIp);
  int nResult = ::bind(m_sockChannel.m_sock, (sockaddr*)&si, sizeof(si));
  if (nResult == SOCKET_ERROR) {
    return false;
  }
  StartWork();
  return true;
}


void QUDPServer::Recv(char* buf, int nLen) {
  while (m_sockChannel.m_vctBuf.size() < nLen) {
    std::this_thread::sleep_for(100ms);
  }
  std::lock_guard lock(m_sockChannel.m_mtxVct);
  memcpy(buf, m_sockChannel.m_vctBuf.data(), nLen);
  m_sockChannel.m_vctBuf.erase(m_sockChannel.m_vctBuf.begin(),
    m_sockChannel.m_vctBuf.begin() + nLen);
}

/*
发包：nLen是该包的总长度
*/
void QUDPServer::Send(QUDP_TYPE type, const char* buf, int nLen,
  const SocketChannelPtr& ptr) {
  int nCount = nLen % QUDP_MSS == 0
    ? nLen / QUDP_MSS : nLen / QUDP_MSS + 1;

  for (size_t i = 0; i < nCount; ++i) {
    //包数量超过最大缓存则阻塞
    while (ptr->m_mapSend.size() > QUDP_CACHE) {
      std::this_thread::sleep_for(100ms);
    }

    int nLastSize = (i == nCount - 1) ? nLen - i * QUDP_MSS : QUDP_MSS;

    Package package(type, m_sockChannel.m_nConvId,
      ptr->m_nNextSendSeq, buf + (i * QUDP_MSS), nLastSize, nLen);
    //Packet packet(type, m_sockChannel.m_nConvId, ptr->m_nNextSendSeq);

    auto now = chrono::system_clock::now();

    {
      std::lock_guard lock(ptr->m_mtxSend);
      //放进发包容器map里
      PackageInfo pkgInfo = PackageInfo{ now, package };
      ptr->m_mapSend[ptr->m_nNextSendSeq] = pkgInfo;
      ptr->m_nNextSendSeq++; //序号+1
    }
    //发包
    SendPackage(package, ptr);
    std::this_thread::sleep_for(10ms);
  }
}

#if 0
void QUDPServer::Send(QUDP_TYPE type, const char* buf, int nLen, const SocketChannelPtr& ptr) {
  int nCount = nLen % QUDP_MSS == 0
    ? nLen / QUDP_MSS : nLen / QUDP_MSS + 1;

  for (size_t i = 0; i < nCount; ++i) {
    //包数量超过最大缓存则阻塞
    while (ptr->m_mapSend.size() > QUDP_CACHE) {
      std::this_thread::yield();
    }

    int nLastSize = (i == nCount - 1) ? nLen - i * QUDP_MSS : QUDP_MSS;

    Package package(type, m_sockChannel.m_nConvId,
      ptr->m_nNextSendSeq, buf + (i * QUDP_MSS), nLastSize);
    //Packet packet(type, m_sockChannel.m_nConvId, ptr->m_nNextSendSeq);

    auto now = chrono::system_clock::now();

    {
      std::lock_guard lock(ptr->m_mtxSend);
      //放进发包容器map里
      PackageInfo pkgInfo = PackageInfo{ now, package };
      ptr->m_mapSend[ptr->m_nNextSendSeq] = pkgInfo;
      ptr->m_nNextSendSeq++; //序号+1
    }
    //发包
    SendPackage(package, ptr);
  }
}
#endif // 0


void QUDPServer::SendPackage(const Package& package, const SocketChannelPtr& ptr) {
  int nRet = sendto(ptr->m_sock, (char*)&package,
    sizeof(package), 0, (sockaddr*)&(ptr->m_destAddr), sizeof(ptr->m_destAddr));
  //LOG(Log("发包\n"));
}

void QUDPServer::SendPacketFunc(TupleType tuple, SocketChannelPtr channelPtr)
{

  MySQLResult result = std::get<0>(tuple); //获取第1个结果集
  if (result.m_res.get() == nullptr) {
    return;
  }
  MYSQL_ROW row = nullptr;

  size_t count = result.m_res.get()->row_count;
  size_t nDataLen = 0;

  MYSQL_RES* res = result.m_res.get();
  int num_fields = mysql_num_fields(res);
  MYSQL_FIELD* fields = mysql_fetch_fields(res);
  string table = fields->table;

  while ((row = mysql_fetch_row(res))) {
    std::string fmt;
    for (int i = 0; i < num_fields; i++) {
      fmt += std::format("{}::{}----", fields[i].name, row[i] ? row[i] : "");
    }
    Send(S2C_SELECT, (char*)fmt.c_str(), fmt.size(), channelPtr);

  }

}

void QUDPServer::CheckConvId(size_t convId)
{
  //每收到一个包，若是会话ID是新的，则放进 m_channelMap 里
  SocketChannelPtr socketPtr = std::make_shared<SocketChannel>(m_sockChannel);
  socketPtr->m_nConvId = convId;
  std::lock_guard lock(m_mtxMap);
  m_channelMap.insert(std::make_pair(socketPtr->m_nConvId, socketPtr));
}

//using QueryFunc = MySQLResult(MySQLConnector::*)(std::string& sql);
std::tuple<MySQLResult, std::shared_ptr<MySQLConnector>>
QUDPServer::GetQueryResult(std::string sql, QueryFunc func)
{
  //从连接池获取一个连接，生成可调用对象提交 SubmitTask 到线程池任务队列里查询
  //&(MySQLConnector::Query)
  shared_ptr<MySQLConnector> conn = m_pConnPool->GetConnection();

  auto task = std::bind(func, conn.get(), sql);

#if 0  //单连接
  auto task = std::bind(&(MySQLConnector::Query), &m_conn, sql);
#endif //单连接

  auto future = m_threadPool.SubmitTask(task);
  if (!future.valid()) {
    return std::make_tuple(MySQLResult(nullptr, 0), conn);
  }

  return std::make_tuple(future.get(), conn);
}





bool QUDPServer::RecvPackage(Package& package) {
  int nAddrLen = sizeof(m_sockChannel.m_destAddr);
  int nRet = recvfrom(m_sockChannel.m_sock, (char*)&package,
    sizeof(package), 0, (sockaddr*)&m_sockChannel.m_destAddr, &nAddrLen);
  if (nRet == SOCKET_ERROR || nRet == 0) {
    return false;
  }
  CheckConvId(package.m_nConvId);
  return true;
}

void QUDPServer::StartWork() {
  u_long bEnable = true;
  //开启非阻塞模式
  ioctlsocket(m_sockChannel.m_sock, FIONBIO, &bEnable);
  m_bWorking = true;
  thread(&QUDPServer::WorkingThread, this).detach();

  // 生成32位唯一ID
  srand(time(NULL));
  unsigned int convId = rand() << 16 | rand() << 1 | rand() % 2;
  m_sockChannel.m_nConvId = convId;
  //开启线程池
  m_threadPool.SetMode(MODE_CACHED);
  m_threadPool.Start(4); //初始线程
  LOG(std::format("服务端 {} 启动成功", convId));
}

void QUDPServer::WorkingThread() {
  int n = 0;
  while (m_bWorking) {
    { //超时重发
      std::lock_guard lock(m_mtxMap);
      for (const auto& mapPair : m_channelMap) {
        std::lock_guard lock(mapPair.second->m_mtxSend);
        for (auto& packetInfo : mapPair.second->m_mapSend) {
          auto now = system_clock::now();
          auto duration = duration_cast<chrono::milliseconds>(now - packetInfo.second.m_sendTime);
          if (duration.count() > 200000) { //超时，这个时间通常是计算得出
            SendPackage(packetInfo.second.m_packageSend, mapPair.second);
            LOG(Log("服务端超时重发包：ConvId=%d, m_nSeq = %d，m_nSize = %d",
              packetInfo.second.m_packageSend.m_nConvId,
              packetInfo.second.m_packageSend.m_nSeq,
              packetInfo.second.m_packageSend.m_nSize));
            //printf("服务端超时重发包：m_nSeq=%d \n", packetInfo.second.m_packageSend.m_nSeq);
            packetInfo.second.m_sendTime = system_clock::now();
            //break;
          }
        }
      }
    } //锁作用域结束

    //接收数据包
    Package recvPkg;
    if (!RecvPackage(recvPkg)) {
      std::this_thread::sleep_for(10ms);
      continue;
    }
    if (recvPkg.m_nType != QUDP_ACK && recvPkg.m_nType != QUDP_SYN
      && recvPkg.m_nType != QUDP_FIN) {
      //收到发来的包，校验，成功则发送确认包
      int check = PackCheckHash{}(recvPkg.m_buf, recvPkg.m_nSize);
      if (recvPkg.m_nCheck == check) { //校验失败则丢弃
        Package pkgAck(QUDP_ACK, m_sockChannel.m_nConvId, recvPkg.m_nSeq);
        /*LOG(Log("服务端收到 %s 包：ConvId: %d, m_nSeq = %d，m_nSize = %d",
          GetPacketText(recvPkg.m_nType).c_str(), recvPkg.m_nConvId,
          recvPkg.m_nSeq, recvPkg.m_nSize));*/

        SendPackage(pkgAck, m_channelMap[recvPkg.m_nConvId]);
      }
      else {
        LOG(Log("服务端校验失败则丢弃 ConvId: %d", recvPkg.m_nConvId));
        continue;
      }
    }

    switch (recvPkg.m_nType) {
      case C2S_DELETE:
      case C2S_UPDATE:
      case C2S_INSERT: {
        auto& channelPtr = m_channelMap[recvPkg.m_nConvId];
        std::string sql;
        sql.assign(recvPkg.m_buf, recvPkg.m_nSize);
        auto tuple = GetQueryResult(sql, &(MySQLConnector::Update));
        MySQLResult result = std::get<0>(tuple); //获取第1个结果集
        auto conn = std::get<1>(tuple);
        std::string msg; //返回的消息
        if (result.m_nAffectedRows == 0 && !result.m_error.empty()) {
          msg = std::format("失败: {} ", result.m_error);
          Send(S2C_INSERT, (char*)msg.c_str(), msg.size(), channelPtr);
          break;
        }
        msg = "成功";
        Send(S2C_INSERT, (char*)msg.c_str(), msg.size(), channelPtr);
        break;
      }
      case C2S_SELECT: {
        auto& channelPtr = m_channelMap[recvPkg.m_nConvId];
        std::string sql;
        sql.assign(recvPkg.m_buf, recvPkg.m_nSize);

        TupleType tuple = GetQueryResult(sql);
        
        //SendPacketFunc(tuple, channelPtr);
        
        //有括号，编译不通过 &(QUDPServer::SendPacketFunc)
        //auto task = std::bind(&(QUDPServer::SendPacketFunc), this, tuple, channelPtr);
        //下面这一行没有括号，能够编译通过
        auto task = std::bind(&QUDPServer::SendPacketFunc, this, tuple, channelPtr);
        m_threadPool.SubmitTask(task);
     

        break;

      }
      case C2S_SHOW_TABLE: {
        auto& channelPtr = m_channelMap[recvPkg.m_nConvId];
        DBOperatePacket packet = *((DBOperatePacket*)recvPkg.m_buf);
        //同时查询两条语句
    /*    shared_ptr<MySQLConnector> conn1 = m_pConnPool->GetConnection();
        mysql_set_server_option(conn1.get()->GetMySQLConn(), MYSQL_OPTION_MULTI_STATEMENTS_ON);*/
        //GetQueryResult(std::format("USE {};", packet.m_szDbName));
        string sql = std::format("  SHOW TABLES;");

        auto tuple = GetQueryResult(sql);
        MySQLResult result = std::get<0>(tuple); //获取第1个结果集
        auto conn = std::get<1>(tuple); //获取第2个返回值
        //result = conn->GetMySQLResult(); //获取第2个结果集

        if (result.m_res.get() == nullptr) {
          break;
        }

        MYSQL_ROW row = nullptr;
        std::string fmt;
        std::vector<TablePacket> vctTable;
        size_t count = result.m_res.get()->row_count;
        size_t nDataLen = 0;

        while ((row = mysql_fetch_row(result.m_res.get()))) {
          fmt += std::format("{}\n", *row);
          int nStrLen = strlen(*row) + 1;
          TablePacket packet(count, strlen(*row) + 1, *row);
          //nDataLen += packet.m_nLen;
          Send(S2C_SHOW_TABLE, (char*)&packet, sizeof(packet), channelPtr);
          //vctTable.push_back(packet);
        }
        //printf("%s\n",fmt.c_str());  
        mysql_set_server_option(conn.get()->GetMySQLConn(), MYSQL_OPTION_MULTI_STATEMENTS_OFF);
        break;
    }
      case C2S_SHOW_DB: {
        auto& channelPtr = m_channelMap[recvPkg.m_nConvId];

        string sql = "SHOW DATABASES;";
        //获取任务执行结果
        MySQLResult result = std::get<0>(GetQueryResult(sql)); //获取第1个结果集
        if (result.m_res.get() == nullptr) {
          break;
        }
        MYSQL_ROW row = nullptr;
        std::string fmt;
        std::vector<DBPacket> vctDb;
        size_t count = result.m_res.get()->row_count;
        while ((row = mysql_fetch_row(result.m_res.get()))) {
          fmt += std::format("{},", *row);
          DBPacket packet(count, strlen(*row) + 1, *row);
          Send(S2C_SHOW_DB, (char*)&packet, sizeof(packet), channelPtr);
          //vctDb.push_back(packet);
        }
        LOG(fmt);
        //发送回客户端
        //Send(S2C_SHOW_DB, (char*)vctDb.data(), vctDb.size(), channelSockPtr);

        break;
      }
      case QUDP_ACK: //收到确认包
      {
        std::lock_guard lock(m_mtxMap);
        auto& channelPtr = m_channelMap[recvPkg.m_nConvId];
        std::lock_guard lock2(channelPtr->m_mtxSend);
        channelPtr->m_mapSend.erase(recvPkg.m_nSeq);
        //LOG(std::format("服务端收到确认包：{}\n", recvPkg.m_nSeq));
        //printf("服务端收到确认包：%d\n", recvPkg.m_nSeq);
        break;
      }
      case QUDP_PSH: //收到发送包，接收方可以同时是客户端和服务端
      {

        //放进对应客户端的收包容器里
        auto& channelPtr = m_channelMap[recvPkg.m_nConvId];
        channelPtr->m_mapRecv[recvPkg.m_nSeq] = recvPkg;

        while (channelPtr->m_mapRecv.find(channelPtr->m_nNextHandleSeq)
          != channelPtr->m_mapRecv.end()) {
          auto& pkg = channelPtr->m_mapRecv[channelPtr->m_nNextHandleSeq];

          channelPtr->m_mtxVct.lock();
          channelPtr->m_vctBuf.insert(channelPtr->m_vctBuf.end(), pkg.m_buf, pkg.m_buf + pkg.m_nSize);
          channelPtr->m_mtxVct.unlock();

          channelPtr->m_nNextHandleSeq++;
        }

        break;
      }
      case QUDP_SYN: //收到连接包
      {
        
        LOG(std::format("服务端收到连接包 ConvId: {}", recvPkg.m_nConvId));
        //然后返回确认连接
        SendPackage(Package(QUDP_SYN, m_sockChannel.m_nConvId, 0),
          m_channelMap[recvPkg.m_nConvId]);
        break;
      }
      case QUDP_SYNACK: //收到连接确认包
      {
        LOG(std::format("服务端收到连接确认包，会话建立：ConvId: {}", recvPkg.m_nConvId));
        CheckConvId(recvPkg.m_nConvId);
        break;
      }
      case QUDP_FIN:
      {
        break;
      }

      default: break;
  }
}
}


#endif // 0

#if 0

bool MyQUDP::StartServer(short nPort, const char* szIp) {
  m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (m_sock == INVALID_SOCKET) { return false; };
  sockaddr_in si = {};
  si.sin_family = AF_INET;
  si.sin_port = htons(nPort);
  si.sin_addr.s_addr = inet_addr(szIp);
  int nResult = bind(m_sock, (sockaddr*)&si, sizeof(si));
  if (nResult == SOCKET_ERROR) {
    return false;
  }
  StartWork();
  return true;
}

bool MyQUDP::ConnectServer(short nPort, const char* szIp) {
  m_nDestPort = nPort;
  m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (m_sock == INVALID_SOCKET) { return false; };
  m_destAddr.sin_family = AF_INET;
  m_destAddr.sin_port = htons(nPort);
  inet_pton(AF_INET, szIp, &m_destAddr.sin_addr.s_addr);
  //连接

  return true;
}

void MyQUDP::Recv(char* buf, int nLen) {
  while (m_vctBuf.size() < nLen) {
    std::this_thread::yield();
  }
  std::lock_guard lock(m_mtxVct);
  memcpy(buf, m_vctBuf.data(), nLen);
  m_vctBuf.erase(m_vctBuf.begin(), m_vctBuf.begin() + nLen);
}

void MyQUDP::Send(char* buf, int nLen) {
  int nCount = nLen % QUDP_MSS == 0
    ? nLen / QUDP_MSS : nLen / QUDP_MSS + 1;

  for (size_t i = 0; i < nCount; ++i) {
    //包数量超过最大缓存则阻塞
    while (m_mapSend.size() > QUDP_CACHE) {
      std::this_thread::yield();
    }

    int nLastSize = (i == nCount - 1) ? nLen - i * QUDP_MSS : QUDP_MSS;
    Package package(QUDP_PSH,
      m_nNextSendSeq, buf + i * QUDP_MSS,
      nLastSize);
    auto now = chrono::system_clock::now();

    {
      std::lock_guard lock(m_mtxSend);
      //放进发包容器map里
      PackageInfo pkgInfo = PackageInfo{ now, package };
      m_mapSend[m_nNextSendSeq] = pkgInfo;
      m_nNextSendSeq++; //序号+1
    }
    //发包
    SendPackage(package);
  }
}

void MyQUDP::SendPackage(const Package& package) {
  int nRet = sendto(m_sock, (char*)&package,
    sizeof(package), 0, (sockaddr*)&m_destAddr, sizeof(m_destAddr));
}

bool MyQUDP::RecvPackage(Package& package) {
  int nAddrLen = sizeof(m_destAddr);
  int nRet = recvfrom(m_sock, (char*)&package,
    sizeof(package), 0, (sockaddr*)&m_destAddr, &nAddrLen);
  if (nRet == SOCKET_ERROR || nRet == 0) {
    return false;
  }

  return true;
}

void MyQUDP::StartWork() {
  u_long bEnable = true;
  //开启非阻塞模式
  ioctlsocket(m_sock, FIONBIO, &bEnable);
  m_bWorking = true;
  thread(&MyQUDP::WorkingThread, this).detach();
  LOG(Log("启动成功"));
}

void MyQUDP::WorkingThread() {
  while (m_bWorking) {
    { //超时重发
      std::lock_guard lock(m_mtxSend);
      for (auto& pair : m_mapSend) {
        auto now = chrono::system_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(now - pair.second.m_sendTime);
        if (duration.count() > 1000) { //超时，这个时间通常是计算得出
          SendPackage(pair.second.m_packageSend);
          LOG(Log("收到发送包：m_nSeq = %d，m_nSize = ",
            pair.second.m_packageSend.m_nSeq,
            pair.second.m_packageSend.m_nSize));
        }
      }
    }

    //接收数据包
    Package recvPkg;
    if (!RecvPackage(recvPkg)) {
      continue;
    }

    switch (recvPkg.m_nType) {

      case QUDP_ACK: //收到确认包
      {
        std::lock_guard lock(m_mtxSend);
        m_mapSend.erase(recvPkg.m_nSeq);
        LOG(Log("收到确认包"));
        break;
      }
      case QUDP_PSH: //收到发送包，接收方可以同时是客户端和服务端
      {
        //收到发来的包，校验，成功则发送确认包
        int check = PackCheckHash{}(recvPkg.m_buf, recvPkg.m_nSize);
        if (recvPkg.m_nCheck == check) {
          Package pkgAck(QUDP_ACK, recvPkg.m_nSeq);
          SendPackage(pkgAck);
          LOG(Log("收到发送包：m_nSeq = %d，m_nSize = %d", recvPkg.m_nSeq, recvPkg.m_nSize));
        }
        else { //校验失败则丢弃
          LOG(Log("校验失败则丢弃"));
          continue;
        }

        m_mapRecv[recvPkg.m_nSeq] = recvPkg;

        while (m_mapRecv.find(m_nNextHandleSeq)
          != m_mapRecv.end()) {
          auto& pkg = m_mapRecv[m_nNextHandleSeq];
          m_mtxVct.lock();
          m_vctBuf.insert(m_vctBuf.end(), pkg.m_buf, pkg.m_buf + pkg.m_nSize);
          m_mtxVct.unlock();
          m_nNextHandleSeq++;
        }
        break;
      }
      case QUDP_SYN:
      {
        break;
      }
      case QUDP_FIN:
      {
        break;
      }

      default: break;
    }
  }
}


#endif // 0