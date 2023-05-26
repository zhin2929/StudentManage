#include "Package.h"
#include <string>

using enum QUDP_TYPE;

std::string GetPacketText(QUDP_TYPE type)
{
  switch (type)
  {
    case QUDP_TYPE::QUDP_UNKNOWN_TYPE:
      return "δ֪����";
      break;
    case QUDP_TYPE::QUDP_ACK:
      return "ȷ�ϰ�";
      break;
    case QUDP_TYPE::QUDP_PSH:
      return "���Ͱ�";
      break;
    case QUDP_TYPE::QUDP_SYN:
      return "���Ӱ�";
      break;
    case QUDP_TYPE::QUDP_FIN:
      return "��ֹ��";
      break;
    case QUDP_TYPE::C2S_INSERT:
      return "�����";
      break;
    case QUDP_TYPE::C2S_DELETE:
      return "ɾ����";
      break;
    case QUDP_TYPE::C2S_UPDATE:
      return "���°�";
      break;
    case QUDP_TYPE::C2S_SELECT:
      return "��ѯ��";
      break;
    case QUDP_TYPE::C2S_SHOW_DB:
      return "��ʾ���ݿ��";
      break;
    case QUDP_TYPE::C2S_SHOW_TABLE:
      return "��ʾ���б��";
      break;
    case QUDP_TYPE::S2C_INSERT:
      return "����˷��ز����";
      break;
    case QUDP_TYPE::S2C_DELETE:
      return "����˷���ɾ����";
      break;
    case QUDP_TYPE::S2C_UPDATE:
      return "����˷��ظ��°�";
      break;
    case QUDP_TYPE::S2C_SELECT:
      return "����˷��ز�ѯ��";
      break;
    case QUDP_TYPE::S2C_SHOW_DB:
      return "����˷������ݿ��";
      break;
    case QUDP_TYPE::S2C_SHOW_TABLE:
      return "����˷��ر�����";
      break;
    default:
      break;
  }
  return std::string();
}
