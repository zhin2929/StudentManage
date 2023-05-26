#pragma once

#include <functional>

class MyThread {
public:
  using ThreadFunc = std::function<void(int)>; // 线程函数对象类型
  MyThread(ThreadFunc func); // 线程构造
  ~MyThread(); // 线程析构
  void Start(); // 启动线程
  int GetId() const {
    return m_threadId;
  }
private:
  ThreadFunc m_func;
  static int generateId;
  int m_threadId;
};
