#pragma once

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <iostream>
#include <queue>
#include <type_traits>

template<typename T>
class SafeQueue {
public:
	SafeQueue() = default;

	~SafeQueue() = default;
	
	SafeQueue(const SafeQueue& other) = delete;
	
	SafeQueue& operator = (const SafeQueue& other) = delete;
	
	SafeQueue(SafeQueue&& other) = delete;

	SafeQueue& operator = (SafeQueue&& other) = delete;
	
	SafeQueue(const const SafeQueue&& other) = delete;
	
	bool empty() {
		std::unique_lock<std::mutex> locker(m_Mutex);
		return m_Queue.empty();
	}

	int size() {
		std::unique_lock<std::mutex> locker(m_Mutex);
		return m_Queue.size();
	}

	void push(T& value) {
		std::unique_lock<std::mutex> locker(m_Mutex);
		m_Queue.emplace(value);
	}

	void push(T&& value) {
		std::unique_lock<std::mutex> locker(m_Mutex);
		m_Queue.emplace(std::move(value));
	}

	bool pop(T& value) {
		std::unique_lock<std::mutex> locker(m_Mutex);
		if (m_Queue.empty()) {
			return false;
		}
		else {
			value = std::move(m_Queue.front());
			m_Queue.pop();
			return true;
		}
	}
private:
	std::queue<T> m_Queue;
	std::mutex m_Mutex;
};
// ��������У����������
// �ύ��������ͨ�ĺ����������������º����������ˣ��������ṹ�壩�����Ա������std::function
// ���⣺����ֵ��ͬ�������б�ͬ
// �ύ���񣬰�װ�ɲ�������ֵ�����������Ŀɵ��ö������ⲿ��ȡ�䷵��ֵ

class SimpleThreadPool {
public:
	SimpleThreadPool(const SimpleThreadPool& other) = delete;
	SimpleThreadPool& operator = (const SimpleThreadPool& other) = delete;
	SimpleThreadPool(SimpleThreadPool&& other) = delete;
	SimpleThreadPool& operator = (SimpleThreadPool&& other) = delete;
	
	SimpleThreadPool() :m_Threads(std::thread::hardware_concurrency()), m_RunningStatus(true) {
		initialize();
	}

	SimpleThreadPool(int threadNum) :m_Threads(threadNum), m_RunningStatus(true) {
		initialize();
	}
	template<typename Func, typename... Args>
	auto submitTask(Func&& func, Args... args) {
		using returnType = typename std::invoke_result<Func, Args...>::type;
		std::function<returnType()> taskWrapper1 = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);
		//Ĩ�������ͷ���ֵ�����
		auto taskWrapper2 = std::make_shared<std::packaged_task<returnType()>>(taskWrapper1);
		TaskType wrapperFunction = [taskWrapper2]() {
			(*taskWrapper2)();//Ĩ�������ͷ���
		};
		m_TaskQueue.push(wrapperFunction);
		m_CV.notify_one();
		return taskWrapper2->get_future();
	}
	~SimpleThreadPool() {
		m_RunningStatus = false;
		m_CV.notify_all();
		for (auto& thread : m_Threads) {
			if (thread.joinable()) {
				thread.join();
			}
		}
	}
private:
	using TaskType = std::function<void()>;
	SafeQueue<TaskType> m_TaskQueue;
	std::vector<std::thread> m_Threads;
	std::condition_variable m_CV;
	std::mutex m_Mutex;
	std::atomic<bool> m_RunningStatus;
	void initialize() {
		for (size_t i = 0; i < m_Threads.size(); ++i) {
			auto worker = [&]() {
				while (m_RunningStatus) {//����״̬
					TaskType task;
					bool isSuccess = false;
					{
						std::unique_lock<std::mutex> locker(m_Mutex);
						if (this->m_TaskQueue.empty()) {
							this->m_CV.wait(locker);
						}
						isSuccess = this->m_TaskQueue.pop(task);
					}
					if (isSuccess) {
						std::cout << "Start running task inworker:[ID]" << i << "\n";
						task();
						std::cout << "End running task inworker:[ID]" << i << "\n";
					}
					else {
						std::cout << "Threadpool closed\n";
					}
				}
			};
			m_Threads[i] = std::thread(worker);
		}
	}
};






