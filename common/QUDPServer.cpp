#include "QUDPServer.h"
#include "Log.hpp"
using namespace std;
using namespace std::chrono_literals;
using namespace std::chrono;
using enum QUDP_TYPE;

#if 1

QUDPServer::QUDPServer()
{
#if 0  //������
  m_conn.Connect("127.0.0.1", 3306, "root", "toor", "student_info");
#endif //������
  //��ʼ�����ӳ�
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
������nLen�Ǹð����ܳ���
*/
void QUDPServer::Send(QUDP_TYPE type, const char* buf, int nLen,
  const SocketChannelPtr& ptr) {
  int nCount = nLen % QUDP_MSS == 0
    ? nLen / QUDP_MSS : nLen / QUDP_MSS + 1;

  for (size_t i = 0; i < nCount; ++i) {
    //������������󻺴�������
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
      //�Ž���������map��
      PackageInfo pkgInfo = PackageInfo{ now, package };
      ptr->m_mapSend[ptr->m_nNextSendSeq] = pkgInfo;
      ptr->m_nNextSendSeq++; //���+1
    }
    //����
    SendPackage(package, ptr);
    std::this_thread::sleep_for(10ms);
  }
}

#if 0
void QUDPServer::Send(QUDP_TYPE type, const char* buf, int nLen, const SocketChannelPtr& ptr) {
  int nCount = nLen % QUDP_MSS == 0
    ? nLen / QUDP_MSS : nLen / QUDP_MSS + 1;

  for (size_t i = 0; i < nCount; ++i) {
    //������������󻺴�������
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
      //�Ž���������map��
      PackageInfo pkgInfo = PackageInfo{ now, package };
      ptr->m_mapSend[ptr->m_nNextSendSeq] = pkgInfo;
      ptr->m_nNextSendSeq++; //���+1
    }
    //����
    SendPackage(package, ptr);
  }
}
#endif // 0


void QUDPServer::SendPackage(const Package& package, const SocketChannelPtr& ptr) {
  int nRet = sendto(ptr->m_sock, (char*)&package,
    sizeof(package), 0, (sockaddr*)&(ptr->m_destAddr), sizeof(ptr->m_destAddr));
  //LOG(Log("����\n"));
}

void QUDPServer::SendPacketFunc(TupleType tuple, SocketChannelPtr channelPtr)
{

  MySQLResult result = std::get<0>(tuple); //��ȡ��1�������
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
  //ÿ�յ�һ���������ǻỰID���µģ���Ž� m_channelMap ��
  SocketChannelPtr socketPtr = std::make_shared<SocketChannel>(m_sockChannel);
  socketPtr->m_nConvId = convId;
  std::lock_guard lock(m_mtxMap);
  m_channelMap.insert(std::make_pair(socketPtr->m_nConvId, socketPtr));
}

//using QueryFunc = MySQLResult(MySQLConnector::*)(std::string& sql);
std::tuple<MySQLResult, std::shared_ptr<MySQLConnector>>
QUDPServer::GetQueryResult(std::string sql, QueryFunc func)
{
  //�����ӳػ�ȡһ�����ӣ����ɿɵ��ö����ύ SubmitTask ���̳߳�����������ѯ
  //&(MySQLConnector::Query)
  shared_ptr<MySQLConnector> conn = m_pConnPool->GetConnection();

  auto task = std::bind(func, conn.get(), sql);

#if 0  //������
  auto task = std::bind(&(MySQLConnector::Query), &m_conn, sql);
#endif //������

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
  //����������ģʽ
  ioctlsocket(m_sockChannel.m_sock, FIONBIO, &bEnable);
  m_bWorking = true;
  thread(&QUDPServer::WorkingThread, this).detach();

  // ����32λΨһID
  srand(time(NULL));
  unsigned int convId = rand() << 16 | rand() << 1 | rand() % 2;
  m_sockChannel.m_nConvId = convId;
  //�����̳߳�
  m_threadPool.SetMode(MODE_CACHED);
  m_threadPool.Start(4); //��ʼ�߳�
  LOG(std::format("����� {} �����ɹ�", convId));
}

void QUDPServer::WorkingThread() {
  int n = 0;
  while (m_bWorking) {
    { //��ʱ�ط�
      std::lock_guard lock(m_mtxMap);
      for (const auto& mapPair : m_channelMap) {
        std::lock_guard lock(mapPair.second->m_mtxSend);
        for (auto& packetInfo : mapPair.second->m_mapSend) {
          auto now = system_clock::now();
          auto duration = duration_cast<chrono::milliseconds>(now - packetInfo.second.m_sendTime);
          if (duration.count() > 200000) { //��ʱ�����ʱ��ͨ���Ǽ���ó�
            SendPackage(packetInfo.second.m_packageSend, mapPair.second);
            LOG(Log("����˳�ʱ�ط�����ConvId=%d, m_nSeq = %d��m_nSize = %d",
              packetInfo.second.m_packageSend.m_nConvId,
              packetInfo.second.m_packageSend.m_nSeq,
              packetInfo.second.m_packageSend.m_nSize));
            //printf("����˳�ʱ�ط�����m_nSeq=%d \n", packetInfo.second.m_packageSend.m_nSeq);
            packetInfo.second.m_sendTime = system_clock::now();
            //break;
          }
        }
      }
    } //�����������

    //�������ݰ�
    Package recvPkg;
    if (!RecvPackage(recvPkg)) {
      std::this_thread::sleep_for(10ms);
      continue;
    }
    if (recvPkg.m_nType != QUDP_ACK && recvPkg.m_nType != QUDP_SYN
      && recvPkg.m_nType != QUDP_FIN) {
      //�յ������İ���У�飬�ɹ�����ȷ�ϰ�
      int check = PackCheckHash{}(recvPkg.m_buf, recvPkg.m_nSize);
      if (recvPkg.m_nCheck == check) { //У��ʧ������
        Package pkgAck(QUDP_ACK, m_sockChannel.m_nConvId, recvPkg.m_nSeq);
        /*LOG(Log("������յ� %s ����ConvId: %d, m_nSeq = %d��m_nSize = %d",
          GetPacketText(recvPkg.m_nType).c_str(), recvPkg.m_nConvId,
          recvPkg.m_nSeq, recvPkg.m_nSize));*/

        SendPackage(pkgAck, m_channelMap[recvPkg.m_nConvId]);
      }
      else {
        LOG(Log("�����У��ʧ������ ConvId: %d", recvPkg.m_nConvId));
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
        MySQLResult result = std::get<0>(tuple); //��ȡ��1�������
        auto conn = std::get<1>(tuple);
        std::string msg; //���ص���Ϣ
        if (result.m_nAffectedRows == 0 && !result.m_error.empty()) {
          msg = std::format("ʧ��: {} ", result.m_error);
          Send(S2C_INSERT, (char*)msg.c_str(), msg.size(), channelPtr);
          break;
        }
        msg = "�ɹ�";
        Send(S2C_INSERT, (char*)msg.c_str(), msg.size(), channelPtr);
        break;
      }
      case C2S_SELECT: {
        auto& channelPtr = m_channelMap[recvPkg.m_nConvId];
        std::string sql;
        sql.assign(recvPkg.m_buf, recvPkg.m_nSize);

        TupleType tuple = GetQueryResult(sql);
        
        //SendPacketFunc(tuple, channelPtr);
        
        //�����ţ����벻ͨ�� &(QUDPServer::SendPacketFunc)
        //auto task = std::bind(&(QUDPServer::SendPacketFunc), this, tuple, channelPtr);
        //������һ��û�����ţ��ܹ�����ͨ��
        auto task = std::bind(&QUDPServer::SendPacketFunc, this, tuple, channelPtr);
        m_threadPool.SubmitTask(task);
     

        break;

      }
      case C2S_SHOW_TABLE: {
        auto& channelPtr = m_channelMap[recvPkg.m_nConvId];
        DBOperatePacket packet = *((DBOperatePacket*)recvPkg.m_buf);
        //ͬʱ��ѯ�������
    /*    shared_ptr<MySQLConnector> conn1 = m_pConnPool->GetConnection();
        mysql_set_server_option(conn1.get()->GetMySQLConn(), MYSQL_OPTION_MULTI_STATEMENTS_ON);*/
        //GetQueryResult(std::format("USE {};", packet.m_szDbName));
        string sql = std::format("  SHOW TABLES;");

        auto tuple = GetQueryResult(sql);
        MySQLResult result = std::get<0>(tuple); //��ȡ��1�������
        auto conn = std::get<1>(tuple); //��ȡ��2������ֵ
        //result = conn->GetMySQLResult(); //��ȡ��2�������

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
        //��ȡ����ִ�н��
        MySQLResult result = std::get<0>(GetQueryResult(sql)); //��ȡ��1�������
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
        //���ͻؿͻ���
        //Send(S2C_SHOW_DB, (char*)vctDb.data(), vctDb.size(), channelSockPtr);

        break;
      }
      case QUDP_ACK: //�յ�ȷ�ϰ�
      {
        std::lock_guard lock(m_mtxMap);
        auto& channelPtr = m_channelMap[recvPkg.m_nConvId];
        std::lock_guard lock2(channelPtr->m_mtxSend);
        channelPtr->m_mapSend.erase(recvPkg.m_nSeq);
        //LOG(std::format("������յ�ȷ�ϰ���{}\n", recvPkg.m_nSeq));
        //printf("������յ�ȷ�ϰ���%d\n", recvPkg.m_nSeq);
        break;
      }
      case QUDP_PSH: //�յ����Ͱ������շ�����ͬʱ�ǿͻ��˺ͷ����
      {

        //�Ž���Ӧ�ͻ��˵��հ�������
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
      case QUDP_SYN: //�յ����Ӱ�
      {
        
        LOG(std::format("������յ����Ӱ� ConvId: {}", recvPkg.m_nConvId));
        //Ȼ�󷵻�ȷ������
        SendPackage(Package(QUDP_SYN, m_sockChannel.m_nConvId, 0),
          m_channelMap[recvPkg.m_nConvId]);
        break;
      }
      case QUDP_SYNACK: //�յ�����ȷ�ϰ�
      {
        LOG(std::format("������յ�����ȷ�ϰ����Ự������ConvId: {}", recvPkg.m_nConvId));
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
  //����

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
    //������������󻺴�������
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
      //�Ž���������map��
      PackageInfo pkgInfo = PackageInfo{ now, package };
      m_mapSend[m_nNextSendSeq] = pkgInfo;
      m_nNextSendSeq++; //���+1
    }
    //����
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
  //����������ģʽ
  ioctlsocket(m_sock, FIONBIO, &bEnable);
  m_bWorking = true;
  thread(&MyQUDP::WorkingThread, this).detach();
  LOG(Log("�����ɹ�"));
}

void MyQUDP::WorkingThread() {
  while (m_bWorking) {
    { //��ʱ�ط�
      std::lock_guard lock(m_mtxSend);
      for (auto& pair : m_mapSend) {
        auto now = chrono::system_clock::now();
        auto duration = chrono::duration_cast<chrono::milliseconds>(now - pair.second.m_sendTime);
        if (duration.count() > 1000) { //��ʱ�����ʱ��ͨ���Ǽ���ó�
          SendPackage(pair.second.m_packageSend);
          LOG(Log("�յ����Ͱ���m_nSeq = %d��m_nSize = ",
            pair.second.m_packageSend.m_nSeq,
            pair.second.m_packageSend.m_nSize));
        }
      }
    }

    //�������ݰ�
    Package recvPkg;
    if (!RecvPackage(recvPkg)) {
      continue;
    }

    switch (recvPkg.m_nType) {

      case QUDP_ACK: //�յ�ȷ�ϰ�
      {
        std::lock_guard lock(m_mtxSend);
        m_mapSend.erase(recvPkg.m_nSeq);
        LOG(Log("�յ�ȷ�ϰ�"));
        break;
      }
      case QUDP_PSH: //�յ����Ͱ������շ�����ͬʱ�ǿͻ��˺ͷ����
      {
        //�յ������İ���У�飬�ɹ�����ȷ�ϰ�
        int check = PackCheckHash{}(recvPkg.m_buf, recvPkg.m_nSize);
        if (recvPkg.m_nCheck == check) {
          Package pkgAck(QUDP_ACK, recvPkg.m_nSeq);
          SendPackage(pkgAck);
          LOG(Log("�յ����Ͱ���m_nSeq = %d��m_nSize = %d", recvPkg.m_nSeq, recvPkg.m_nSize));
        }
        else { //У��ʧ������
          LOG(Log("У��ʧ������"));
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