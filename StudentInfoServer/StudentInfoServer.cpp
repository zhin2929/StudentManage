#include "MySQLConnector.h"
#include "ConnectionPool.h"
#include "QUDPServer.h"

#include <string>
#include <iostream>

using namespace std::literals::chrono_literals;


using namespace std;

//初始化配置文件
void InitConfig() {

  char szPath[MAX_PATH]{ 0 };
  GetCurrentDirectory(MAX_PATH, szPath);
  string strPath = szPath;
  strPath += "\\mysql_config.ini"; //配置文件名
  char szString[MAXBYTE]{};
  GetPrivateProfileString("配置信息", "ip", "找不到配置",
    szString, sizeof(szString), strPath.c_str());
  if (szString == string("找不到配置")) {
    WritePrivateProfileString("配置信息", "ip", "127.0.0.1", strPath.c_str()); // 连接IP
    WritePrivateProfileString("配置信息", "port", "3306", strPath.c_str()); // 连接端口
    WritePrivateProfileString("配置信息", "user", "root", strPath.c_str()); // 登录用户名
    WritePrivateProfileString("配置信息", "password", "toor", strPath.c_str()); // 登录用户密码
    WritePrivateProfileString("配置信息", "dbname", "student_info", strPath.c_str()); // 数据库名
    WritePrivateProfileString("配置信息", "ConnInitSize", "10", strPath.c_str()); // 初始连接数量
    WritePrivateProfileString("配置信息", "ConnMaxSize", "1000", strPath.c_str()); // 连接最大数量
    WritePrivateProfileString("配置信息", "MaxIdleTime", "60", strPath.c_str()); // 空闲时间 60秒
    WritePrivateProfileString("配置信息", "ConnTimeout", "30", strPath.c_str()); // 连接超时 30秒
  }

}


int main()
{
  InitConfig();

  QUDPServer qudp;
  
  qudp.StartServer(9527);

  while (true) {
    std::this_thread::sleep_for(24h);
  }

  return 0;
}


