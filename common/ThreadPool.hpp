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

//线程池支持模式
enum class PoolMode {
  MODE_FIXED, //固定数量的线程池，不需要考虑线程安全
  MODE_CACHED, //不定数量的线程池，回收时需要考虑线程安全，适合小而快的任务
};

using enum PoolMode;



/*
使用示例:
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

  //设置线程池模式
  void SetMode(PoolMode mode);

  //设置初始线程数量
  void SetInitThreadSize(size_t nInitThreadSize);

  //设置任务队列最大数
  void SetTaskQueueMaxSize(int nQueueMaxSize);

  //设置线程池cached模式下线程最大数
  void SetThreadMaxSize(int nThreadSize);

  //给线程池提交任务
  template<typename Func, typename... Args>
  auto SubmitTask(Func&& func, Args&&... args) -> std::future<decltype(func(args...))>;

  //开启线程池，默认线程池大小为硬件并发数量
  void Start(size_t nInitThreadSize = std::thread::hardware_concurrency());

private:
  //线程函数，访问任务队列是否有任务，有则分配给其他线程
  void ThreadFunc(int threadId);

  //检查pool的运行状态
  bool CheckRunningState();

private:
  std::unordered_map<int, std::unique_ptr<MyThread>> m_threads; //线程容器
  size_t m_nThreadInitSize; //初始线程数量，用于fixed模式
  size_t m_nThreadMaxSize; //最大线程数量，用于Cached模式

  //之所以不用容器的 size() 方法表示当前线程数量，是因为线程不安全
  std::atomic_uint m_nCurThreadSize; //当前线程数量
  std::atomic_uint m_nIdleThreadSize; //空闲线程数量

  //不知道返回参数，先用 void，然后用lambda表达式包一层，再返回 Task 的返回值
  using Task = std::function<void()>;
  std::queue<Task> m_taskQueue; //任务队列
  std::atomic_uint m_nTaskSize; //任务数量
  size_t m_nTaskQueueMaxSize; //任务队列最大任务数量

  std::mutex m_taskQueueMtx; //保证任务队列的线程安全
  std::condition_variable m_cvNotFull;  //任务队列不满
  std::condition_variable m_cvNotEmpty; //任务队列不空
  std::condition_variable m_cvExit; //等待线程资源全部退出

  PoolMode m_poolMode; //线程池模式
  std::atomic_bool m_bIsPoolRunning; //线程池是否运行

  static constexpr const int TASK_QUEUE_MAX_SIZE = 1000; //任务队列最大数
  static constexpr const int THREAD_MAX_SIZE = 10; // 动态增加最大线程数 200
  static constexpr const int THREAD_MAX_IDLE_TIME = 60; // 单位秒，空闲时间回收
};

/************************************************************************************
 实现部分
************************************************************************************/
template<typename Func, typename... Args>
auto ThreadPool::SubmitTask(Func&& func, Args&&... args) 
-> std::future<decltype(func(args...))> {
  //使用 decltype 获取该函数类型
  using RType = decltype(func(args...));
  // 1. 先用 bind 绑定函数指针 Func，参数 args
  // 2. 使用 packaged_task 打包第一步绑定的可调用对象
  // 3. 生成 packaged_task 对象实例的智能指针
  auto task =
    std::make_shared<std::packaged_task<RType()>>(
      std::bind(
        std::forward<Func>(func),
        std::forward<Args>(args)...
      )
    );
  std::future<RType> result = task->get_future();

  //获取锁
  std::unique_lock lock(m_taskQueueMtx);
  //线程通信，等待任务队列有空闲位置
  bool bIsSpare = m_cvNotFull.wait_for(lock,
    std::chrono::seconds(1),
    [this]() -> bool {
      int size = m_taskQueue.size();
      return size < m_nTaskQueueMaxSize;
    }
  );

  if (!bIsSpare) { // 1 秒内没有等到，设为提交失败
    printf("线程ID：%d 提交任务失败\n", std::this_thread::get_id());
    auto task =
      std::make_shared<std::packaged_task<RType()>>(
        []() -> RType { return RType(); }
    );
    (*task)(); //task是智能指针，需要先解引用
    return task->get_future();
  }
  //放入任务队列
  m_taskQueue.emplace([task]() { (*task)(); });
  m_nTaskSize++; //任务数量++

  //此时任务队列不空，唤醒所有线程
  m_cvNotEmpty.notify_all();

  //Cached 模式，适合小而快的任务
  if (m_poolMode == MODE_CACHED
    && m_nTaskSize > m_nIdleThreadSize //任务数量大于空闲数量
    && m_nCurThreadSize < m_nThreadMaxSize) { //当前线程池的线程数量小于总线程数量
    //创建新线程
    auto pThread = std::make_unique<MyThread>(std::bind(&ThreadPool::ThreadFunc, this, std::placeholders::_1));
    int id = pThread->GetId();
    //unique_ptr 不允许复制，需要转移资源到m_threads里
    m_threads.emplace(id, std::move(pThread));
    m_threads[id]->Start(); //启动线程
    m_nCurThreadSize++;
    m_nIdleThreadSize++; //当前线程数量++，空闲线程++
    printf("创建线程 现有 %d 个线程\n", m_nCurThreadSize.load());
  }
  return result; //返回任务的Result对象
}




