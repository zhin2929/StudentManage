#pragma once

#include <functional>

class MyThread {
public:
  using ThreadFunc = std::function<void(int)>; // �̺߳�����������
  MyThread(ThreadFunc func); // �̹߳���
  ~MyThread(); // �߳�����
  void Start(); // �����߳�
  int GetId() const {
    return m_threadId;
  }
private:
  ThreadFunc m_func;
  static int generateId;
  int m_threadId;
};
