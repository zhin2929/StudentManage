#include "Package.h"
#include <string>

using enum QUDP_TYPE;

std::string GetPacketText(QUDP_TYPE type)
{
  switch (type)
  {
    case QUDP_TYPE::QUDP_UNKNOWN_TYPE:
      return "未知包名";
      break;
    case QUDP_TYPE::QUDP_ACK:
      return "确认包";
      break;
    case QUDP_TYPE::QUDP_PSH:
      return "发送包";
      break;
    case QUDP_TYPE::QUDP_SYN:
      return "连接包";
      break;
    case QUDP_TYPE::QUDP_FIN:
      return "终止包";
      break;
    case QUDP_TYPE::C2S_INSERT:
      return "插入包";
      break;
    case QUDP_TYPE::C2S_DELETE:
      return "删除包";
      break;
    case QUDP_TYPE::C2S_UPDATE:
      return "更新包";
      break;
    case QUDP_TYPE::C2S_SELECT:
      return "查询包";
      break;
    case QUDP_TYPE::C2S_SHOW_DB:
      return "显示数据库包";
      break;
    case QUDP_TYPE::C2S_SHOW_TABLE:
      return "显示所有表包";
      break;
    case QUDP_TYPE::S2C_INSERT:
      return "服务端返回插入包";
      break;
    case QUDP_TYPE::S2C_DELETE:
      return "服务端返回删除包";
      break;
    case QUDP_TYPE::S2C_UPDATE:
      return "服务端返回更新包";
      break;
    case QUDP_TYPE::S2C_SELECT:
      return "服务端返回查询包";
      break;
    case QUDP_TYPE::S2C_SHOW_DB:
      return "服务端返回数据库包";
      break;
    case QUDP_TYPE::S2C_SHOW_TABLE:
      return "服务端返回表名包";
      break;
    default:
      break;
  }
  return std::string();
}
