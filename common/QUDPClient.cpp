#include "QUDPClient.h"
#include "Log.hpp"
#include <ctime>

using namespace std;
using namespace std::chrono_literals;
using namespace std::chrono;
using enum QUDP_TYPE;



bool QUDPClient::ConnectServer(short nPort, const char* szIp) {
  m_sockChannel.m_nDestPort = nPort;
  m_sockChannel.m_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (m_sockChannel.m_sock == INVALID_SOCKET) { return false; };
  m_sockChannel.m_destAddr.sin_family = AF_INET;
  m_sockChannel.m_destAddr.sin_port = htons(nPort);
  inet_pton(AF_INET, szIp, &m_sockChannel.m_destAddr.sin_addr.s_addr);
  //连接

  return true;
}

void QUDPClient::Recv(char* buf, int nLen) {
  while (m_sockChannel.m_vctBuf.size() < nLen) {
    std::this_thread::yield();
  }
  std::lock_guard lock(m_sockChannel.m_mtxVct);
  memcpy(buf, m_sockChannel.m_vctBuf.data(), nLen);
  m_sockChannel.m_vctBuf.erase(m_sockChannel.m_vctBuf.begin(), 
    m_sockChannel.m_vctBuf.begin() + nLen);
}

void QUDPClient::Send(char* buf, int nLen) {
  int nCount = nLen % QUDP_MSS == 0
    ? nLen / QUDP_MSS : nLen / QUDP_MSS + 1;

  for (size_t i = 0; i < nCount; ++i) {
    //包数量超过最大缓存则阻塞
    while (m_sockChannel.m_mapSend.size() > QUDP_CACHE) {
      std::this_thread::yield();
    }

    int nLastSize = (i == nCount - 1) ? nLen - i * QUDP_MSS : QUDP_MSS;
    Package package(QUDP_PSH, m_sockChannel.m_nConvId,
      m_sockChannel.m_nNextSendSeq, buf + i * QUDP_MSS, nLastSize, nLen);

    auto now = chrono::system_clock::now();

    {
      std::lock_guard lock(m_sockChannel.m_mtxSend);
      //放进发包容器map里
      PackageInfo pkgInfo = PackageInfo{ now, package };
      m_sockChannel.m_mapSend[m_sockChannel.m_nNextSendSeq] = pkgInfo;
      m_sockChannel.m_nNextSendSeq++; //序号+1
    }
    //发包
    SendPackage(package);
    Sleep(10);
  }
}

void QUDPClient::SendPackage(const Package& package) {
  int nRet = sendto(m_sockChannel.m_sock, (char*)&package,
    sizeof(package), 0, (sockaddr*)&m_sockChannel.m_destAddr,
    sizeof(m_sockChannel.m_destAddr));
  //LOG(Log("客户端 %d 发送包：m_nSeq = %d，m_nSize = %d", m_sockChannel.m_nConvId,
  //  package.m_nSeq, package.m_nSize));
}

bool QUDPClient::RecvPackage(Package& package) {
  int nAddrLen = sizeof(m_sockChannel.m_destAddr);
  int nRet = recvfrom(m_sockChannel.m_sock, (char*)&package,
    sizeof(package), 0, (sockaddr*)&m_sockChannel.m_destAddr, &nAddrLen);
  if (nRet == SOCKET_ERROR || nRet == 0) {
    return false;
  }
  return true;
}

void QUDPClient::StartWork(UpdateUIFunc func) {
  m_uiFunc = func;
  u_long bEnable = true;
  //开启非阻塞模式
  ioctlsocket(m_sockChannel.m_sock, FIONBIO, &bEnable);
  m_bWorking = true;
  thread(&QUDPClient::WorkingThread, this).detach();

  // 生成32位唯一ID
  srand(time(NULL));
  unsigned int convId = rand() << 16 | rand() << 1 | rand() % 2;
  m_sockChannel.m_nConvId = convId;

  LOG(Log("客户端 %d 启动成功", convId));
}

void QUDPClient::WorkingThread() {
  while (m_bWorking) {
    { //超时重发
      std::lock_guard lock(m_sockChannel.m_mtxSend);
      for (auto& pair : m_sockChannel.m_mapSend) {
        auto now = system_clock::now();
        auto duration = duration_cast<milliseconds>(now - pair.second.m_sendTime);
        if (duration.count() > 1000) { //超时，这个时间通常是计算得出
          SendPackage(pair.second.m_packageSend);
          pair.second.m_sendTime = system_clock::now();
          LOG(Log("客户端 %d 超时重发包：m_nSeq = %d，m_nSize = %d", 
            m_sockChannel.m_nConvId,
            pair.second.m_packageSend.m_nSeq,
            pair.second.m_packageSend.m_nSize));
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
        SendPackage(pkgAck);
        LOG(Log("客户端收到 %s 包：ConvId: %d, m_nSeq = %d，m_nSize = %d",
          GetPacketText(recvPkg.m_nType).c_str(), m_sockChannel.m_nConvId,
          recvPkg.m_nSeq, recvPkg.m_nSize));
      }
      else {
        LOG(Log("客户端 %d 校验失败则丢弃", m_sockChannel.m_nConvId));
        continue;
      }
    }
    

    switch (recvPkg.m_nType) {
      case S2C_INSERT:
      case S2C_SELECT: {
        m_sockChannel.m_mapRecv[recvPkg.m_nSeq] = recvPkg;
        while (m_sockChannel.m_mapRecv.find(m_sockChannel.m_nNextHandleSeq)
          != m_sockChannel.m_mapRecv.end()) {
          m_sockChannel.m_nNextHandleSeq++;
          m_uiFunc(recvPkg.m_nType, recvPkg.m_buf);
        }
        break;
      }
      case S2C_SHOW_TABLE: {
        m_sockChannel.m_mapRecv[recvPkg.m_nSeq] = recvPkg;
        int nLen = recvPkg.m_nTotalSize;
        while (m_sockChannel.m_mapRecv.find(m_sockChannel.m_nNextHandleSeq)
          != m_sockChannel.m_mapRecv.end()) {
          auto& pkg = m_sockChannel.m_mapRecv[m_sockChannel.m_nNextHandleSeq];
          m_sockChannel.m_mtxVct.lock();
          m_sockChannel.m_vctBuf.insert(m_sockChannel.m_vctBuf.end(),
            pkg.m_buf, pkg.m_buf + pkg.m_nSize);
          m_sockChannel.m_mtxVct.unlock();
          m_sockChannel.m_nNextHandleSeq++;
          m_uiFunc(recvPkg.m_nType, recvPkg.m_buf);
        }

        //m_uiFunc(recvPkg.m_nType, nLen, m_sockChannel.m_vctBuf);
        break;
      }
      case S2C_SHOW_DB: {
        m_sockChannel.m_mapRecv[recvPkg.m_nSeq] = recvPkg;
        while (m_sockChannel.m_mapRecv.find(m_sockChannel.m_nNextHandleSeq)
          != m_sockChannel.m_mapRecv.end()) {
          auto& pkg = m_sockChannel.m_mapRecv[m_sockChannel.m_nNextHandleSeq];
          m_sockChannel.m_mtxVct.lock();
          m_sockChannel.m_vctBuf.insert(m_sockChannel.m_vctBuf.end(),
            pkg.m_buf, pkg.m_buf + pkg.m_nSize);
          m_sockChannel.m_mtxVct.unlock();
          m_sockChannel.m_nNextHandleSeq++;
          m_uiFunc(recvPkg.m_nType, recvPkg.m_buf);
        }
        break;
      }
      case QUDP_ACK: //收到确认包
      {
        std::lock_guard lock(m_sockChannel.m_mtxSend);
        m_sockChannel.m_mapSend.erase(recvPkg.m_nSeq);
        LOG(Log("客户端收到确认包: %d", recvPkg.m_nSeq));
        break;
      }
      case QUDP_PSH: //收到发送包，接收方可以同时是客户端和服务端
      {
        m_sockChannel.m_mapRecv[recvPkg.m_nSeq] = recvPkg;
        while (m_sockChannel.m_mapRecv.find(m_sockChannel.m_nNextHandleSeq)
          != m_sockChannel.m_mapRecv.end()) {
          auto& pkg = m_sockChannel.m_mapRecv[m_sockChannel.m_nNextHandleSeq];
          m_sockChannel.m_mtxVct.lock();
          m_sockChannel.m_vctBuf.insert(m_sockChannel.m_vctBuf.end(), 
            pkg.m_buf, pkg.m_buf + pkg.m_nSize);
          m_sockChannel.m_mtxVct.unlock();
          m_sockChannel.m_nNextHandleSeq++;
        }
        break;
      }
      case QUDP_SYN: //收到连接包
      {
        
        //客户端只有一个服务端
        if (m_nServerConvId == 0) {
          m_nServerConvId = recvPkg.m_nConvId;
        }
        SendPackage(Package (QUDP_SYNACK, m_sockChannel.m_nConvId, recvPkg.m_nSeq));
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