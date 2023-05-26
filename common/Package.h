#pragma once
#include <thread>
#include <mutex>
#include <map>
#include "Log.hpp"

constexpr int PACKAGE_SIZE = 30; //Package 大小
constexpr int QUDP_MTU = 1480 - 8; //减去UDP 8 字节头部 = 可用字节
//减去 15 Package 大小 = 数据可用字节
constexpr int QUDP_MSS = (QUDP_MTU - PACKAGE_SIZE);
constexpr int QUDP_CACHE = 1000;



//指定枚举类型大小
enum class QUDP_TYPE : uint32_t {
  QUDP_UNKNOWN_TYPE = 0x00,
  QUDP_ACK = 0x11, //确认包
  QUDP_PSH = 0x22, //发送包
  QUDP_SYN = 0x33, //连接包
  QUDP_SYNACK = 0x34, //客户端连接确认包
  QUDP_FIN = 0x44, //终止包
  C2S_INSERT = 0x111, //增  client to server
  C2S_DELETE = 0x222, //删
  C2S_UPDATE = 0x333, //改
  C2S_SELECT = 0x444, //查
  C2S_SHOW_DB = 0x555,
  C2S_SHOW_TABLE = 0x666,
  S2C_INSERT = 0x112, //增  server to client 
  S2C_DELETE = 0x223, //删
  S2C_UPDATE = 0x334, //改
  S2C_SELECT = 0x445, //查
  S2C_SHOW_DB = 0x556,
  S2C_SHOW_TABLE = 0x667,
};

using enum QUDP_TYPE;

std::string GetPacketText(QUDP_TYPE type);


//校验函数
struct PackCheckHash {
  size_t operator()(const char* buf, int nSize) {
    size_t h1 = 0;
    for (int i = 0; i < nSize; ++i) {
      h1 = h1 ^ ((std::hash<char>{}(buf[i])) << 1);
    }
    return h1;
  }
};

#pragma pack(push)
#pragma pack(1)
struct Package {
  Package()
    : m_nType(QUDP_UNKNOWN_TYPE)
    , m_nConvId(0)
    , m_nSeq(0)
    , m_nCheck(0)
    , m_nSize(0)
  {}

  Package(QUDP_TYPE nType, uint32_t nConvId, int nSeq)
    : m_nType(nType)
    , m_nConvId(nConvId)
    , m_nSeq(nSeq)
    , m_nCheck(0)
    , m_nSize(0)
  {}

  Package(QUDP_TYPE nType, uint32_t nConvId, uint32_t nSeq,
    const char* buf, uint32_t nSize)
    : m_nType(nType)
    , m_nConvId(nConvId)
    , m_nSeq(nSeq)
    , m_nSize(nSize)
    , m_nTotalSize(nSize)
    , m_nCheck(PackCheckHash{}(buf, nSize)) {
    std::memcpy(m_buf, buf, nSize);
  }

  Package(QUDP_TYPE nType, uint32_t nConvId, uint32_t nSeq,
    const char* buf, uint32_t nSize, uint32_t nTotalSize)
    : m_nType(nType)
    , m_nConvId(nConvId)
    , m_nSeq(nSeq)
    , m_nSize(nSize)
    , m_nTotalSize(nTotalSize)
    , m_nCheck(PackCheckHash{}(buf, nSize)) {
    std::memcpy(m_buf, buf, nSize);
  }

  uint32_t m_nConvId; //会话ID，标识每一个连接。
  QUDP_TYPE m_nType; //包类型 uint32_t
  uint32_t m_nSize; //数据长度
  uint32_t m_nTotalSize; //数据总长度
  uint32_t m_nSeq; //包序号
  int m_nCheck; //校验
  char m_buf[QUDP_MSS]; //数据字段
};

struct PackageInfo {
  using time_point = std::chrono::system_clock::time_point;
  time_point m_sendTime; //8 字节
  Package m_packageSend;
  bool m_bIsAck = false; //是否确认收到。
};

//客户端使用
struct DBOperatePacket {
  DBOperatePacket() {}

  char m_szDbName[100];
  char m_szTableName[100];
  char m_where[200];
};



struct Packet2 {
  Packet2() : m_nSize(0),
    m_nType(QUDP_UNKNOWN_TYPE) {
  };
  Packet2(size_t nLen, QUDP_TYPE packetType)
    : m_nSize(nLen), m_nType(packetType) {
  }
  Packet2(QUDP_TYPE nType, uint32_t nConvId, int nSeq)
    : m_nType(nType)
    , m_nConvId(nConvId)
    , m_nSeq(nSeq)
    , m_nCheck(0) {
    m_nSize = sizeof(m_nConvId) + sizeof(m_nType);
    m_nSize += sizeof(m_nSize) ;
    m_nSize += sizeof(m_nSeq) + sizeof(m_nCheck);
  }
  uint32_t m_nConvId; //会话ID，标识每一个连接。
  QUDP_TYPE m_nType; //包类型
  size_t m_nSize; //数据总长度
  size_t m_nSeq; //包序号
  int m_nCheck; //校验
};


#if 1
struct Packet {
  Packet() : m_nSize(0),
    m_nType(QUDP_UNKNOWN_TYPE) {
  };
  Packet(size_t nLen, QUDP_TYPE packetType)
    : m_nSize(nLen), m_nType(packetType) {
  }
  Packet(QUDP_TYPE nType, uint32_t nConvId, int nSeq)
    : m_nType(nType)
    , m_nConvId(nConvId)
    , m_nSeq(nSeq)
    , m_nCount(1) //子类自己设置
    , m_nCheck(0) {
    m_nSize = sizeof(m_nConvId) + sizeof(m_nType);
    m_nSize += sizeof(m_nSize) + sizeof(m_nCount);
    m_nSize += sizeof(m_nSeq) + sizeof(m_nCheck);
  }
  uint32_t m_nConvId; //会话ID，标识每一个连接。
  QUDP_TYPE m_nType; //包类型
  size_t m_nSize; //数据总长度
  size_t m_nCount; //数据总长度
  size_t m_nSeq; //包序号
  int m_nCheck; //校验
};
#endif // 0


struct PacketInfo {
  using time_point = std::chrono::system_clock::time_point;
  time_point m_sendTime; //8 字节
  Packet m_packageSend;
  bool m_bIsAck = false; //是否确认收到。
};

struct ClassPacket : Packet {
  int m_nId;
  int m_nClassId;
  char m_nClassName[255];
};

struct CoursePacket : Packet {
  int m_nId;
  int m_nCourseId;
  char m_nCourseName[255];
};

struct StudentPacket : Packet {
  int m_nId;
  int m_nStudentId;
  int m_nClassId;
  int m_nAge;
  char m_szStudentName[255];
};

struct ScorePacket : Packet {
  int m_nId;
  int m_nStudentId;
  int m_nClassId;
  int m_nScore;
};

struct DBPacket : Packet {
  DBPacket() {}
  DBPacket(size_t nCount, size_t nDataLen, char* pBuf) 
  {
    if (nDataLen > 50) {
      LOG(std::string("长度过长"));
      return;
    }
    m_nCount = nCount;
    m_nSize = sizeof(Packet) + (nCount * sizeof(m_data));
    std::memcpy(m_data, pBuf, nDataLen);
  }
  char m_data[50];
};

struct TablePacket2 {
  TablePacket2() {}
  TablePacket2(uint32_t nDataLen, char* pBuf) {
    m_nLen = sizeof(m_nLen) + nDataLen;
    std::memcpy(m_data, pBuf, nDataLen);
  }
  uint32_t m_nLen;
  char m_data[0];
};

struct TablePacket : Packet {
  TablePacket() {}
  TablePacket(size_t nCount, size_t nDataLen, char* pBuf) {
    if (nDataLen > 100) {
      LOG(std::format("长度过长 {}", nDataLen));
      return;
    }
    m_nCount = nCount;
    m_nSize = sizeof(Packet) + (nCount * sizeof(m_data));
    std::memcpy(m_data, pBuf, nDataLen);
  }
  char m_data[100];
};





#pragma pack(pop)
