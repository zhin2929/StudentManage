#include "ConnectionPool.h"
using namespace std;

ConnectionPool* ConnectionPool::GetConnectionPool() {
  static ConnectionPool pool;
  return &pool;
}

//从配置文件中获取配置
bool ConnectionPool::LoadConfigFromFile() {

  char szPath[MAX_PATH]{ 0 };
  GetCurrentDirectory(MAX_PATH, szPath);
  string strPath = szPath;
  strPath += "\\mysql_config.ini"; //配置文件名
  char szString[MAXBYTE]{};
  GetPrivateProfileString("配置信息", "ip", "找不到配置",
    szString, sizeof(szString), strPath.c_str());
  if (szString == string("找不到配置")) {
    printf("找不到配置");
    return false;
  }

  m_szIp = szString;
  m_nPort = GetPrivateProfileInt("配置信息", "port", 3306, strPath.c_str());
  
  GetPrivateProfileString("配置信息", "user", "找不到配置",
    szString, sizeof(szString), strPath.c_str());
  m_szUser = szString;

  m_szPassword = GetPrivateProfileString("配置信息", "password", "找不到配置",
    szString, sizeof(szString), strPath.c_str());
  m_szPassword = szString;

  m_szDbname = GetPrivateProfileString("配置信息", "dbname", "找不到配置",
    szString, sizeof(szString), strPath.c_str());
  m_szDbname = szString;

  m_nConnInitSize = GetPrivateProfileInt("配置信息", "ConnInitSize", 10, strPath.c_str());
  m_nConnMaxSize = GetPrivateProfileInt("配置信息", "ConnMaxSize", 1000, strPath.c_str());
  m_nMaxIdleTime = GetPrivateProfileInt("配置信息", "MaxIdleTime", 60, strPath.c_str());
  m_nConnTimeout = GetPrivateProfileInt("配置信息", "ConnTimeout", 30, strPath.c_str());

  return true;
}

// 连接池的构造
ConnectionPool::ConnectionPool() {
  // 加载配置项
  if (!LoadConfigFromFile()) {
    return;
  }

  // 创建初始数量的连接
  for (int i = 0; i < m_nConnInitSize; ++i) {
    MySQLConnector* conn = new MySQLConnector();
    conn->Connect(m_szIp, m_nPort, m_szUser, m_szPassword, m_szDbname);
    conn->RefreshAliveTime(); // 刷新开始空闲的起始时间
    m_connQue.push(conn);
    m_nConnCount++;
  }

  // 启动一个新的线程，作为连接的生产者 
  std::thread produce(std::bind(&ConnectionPool::MakeConnection, this));
  produce.detach();

  // 启动一个新的定时线程，扫描超过maxIdleTime时间的空闲连接，进行对于的连接回收
  std::thread scanner(std::bind(&ConnectionPool::ScanConnection, this));
  scanner.detach();
}



std::shared_ptr<MySQLConnector> ConnectionPool::GetConnection() {
  unique_lock<mutex> lock(m_queueMtx);
  while (m_connQue.empty()) {
    // 若连接队列为空，则睡眠 m_nConnTimeout 时间，
    cv_status status = m_cvConn.wait_for(lock, chrono::milliseconds(m_nConnTimeout));
    if (status == cv_status::timeout) {
      if (m_connQue.empty()) {
        printf("获取连接失败，连接超时......");
        return nullptr;
      }
    }
  }

  /*
  自定义shared_ptr的释放资源的方式，把 conn 直接归还到queue当中
  */
  shared_ptr<MySQLConnector> spConn(m_connQue.front(),
    [&](MySQLConnector* pConn) {
    // 这里是在服务器应用线程中调用的，所以一定要考虑队列的线程安全操作
    unique_lock<mutex> lock(m_queueMtx);
    pConn->RefreshAliveTime(); // 刷新一下开始空闲的起始时间
    m_connQue.push(pConn);
  });

  m_connQue.pop();
  m_cvConn.notify_all();  // 消费完连接以后，通知生产者线程检查，如果队列为空，就生产连接

  return spConn;
}


void ConnectionPool::MakeConnection() {
  while (true) {
    unique_lock<mutex> lock(m_queueMtx);
    while (!m_connQue.empty()) {
      // 连接队列不空，此处生产线程进入等待状态，队列空了且未到最大连接数，就产生一个连接
      m_cvConn.wait(lock); 
    }

    // 连接数量没有到达上限，继续创建新的连接
    if (m_nConnCount < m_nConnMaxSize) {
      MySQLConnector* pConn = new MySQLConnector();
      pConn->Connect(m_szIp, m_nPort, m_szUser, m_szPassword, m_szDbname);
      pConn->RefreshAliveTime(); // 刷新一下开始空闲的起始时间
      m_connQue.push(pConn);
      m_nConnCount++;
    }

    // 通知消费者线程，可以消费连接
    m_cvConn.notify_all();
  }
}

void ConnectionPool::ScanConnection() {
  while (true) {
    // 通过sleep模拟定时效果，等待60秒扫描一次空闲连接。
    this_thread::sleep_for(chrono::seconds(m_nMaxIdleTime));

    // 扫描整个队列，释放多余的连接
    unique_lock<mutex> lock(m_queueMtx);
    while (m_nConnCount > m_nConnInitSize) {
      MySQLConnector* pConn = m_connQue.front();
      if (pConn->GetAliveeTime() >= (m_nMaxIdleTime * 1000)) {
        m_connQue.pop();
        m_nConnCount--;
        delete pConn; // 调用 ~MySQLConnector() 释放连接
      } else {
        break; // 队头的连接没有超过 m_nMaxIdleTime，其它连接也肯定没有
      }
    }
  }
}
