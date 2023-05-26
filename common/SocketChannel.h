#pragma once
#include <map>
#include <vector>
#include <memory>

struct SocketChannel {
  SocketChannel() : m_nNextSendSeq(0)
    , m_nNextHandleSeq(0)
    , m_nDestPort(0)
    , m_nConvId(0)
  {}

  SocketChannel(const SocketChannel& rhs)
  {
    m_sock = rhs.m_sock;
    m_destAddr = rhs.m_destAddr;
    m_nNextSendSeq = rhs.m_nNextSendSeq;
    m_nNextHandleSeq = rhs.m_nNextHandleSeq;
    m_nDestPort = rhs.m_nDestPort;
  }

  std::recursive_mutex m_mtxSend;
  std::map<int, PackageInfo> m_mapSend; //发包容器

  std::map<int, Package> m_mapRecv; //收包容器
  std::map<int, int> m_mapRecvSeq; //收包容器，标记收到多少包

  int m_nNextSendSeq;
  int m_nNextHandleSeq;
  SOCKET m_sock = INVALID_SOCKET;
  sockaddr_in m_destAddr; //连接目标套接字地址
  int m_nDestPort;
  uint32_t m_nConvId; //会话ID，标识每一个连接

  std::recursive_mutex m_mtxVct;
  std::vector<char> m_vctBuf;
};