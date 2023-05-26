#pragma once
#include <memory>
#include "MySQLConnector.h"
#include <string>
#include <queue>
#include <mutex>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <functional>

// ���ӳ�
class ConnectionPool {
public:
  //��ȡ���ӳض���
  static ConnectionPool* GetConnectionPool();
  // ���ⲿ�ṩ�ӿڣ������ӳ��л�ȡһ����������
  std::shared_ptr<MySQLConnector> GetConnection();

private:
  //˽�У�����
  ConnectionPool();

  //���ļ��л�ȡ������Ϣ
  bool LoadConfigFromFile();

  //����һ������
  void MakeConnection();

  //ɨ�賬��������ʱ������񣬻������ӡ�
  void ScanConnection();

  std::string m_szIp; //����IP
  unsigned short m_nPort; //���Ӷ˿�
  std::string m_szUser; //�����û�
  std::string m_szPassword; //����
  std::string m_szDbname; //���ݿ���

  int m_nConnInitSize; //���ӳس�ʼ���� 10
  int m_nConnMaxSize; //���ӳ�������� 1000��һ���̳߳ػ�ȡһ������

  int m_nMaxIdleTime; //������ʱ�䣬��λ��
  int m_nConnTimeout; //���ӳػ�ȡ���ӳ�ʱʱ��

  std::queue<MySQLConnector*> m_connQue; //���Ӷ���
  std::mutex m_queueMtx; //����ͬ������
  std::atomic_uint m_nConnCount; //��������
  std::condition_variable m_cvConn; //�������������������̺߳����������̵߳�ͨ��
};

