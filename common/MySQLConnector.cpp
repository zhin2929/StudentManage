
#include "MySQLConnector.h"
#include "Log.hpp"

MySQLConnector::MySQLConnector() : m_conn(nullptr) {
  m_conn = mysql_init(nullptr);
}

MySQLConnector::~MySQLConnector() {
  if (m_conn) {
    mysql_close(m_conn);
  }
}

bool MySQLConnector::Connect(std::string ip, unsigned short port, 
  std::string user, std::string password, std::string dbname) {
  MYSQL* p = mysql_real_connect(m_conn,
    ip.c_str(), user.c_str(), password.c_str(), dbname.c_str(), port,
    nullptr, 0); //CLIENT_MULTI_STATEMENTS支持多语句查询

  if (!p) {
    LOG(Log("连接失败：%s\n", mysql_error(m_conn)));
  }
  SetCharacterSet("gbk");
  return p != nullptr;
}

/*
如果是多条语句查询后，
由调用方调用 GetMySQLResult 获取每一个结果集，有多少个结果集就要获取多少次
*/
MySQLResult MySQLConnector::Query(std::string& sql) {
  printf("执行任务：sql=%s\n", sql.c_str());
  int nRet = mysql_real_query(m_conn, sql.c_str(), sql.size());
  if (nRet != 0) {
    std::string err = mysql_error(m_conn);
    LOG(Log("Query %s 错误：%s\n", sql.c_str(), err.c_str()));
    return MySQLResult(nullptr, 0, err);
  }
  //在 mysql_store_result 之后，如果没有要检索的行，或者检索出错，mysql_fetch_row()返回NULL;
  MYSQL_RES* res = mysql_store_result(m_conn);
  mysql_next_result(m_conn);
  return MySQLResult(res, res ? mysql_num_rows(res) : 0);
}

MySQLResult MySQLConnector::GetMySQLResult() {
  MYSQL_RES* res = mysql_store_result(m_conn);
  mysql_next_result(m_conn);
  return MySQLResult(res, res ? mysql_num_rows(res) : 0);
}

MySQLResult MySQLConnector::Update(std::string& sql) {
  printf("执行任务：sql=%s\n", sql.c_str());
  if (mysql_real_query(m_conn, sql.c_str(), sql.size()) != 0) {
    std::string err = mysql_error(m_conn);
    LOG(Log("Update 错误：%s\n", err.c_str()));
    return MySQLResult(nullptr, 0, err);
  }
  return MySQLResult(nullptr, mysql_affected_rows(m_conn), "");
}

MySQLResult MySQLConnector::Insert(std::string& sql) {
  return Update(sql);
}

MySQLResult MySQLConnector::Delete(std::string& sql) {
  return Update(sql);
}

std::string MySQLConnector::GetLastQueryTable()
{
  const char* result = (mysql_info(m_conn));
  return result;
}

bool MySQLConnector::SetCharacterSet(std::string set) {
  int nRet = mysql_set_character_set(m_conn, set.c_str());
  return nRet == 0; //返回0成功，非0失败
}

size_t MySQLConnector::GetLastInsertId() {
  return mysql_insert_id(m_conn);
}
