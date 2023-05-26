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

  //����mysql_free_result������Ϊ�Զ������������
  MySQLResult(MYSQL_RES* res, int nAffectedRows)
    : m_res(res, mysql_free_result) 
    , m_nAffectedRows(nAffectedRows)
  {
    //����ʹ��reset��������ʼ��m_res��������һ��lambda�������ͷ���Դ��
    //��һ��������Ҫ�����ָ�룬�ڶ���������һ���ɵ��ö��������ͷ�ָ�롣
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

  // ˢ�����ӵ���ʼ����ʱ���
  void RefreshAliveTime() { m_nAlivetime = clock(); }
  // ���ؿ��е�ʱ��
  clock_t GetAliveeTime() const { return clock() - m_nAlivetime; }
private:
  clock_t m_nAlivetime;
  MYSQL* m_conn;

};

