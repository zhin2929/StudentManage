#include "ThreadPool.hpp"
using namespace std::literals::chrono_literals;

ThreadPool::ThreadPool()
  : m_nThreadInitSize(0),
  m_nTaskSize(0),
  m_nTaskQueueMaxSize(TASK_QUEUE_MAX_SIZE),
  m_poolMode(MODE_FIXED),
  m_bIsPoolRunning(false),
  m_nCurThreadSize(0),
  m_nThreadMaxSize(THREAD_MAX_SIZE),
  m_nIdleThreadSize(0) {
}

ThreadPool::~ThreadPool() {
  m_bIsPoolRunning = false;
  //通知所有等待任务队列非空的线程，去销毁
  m_cvNotEmpty.notify_all();
  //等待执行任务的线程结束
  std::unique_lock lock(m_taskQueueMtx);
  m_cvExit.wait(lock,
    [&]() {
      return m_threads.size() == 0;
    }
  );
  printf("线程资源全部退出\n");
}

void ThreadPool::SetMode(PoolMode mode) {
  if (CheckRunningState()) {
    return; // 启动后不允许修改线程模式
  }
  m_poolMode = mode;
}

void ThreadPool::SetInitThreadSize(size_t nInitThreadSize) {
  m_nThreadInitSize = nInitThreadSize;
}

void ThreadPool::SetTaskQueueMaxSize(int nQueueMaxSize) {
  m_nTaskQueueMaxSize = nQueueMaxSize;
}
//设置cached线程池阈值
void ThreadPool::SetThreadMaxSize(int nThreadSize) {
  if (CheckRunningState()) {
    return; // 启动后不允许修改线程最大数量
  }
  if (m_poolMode == PoolMode::MODE_CACHED) {
    m_nThreadMaxSize = nThreadSize;
  }
}

//提交任务到线程池
void ThreadPool::Start(size_t nInitThreadSize) {
  m_nThreadInitSize = nInitThreadSize; // 线程初始数量
  m_nCurThreadSize = nInitThreadSize; // 线程池当前线程数量
  m_bIsPoolRunning = true;
  // 创建初始线程数量
  for (int i = 0; i < m_nThreadInitSize; ++i) {
    auto ptr = std::make_unique<MyThread>(std::bind(&ThreadPool::ThreadFunc, this, std::placeholders::_1));
    int id = ptr->GetId();
    //unique_ptr 需要转移资源到m_threads里
    m_threads.emplace(id, std::move(ptr));
  }
  for (const auto& threadItem : m_threads) {
    threadItem.second->Start();
    m_nIdleThreadSize++; //增加空闲任务
  }
}

//定义线程池的线程函数
void ThreadPool::ThreadFunc(int threadId) {
  auto lastTime = std::chrono::high_resolution_clock().now();
  while (true) {
    Task task;
    { // unique_lock 作用域
      printf("线程ID：%d 尝试获取任务\n", threadId);
      //先获取锁
      std::unique_lock lock(m_taskQueueMtx);
      while (m_taskQueue.size() == 0) {
        //解决死锁问题：任务等于0才销毁线程，有任务，不管是否结束线程池，都等任务运行完
        if (!m_bIsPoolRunning) {
          m_nCurThreadSize--;
          m_threads.erase(threadId);
          printf("析构销毁线程 %d, 剩余 %d 线程\n", threadId, m_nCurThreadSize.load());
          m_cvExit.notify_one();
          return; // 结束线程函数
        }

        //cached模式下，把空闲超过 60s 的线程销毁
        if (m_poolMode == PoolMode::MODE_CACHED && m_nCurThreadSize > m_nThreadInitSize) {
          //条件变量等待任务队列非空，超时返回
          if (std::cv_status::timeout
            == m_cvNotEmpty.wait_for(lock, std::chrono::seconds(1))) {
            auto now = std::chrono::high_resolution_clock().now();
            auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
            if (dur.count() > THREAD_MAX_IDLE_TIME) {
              m_threads.erase(threadId);
              m_nCurThreadSize--;
              m_nIdleThreadSize--;
              printf("空闲销毁线程 %d, 剩余 %d 线程\n", threadId, m_nCurThreadSize.load());
              return; //结束线程函数
            }
          }
        }
        else { //等待任务队列非空
          m_cvNotEmpty.wait(lock);
        }
      }

      //成功获取任务后，空闲线程--
      m_nIdleThreadSize--;

      printf("线程ID：%d 成功获取任务\n", threadId);
      //从任务队列中取一个任务出来
      task = m_taskQueue.front();
      m_taskQueue.pop();
      m_nTaskSize--;
      if (m_nTaskSize > 0) { //通知不为空，可以继续取任务
        m_cvNotEmpty.notify_all();
      }
      //通知任务队列未满，可以继续提交任务
      m_cvNotFull.notify_all();
    } //lock.unlock(); 必须解锁，否则要等到任务执行完才会释放

    assert(task);
    //当前线程负责执行这个任务
    task(); //执行 function<void()>
    m_nIdleThreadSize++; //执行完任务后，空闲线程++
    lastTime = std::chrono::high_resolution_clock().now();
  }
}

bool ThreadPool::CheckRunningState() {
  return m_bIsPoolRunning;
}