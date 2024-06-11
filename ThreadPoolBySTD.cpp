#include "ThreadPoolBySTD.h"

int very_time_consuming_task(int a, int b) {
	std::this_thread::sleep_for(std::chrono::seconds(1));
	return a + b;
}

int main()
{
	SimpleThreadPool simpleThreadPool(12);
	int taskNum = 30;
	std::vector<std::future<int>> result(30);
	std::cout << "Start to submit tasks...\n";
	for (size_t i = 0; i < taskNum; ++i) {
		result[i] = simpleThreadPool.submitTask(very_time_consuming_task, i, i + 1);
	}
	std::cout << "End submit tasks...\n";
	//dosth else
	std::cout << "Main thread do something else...\n";
	std::this_thread::sleep_for(std::chrono::seconds(3));
	std::cout << "Main thread task finished...\n";
	std::cout << "Try getting threadpool task result...\n";
	for (size_t i = 0; i < taskNum; ++i) {
		std::cout << "result[" << i << "]" << result[i].get();
	}
	std::cout << "End of getting result...\n";
}












