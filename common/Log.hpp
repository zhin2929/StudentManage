#pragma once
#include <windows.h>
#include <string>
#include <format>


#define LOG1(str) \
  char szOutput[0x1000]{}; \
  sprintf(szOutput,"�ļ���%s��������%d��ʱ�䣺%s��%s \n", \
__FILE__, __LINE__, __TIMESTAMP__, str.c_str());	\
  printf("����̨�����%s\n", szOutput);	\
  OutputDebugString(szOutput);

#define LOG2(str) \
  std::string strOutput = std::format("�ļ���{}��������{}��ʱ�䣺{}��{} \n",      \
    __FILE__, __LINE__, __TIMESTAMP__, str.c_str());    \
  printf("����̨�����%s\n", strOutput.c_str()); \
  OutputDebugString(strOutput.c_str());

#define LOG(str) \
  printf("%s\n", std::format("�ļ���{}��������{}��ʱ�䣺{}��{} \n", __FILE__, __LINE__, __TIMESTAMP__, str.c_str()).c_str()); \
  OutputDebugString(std::format("�ļ���{}��������{}��ʱ�䣺{}��{} \n", __FILE__, __LINE__, __TIMESTAMP__, str.c_str()).c_str());

#define LOG3(str) \
  OutputDebugString(std::format("ZQZQ�ļ���{}��������{}��ʱ�䣺{}��{} \n", __FILE__, __LINE__, __TIMESTAMP__, str.c_str()).c_str());


//��ʽ������
template<typename... ARGS>
std::string Log(const char* szFmt, ARGS... args) {
  char szBuf[0x1000]{};
  sprintf(szBuf, szFmt, args...);
  std::string strOutput = "strOutput[ZQZQ] ";
  
  strOutput += szBuf;
  return strOutput;
  
}



