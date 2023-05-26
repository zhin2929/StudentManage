
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
    nullptr, 0); //CLIENT_MULTI_STATEMENTS֧�ֶ�����ѯ

  if (!p) {
    LOG(Log("����ʧ�ܣ�%s\n", mysql_error(m_conn)));
  }
  SetCharacterSet("gbk");
  return p != nullptr;
}

/*
����Ƕ�������ѯ��
�ɵ��÷����� GetMySQLResult ��ȡÿһ����������ж��ٸ��������Ҫ��ȡ���ٴ�
*/
MySQLResult MySQLConnector::Query(std::string& sql) {
  printf("ִ������sql=%s\n", sql.c_str());
  int nRet = mysql_real_query(m_conn, sql.c_str(), sql.size());
  if (nRet != 0) {
    std::string err = mysql_error(m_conn);
    LOG(Log("Query %s ����%s\n", sql.c_str(), err.c_str()));
    return MySQLResult(nullptr, 0, err);
  }
  //�� mysql_store_result ֮�����û��Ҫ�������У����߼�������mysql_fetch_row()����NULL;
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
  printf("ִ������sql=%s\n", sql.c_str());
  if (mysql_real_query(m_conn, sql.c_str(), sql.size()) != 0) {
    std::string err = mysql_error(m_conn);
    LOG(Log("Update ����%s\n", err.c_str()));
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
  return nRet == 0; //����0�ɹ�����0ʧ��
}

size_t MySQLConnector::GetLastInsertId() {
  return mysql_insert_id(m_conn);
}
