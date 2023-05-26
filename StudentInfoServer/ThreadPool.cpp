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
  //֪ͨ���еȴ�������зǿյ��̣߳�ȥ����
  m_cvNotEmpty.notify_all();
  //�ȴ�ִ��������߳̽���
  std::unique_lock lock(m_taskQueueMtx);
  m_cvExit.wait(lock,
    [&]() {
      return m_threads.size() == 0;
    }
  );
  printf("�߳���Դȫ���˳�\n");
}

void ThreadPool::SetMode(PoolMode mode) {
  if (CheckRunningState()) {
    return; // �����������޸��߳�ģʽ
  }
  m_poolMode = mode;
}

void ThreadPool::SetInitThreadSize(size_t nInitThreadSize) {
  m_nThreadInitSize = nInitThreadSize;
}

void ThreadPool::SetTaskQueueMaxSize(int nQueueMaxSize) {
  m_nTaskQueueMaxSize = nQueueMaxSize;
}
//����cached�̳߳���ֵ
void ThreadPool::SetThreadMaxSize(int nThreadSize) {
  if (CheckRunningState()) {
    return; // �����������޸��߳��������
  }
  if (m_poolMode == PoolMode::MODE_CACHED) {
    m_nThreadMaxSize = nThreadSize;
  }
}

//�ύ�����̳߳�
void ThreadPool::Start(size_t nInitThreadSize) {
  m_nThreadInitSize = nInitThreadSize; // �̳߳�ʼ����
  m_nCurThreadSize = nInitThreadSize; // �̳߳ص�ǰ�߳�����
  m_bIsPoolRunning = true;
  // ������ʼ�߳�����
  for (int i = 0; i < m_nThreadInitSize; ++i) {
    auto ptr = std::make_unique<MyThread>(std::bind(&ThreadPool::ThreadFunc, this, std::placeholders::_1));
    int id = ptr->GetId();
    //unique_ptr ��Ҫת����Դ��m_threads��
    m_threads.emplace(id, std::move(ptr));
  }
  for (const auto& threadItem : m_threads) {
    threadItem.second->Start();
    m_nIdleThreadSize++; //���ӿ�������
  }
}

//�����̳߳ص��̺߳���
void ThreadPool::ThreadFunc(int threadId) {
  auto lastTime = std::chrono::high_resolution_clock().now();
  while (true) {
    Task task;
    { // unique_lock ������
      printf("�߳�ID��%d ���Ի�ȡ����\n", threadId);
      //�Ȼ�ȡ��
      std::unique_lock lock(m_taskQueueMtx);
      while (m_taskQueue.size() == 0) {
        //����������⣺�������0�������̣߳������񣬲����Ƿ�����̳߳أ���������������
        if (!m_bIsPoolRunning) {
          m_nCurThreadSize--;
          m_threads.erase(threadId);
          printf("���������߳� %d, ʣ�� %d �߳�\n", threadId, m_nCurThreadSize.load());
          m_cvExit.notify_one();
          return; // �����̺߳���
        }

        //cachedģʽ�£��ѿ��г��� 60s ���߳�����
        if (m_poolMode == PoolMode::MODE_CACHED && m_nCurThreadSize > m_nThreadInitSize) {
          //���������ȴ�������зǿգ���ʱ����
          if (std::cv_status::timeout
            == m_cvNotEmpty.wait_for(lock, std::chrono::seconds(1))) {
            auto now = std::chrono::high_resolution_clock().now();
            auto dur = std::chrono::duration_cast<std::chrono::seconds>(now - lastTime);
            if (dur.count() > THREAD_MAX_IDLE_TIME) {
              m_threads.erase(threadId);
              m_nCurThreadSize--;
              m_nIdleThreadSize--;
              printf("���������߳� %d, ʣ�� %d �߳�\n", threadId, m_nCurThreadSize.load());
              return; //�����̺߳���
            }
          }
        }
        else { //�ȴ�������зǿ�
          m_cvNotEmpty.wait(lock);
        }
      }

      //�ɹ���ȡ����󣬿����߳�--
      m_nIdleThreadSize--;

      printf("�߳�ID��%d �ɹ���ȡ����\n", threadId);
      //�����������ȡһ���������
      task = m_taskQueue.front();
      m_taskQueue.pop();
      m_nTaskSize--;
      if (m_nTaskSize > 0) { //֪ͨ��Ϊ�գ����Լ���ȡ����
        m_cvNotEmpty.notify_all();
      }
      //֪ͨ�������δ�������Լ����ύ����
      m_cvNotFull.notify_all();
    } //lock.unlock(); �������������Ҫ�ȵ�����ִ����Ż��ͷ�

    assert(task);
    //��ǰ�̸߳���ִ���������
    task(); //ִ�� function<void()>
    m_nIdleThreadSize++; //ִ��������󣬿����߳�++
    lastTime = std::chrono::high_resolution_clock().now();
  }
}

bool ThreadPool::CheckRunningState() {
  return m_bIsPoolRunning;
}