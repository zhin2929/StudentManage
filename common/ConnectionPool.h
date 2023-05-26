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

// 连接池
class ConnectionPool {
public:
  //获取连接池对象
  static ConnectionPool* GetConnectionPool();
  // 给外部提供接口，从连接池中获取一个可用连接
  std::shared_ptr<MySQLConnector> GetConnection();

private:
  //私有，单例
  ConnectionPool();

  //从文件中获取配置信息
  bool LoadConfigFromFile();

  //产生一个连接
  void MakeConnection();

  //扫描超过最大空闲时间的任务，回收连接。
  void ScanConnection();

  std::string m_szIp; //连接IP
  unsigned short m_nPort; //连接端口
  std::string m_szUser; //连接用户
  std::string m_szPassword; //密码
  std::string m_szDbname; //数据库名

  int m_nConnInitSize; //连接池初始数量 10
  int m_nConnMaxSize; //连接池最大数量 1000，一个线程池获取一个连接

  int m_nMaxIdleTime; //最大空闲时间，单位秒
  int m_nConnTimeout; //连接池获取连接超时时间

  std::queue<MySQLConnector*> m_connQue; //连接队列
  std::mutex m_queueMtx; //队列同步互斥
  std::atomic_uint m_nConnCount; //连接数量
  std::condition_variable m_cvConn; //条件变量，用于生产线程和连接消费线程的通信
};

