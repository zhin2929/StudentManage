#include "MyThread.h"
#include <thread>

int MyThread::generateId = 0;

MyThread::MyThread(ThreadFunc func)
  : m_func(func),
  m_threadId(generateId++) //ΪThread�����ID
{
}

MyThread::~MyThread() {}

void MyThread::Start() {
  //�����߳�ִ���̺߳���
  std::thread t(m_func, m_threadId);
  //printf("m_threadId = %d \n", m_threadId);
  t.detach();
}
