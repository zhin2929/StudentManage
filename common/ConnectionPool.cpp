#include "ConnectionPool.h"
using namespace std;

ConnectionPool* ConnectionPool::GetConnectionPool() {
  static ConnectionPool pool;
  return &pool;
}

//�������ļ��л�ȡ����
bool ConnectionPool::LoadConfigFromFile() {

  char szPath[MAX_PATH]{ 0 };
  GetCurrentDirectory(MAX_PATH, szPath);
  string strPath = szPath;
  strPath += "\\mysql_config.ini"; //�����ļ���
  char szString[MAXBYTE]{};
  GetPrivateProfileString("������Ϣ", "ip", "�Ҳ�������",
    szString, sizeof(szString), strPath.c_str());
  if (szString == string("�Ҳ�������")) {
    printf("�Ҳ�������");
    return false;
  }

  m_szIp = szString;
  m_nPort = GetPrivateProfileInt("������Ϣ", "port", 3306, strPath.c_str());
  
  GetPrivateProfileString("������Ϣ", "user", "�Ҳ�������",
    szString, sizeof(szString), strPath.c_str());
  m_szUser = szString;

  m_szPassword = GetPrivateProfileString("������Ϣ", "password", "�Ҳ�������",
    szString, sizeof(szString), strPath.c_str());
  m_szPassword = szString;

  m_szDbname = GetPrivateProfileString("������Ϣ", "dbname", "�Ҳ�������",
    szString, sizeof(szString), strPath.c_str());
  m_szDbname = szString;

  m_nConnInitSize = GetPrivateProfileInt("������Ϣ", "ConnInitSize", 10, strPath.c_str());
  m_nConnMaxSize = GetPrivateProfileInt("������Ϣ", "ConnMaxSize", 1000, strPath.c_str());
  m_nMaxIdleTime = GetPrivateProfileInt("������Ϣ", "MaxIdleTime", 60, strPath.c_str());
  m_nConnTimeout = GetPrivateProfileInt("������Ϣ", "ConnTimeout", 30, strPath.c_str());

  return true;
}

// ���ӳصĹ���
ConnectionPool::ConnectionPool() {
  // ����������
  if (!LoadConfigFromFile()) {
    return;
  }

  // ������ʼ����������
  for (int i = 0; i < m_nConnInitSize; ++i) {
    MySQLConnector* conn = new MySQLConnector();
    conn->Connect(m_szIp, m_nPort, m_szUser, m_szPassword, m_szDbname);
    conn->RefreshAliveTime(); // ˢ�¿�ʼ���е���ʼʱ��
    m_connQue.push(conn);
    m_nConnCount++;
  }

  // ����һ���µ��̣߳���Ϊ���ӵ������� 
  std::thread produce(std::bind(&ConnectionPool::MakeConnection, this));
  produce.detach();

  // ����һ���µĶ�ʱ�̣߳�ɨ�賬��maxIdleTimeʱ��Ŀ������ӣ����ж��ڵ����ӻ���
  std::thread scanner(std::bind(&ConnectionPool::ScanConnection, this));
  scanner.detach();
}



std::shared_ptr<MySQLConnector> ConnectionPool::GetConnection() {
  unique_lock<mutex> lock(m_queueMtx);
  while (m_connQue.empty()) {
    // �����Ӷ���Ϊ�գ���˯�� m_nConnTimeout ʱ�䣬
    cv_status status = m_cvConn.wait_for(lock, chrono::milliseconds(m_nConnTimeout));
    if (status == cv_status::timeout) {
      if (m_connQue.empty()) {
        printf("��ȡ����ʧ�ܣ����ӳ�ʱ......");
        return nullptr;
      }
    }
  }

  /*
  �Զ���shared_ptr���ͷ���Դ�ķ�ʽ���� conn ֱ�ӹ黹��queue����
  */
  shared_ptr<MySQLConnector> spConn(m_connQue.front(),
    [&](MySQLConnector* pConn) {
    // �������ڷ�����Ӧ���߳��е��õģ�����һ��Ҫ���Ƕ��е��̰߳�ȫ����
    unique_lock<mutex> lock(m_queueMtx);
    pConn->RefreshAliveTime(); // ˢ��һ�¿�ʼ���е���ʼʱ��
    m_connQue.push(pConn);
  });

  m_connQue.pop();
  m_cvConn.notify_all();  // �����������Ժ�֪ͨ�������̼߳�飬�������Ϊ�գ�����������

  return spConn;
}


void ConnectionPool::MakeConnection() {
  while (true) {
    unique_lock<mutex> lock(m_queueMtx);
    while (!m_connQue.empty()) {
      // ���Ӷ��в��գ��˴������߳̽���ȴ�״̬�����п�����δ��������������Ͳ���һ������
      m_cvConn.wait(lock); 
    }

    // ��������û�е������ޣ����������µ�����
    if (m_nConnCount < m_nConnMaxSize) {
      MySQLConnector* pConn = new MySQLConnector();
      pConn->Connect(m_szIp, m_nPort, m_szUser, m_szPassword, m_szDbname);
      pConn->RefreshAliveTime(); // ˢ��һ�¿�ʼ���е���ʼʱ��
      m_connQue.push(pConn);
      m_nConnCount++;
    }

    // ֪ͨ�������̣߳�������������
    m_cvConn.notify_all();
  }
}

void ConnectionPool::ScanConnection() {
  while (true) {
    // ͨ��sleepģ�ⶨʱЧ�����ȴ�60��ɨ��һ�ο������ӡ�
    this_thread::sleep_for(chrono::seconds(m_nMaxIdleTime));

    // ɨ���������У��ͷŶ��������
    unique_lock<mutex> lock(m_queueMtx);
    while (m_nConnCount > m_nConnInitSize) {
      MySQLConnector* pConn = m_connQue.front();
      if (pConn->GetAliveeTime() >= (m_nMaxIdleTime * 1000)) {
        m_connQue.pop();
        m_nConnCount--;
        delete pConn; // ���� ~MySQLConnector() �ͷ�����
      } else {
        break; // ��ͷ������û�г��� m_nMaxIdleTime����������Ҳ�϶�û��
      }
    }
  }
}
