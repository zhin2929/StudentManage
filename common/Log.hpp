#pragma once
#include <windows.h>
#include <string>
#include <format>


#define LOG1(str) \
  char szOutput[0x1000]{}; \
  sprintf(szOutput,"文件：%s，行数：%d，时间：%s：%s \n", \
__FILE__, __LINE__, __TIMESTAMP__, str.c_str());	\
  printf("控制台输出：%s\n", szOutput);	\
  OutputDebugString(szOutput);

#define LOG2(str) \
  std::string strOutput = std::format("文件：{}，行数：{}，时间：{}：{} \n",      \
    __FILE__, __LINE__, __TIMESTAMP__, str.c_str());    \
  printf("控制台输出：%s\n", strOutput.c_str()); \
  OutputDebugString(strOutput.c_str());

#define LOG(str) \
  printf("%s\n", std::format("文件：{}，行数：{}，时间：{}：{} \n", __FILE__, __LINE__, __TIMESTAMP__, str.c_str()).c_str()); \
  OutputDebugString(std::format("文件：{}，行数：{}，时间：{}：{} \n", __FILE__, __LINE__, __TIMESTAMP__, str.c_str()).c_str());

#define LOG3(str) \
  OutputDebugString(std::format("ZQZQ文件：{}，行数：{}，时间：{}：{} \n", __FILE__, __LINE__, __TIMESTAMP__, str.c_str()).c_str());


//格式化参数
template<typename... ARGS>
std::string Log(const char* szFmt, ARGS... args) {
  char szBuf[0x1000]{};
  sprintf(szBuf, szFmt, args...);
  std::string strOutput = "strOutput[ZQZQ] ";
  
  strOutput += szBuf;
  return strOutput;
  
}



