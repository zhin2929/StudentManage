#pragma once
#include <mysql.h>
#include <string>
#include <time.h>
#include <memory>
#include <functional>
#pragma comment(lib, "libmysql.lib")

struct MySQLResult {
  MySQLResult() 
    : m_res(nullptr)
    , m_nAffectedRows(0)
  {}

  //传递mysql_free_result函数作为自定义的析构函数
  MySQLResult(MYSQL_RES* res, int nAffectedRows)
    : m_res(res, mysql_free_result) 
    , m_nAffectedRows(nAffectedRows)
  {
    //或者使用reset函数来初始化m_res，并传递一个lambda函数来释放资源。
    //第一个参数是要管理的指针，第二个参数是一个可调用对象，用于释放指针。
    //m_res.reset(res, [&](MYSQL_RES* res) { mysql_free_result(res); });
  }

  MySQLResult(MYSQL_RES* res, int nAffectedRows, std::string error)
    : m_res(res, mysql_free_result) 
    , m_nAffectedRows(nAffectedRows)
    , m_error(error)
  {}

  ~MySQLResult() {
  }
  std::shared_ptr<MYSQL_RES> m_res;
  int m_nAffectedRows;
  std::string m_error;
};

class MySQLConnector {
public:
  MySQLConnector();

  ~MySQLConnector();

  bool Connect(std::string ip,
    unsigned short port,
    std::string user,
    std::string password,
    std::string dbname);

  

  MySQLResult Query(std::string& sql);

  MySQLResult GetMySQLResult();

  MySQLResult Update(std::string& sql);

  MySQLResult Insert(std::string& sql);

  MySQLResult Delete(std::string& sql);

  std::string GetLastQueryTable();

  bool SetCharacterSet(std::string set);

  size_t GetLastInsertId();

  MYSQL* GetMySQLConn() {
    return m_conn;
  }

  // 刷新连接的起始空闲时间点
  void RefreshAliveTime() { m_nAlivetime = clock(); }
  // 返回空闲的时间
  clock_t GetAliveeTime() const { return clock() - m_nAlivetime; }
private:
  clock_t m_nAlivetime;
  MYSQL* m_conn;

};

