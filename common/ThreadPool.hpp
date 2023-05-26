#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <future>
#include <semaphore>
#include <iostream>
#include <cassert>

#include "MyThread.h"

//�̳߳�֧��ģʽ
enum class PoolMode {
  MODE_FIXED, //�̶��������̳߳أ�����Ҫ�����̰߳�ȫ
  MODE_CACHED, //�����������̳߳أ�����ʱ��Ҫ�����̰߳�ȫ���ʺ�С���������
};

using enum PoolMode;



/*
ʹ��ʾ��:
ThreadPool pool;
pool.SetMode(PoolMode::MODE_CACHED);
pool.Start();
int Sum(int a, int b) { return a + b; }
auto res1 = pool.SubmitTask(Sum, 1, 2);
printf("%d\n", res1.get());
*/
class ThreadPool {
public:
  ThreadPool();

  ~ThreadPool();

  ThreadPool(const ThreadPool&) = delete;

  ThreadPool& operator=(const ThreadPool&) = delete;

  //�����̳߳�ģʽ
  void SetMode(PoolMode mode);

  //���ó�ʼ�߳�����
  void SetInitThreadSize(size_t nInitThreadSize);

  //����������������
  void SetTaskQueueMaxSize(int nQueueMaxSize);

  //�����̳߳�cachedģʽ���߳������
  void SetThreadMaxSize(int nThreadSize);

  //���̳߳��ύ����
  template<typename Func, typename... Args>
  auto SubmitTask(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>;

  //�����̳߳أ�Ĭ���̳߳ش�СΪӲ����������
  void Start(size_t nInitThreadSize = std::thread::hardware_concurrency());

private:
  //�̺߳�����������������Ƿ��������������������߳�
  void ThreadFunc(int threadId);

  //���pool������״̬
  bool CheckRunningState();

private:
  std::unordered_map<int, std::unique_ptr<MyThread>> m_threads; //�߳�����
  size_t m_nThreadInitSize; //��ʼ�߳�����������fixedģʽ
  size_t m_nThreadMaxSize; //����߳�����������Cachedģʽ

  //֮���Բ��������� size() ������ʾ��ǰ�߳�����������Ϊ�̲߳���ȫ
  std::atomic_uint m_nCurThreadSize; //��ǰ�߳�����
  std::atomic_uint m_nIdleThreadSize; //�����߳�����

  //��֪�����ز��������� void��Ȼ����lambda���ʽ��һ�㣬�ٷ��� Task �ķ���ֵ
  using Task = std::function<void()>;
  std::queue<Task> m_taskQueue; //�������
  std::atomic_uint m_nTaskSize; //��������
  size_t m_nTaskQueueMaxSize; //������������������

  std::mutex m_taskQueueMtx; //��֤������е��̰߳�ȫ
  std::condition_variable m_cvNotFull;  //������в���
  std::condition_variable m_cvNotEmpty; //������в���
  std::condition_variable m_cvExit; //�ȴ��߳���Դȫ���˳�

  PoolMode m_poolMode; //�̳߳�ģʽ
  std::atomic_bool m_bIsPoolRunning; //�̳߳��Ƿ�����

  static constexpr const int TASK_QUEUE_MAX_SIZE = 1000; //������������
  static constexpr const int THREAD_MAX_SIZE = 10; // ��̬��������߳��� 200
  static constexpr const int THREAD_MAX_IDLE_TIME = 60; // ��λ�룬����ʱ�����
};

/************************************************************************************
 ʵ�ֲ���
************************************************************************************/
template<typename Func, typename... Args>
auto ThreadPool::SubmitTask(Func&& func, Args&&... args) 
-> std::future<decltype(func(args...))> {
  //ʹ�� decltype ��ȡ�ú�������
  using RType = decltype(func(args...));
  // 1. ���� bind �󶨺���ָ�� Func������ args
  // 2. ʹ�� packaged_task �����һ���󶨵Ŀɵ��ö���
  // 3. ���� packaged_task ����ʵ��������ָ��
  auto task =
    std::make_shared<std::packaged_task<RType()>>(
      std::bind(
        std::forward<Func>(func),
        std::forward<Args>(args)...
      )
    );
  std::future<RType> result = task->get_future();

  //��ȡ��
  std::unique_lock lock(m_taskQueueMtx);
  //�߳�ͨ�ţ��ȴ���������п���λ��
  bool bIsSpare = m_cvNotFull.wait_for(lock,
    std::chrono::seconds(1),
    [this]() -> bool {
      int size = m_taskQueue.size();
      return size < m_nTaskQueueMaxSize;
    }
  );

  if (!bIsSpare) { // 1 ����û�еȵ�����Ϊ�ύʧ��
    printf("�߳�ID��%d �ύ����ʧ��\n", std::this_thread::get_id());
    auto task =
      std::make_shared<std::packaged_task<RType()>>(
        []() -> RType { return RType(); }
    );
    (*task)(); //task������ָ�룬��Ҫ�Ƚ�����
    return task->get_future();
  }
  //�����������
  m_taskQueue.emplace([task]() { (*task)(); });
  m_nTaskSize++; //��������++

  //��ʱ������в��գ����������߳�
  m_cvNotEmpty.notify_all();

  //Cached ģʽ���ʺ�С���������
  if (m_poolMode == MODE_CACHED
    && m_nTaskSize > m_nIdleThreadSize //�����������ڿ�������
    && m_nCurThreadSize < m_nThreadMaxSize) { //��ǰ�̳߳ص��߳�����С�����߳�����
    //�������߳�
    auto pThread = std::make_unique<MyThread>(std::bind(&ThreadPool::ThreadFunc, this, std::placeholders::_1));
    int id = pThread->GetId();
    //unique_ptr �������ƣ���Ҫת����Դ��m_threads��
    m_threads.emplace(id, std::move(pThread));
    m_threads[id]->Start(); //�����߳�
    m_nCurThreadSize++;
    m_nIdleThreadSize++; //��ǰ�߳�����++�������߳�++
    printf("�����߳� ���� %d ���߳�\n", m_nCurThreadSize.load());
  }
  return result; //���������Result����
}




