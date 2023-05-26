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
  std::map<int, PackageInfo> m_mapSend; //��������

  std::map<int, Package> m_mapRecv; //�հ�����
  std::map<int, int> m_mapRecvSeq; //�հ�����������յ����ٰ�

  int m_nNextSendSeq;
  int m_nNextHandleSeq;
  SOCKET m_sock = INVALID_SOCKET;
  sockaddr_in m_destAddr; //����Ŀ���׽��ֵ�ַ
  int m_nDestPort;
  uint32_t m_nConvId; //�ỰID����ʶÿһ������

  std::recursive_mutex m_mtxVct;
  std::vector<char> m_vctBuf;
};