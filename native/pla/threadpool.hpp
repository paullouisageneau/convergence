/*************************************************************************
 *   Copyright (C) 2011-2017 by Paul-Louis Ageneau                       *
 *   paul-louis (at) ageneau (dot) org                                   *
 *                                                                       *
 *   This file is part of Plateform.                                     *
 *                                                                       *
 *   Plateform is free software: you can redistribute it and/or modify   *
 *   it under the terms of the GNU Affero General Public License as      *
 *   published by the Free Software Foundation, either version 3 of      *
 *   the License, or (at your option) any later version.                 *
 *                                                                       *
 *   Plateform is distributed in the hope that it will be useful, but    *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the        *
 *   GNU Affero General Public License for more details.                 *
 *                                                                       *
 *   You should have received a copy of the GNU Affero General Public    *
 *   License along with Plateform.                                       *
 *   If not, see <http://www.gnu.org/licenses/>.                         *
 *************************************************************************/

#ifndef PLA_THREADPOOL_H
#define PLA_THREADPOOL_H

#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

#include "pla/include.hpp"

namespace pla {

class ThreadPool {
public:
	ThreadPool(size_t threads); // TODO: max tasks in queue
	virtual ~ThreadPool(void);

	template <class F, class... Args>
	auto enqueue(F &&f, Args &&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

	virtual void clear(void);
	virtual void join(void);

protected:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;

	std::mutex mutex;
	std::condition_variable condition;
	bool joining;
};

inline ThreadPool::ThreadPool(size_t threads) : joining(false) {
	for (size_t i = 0; i < threads; ++i) {
		workers.emplace_back([this] {
			while (true) {
				try {
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock(mutex);
						condition.wait(lock, [this]() { return !tasks.empty() || joining; });
						if (tasks.empty())
							break;
						task = std::move(tasks.front());
						tasks.pop();
					}

					task();
				} catch (const std::exception &e) {
					LogWarn("ThreadPool", std::string("Unhandled exception: ") + e.what());
				}
			}
		});
	}
}

inline ThreadPool::~ThreadPool(void) {
	joining = true;
	clear();
	join();
}

template <class F, class... Args>
auto ThreadPool::enqueue(F &&f, Args &&... args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
	using type = typename std::result_of<F(Args...)>::type;

	auto task = std::make_shared<std::packaged_task<type()>>(
	    std::bind(std::forward<F>(f), std::forward<Args>(args)...));
	std::future<type> result = task->get_future();

	{
		std::unique_lock<std::mutex> lock(mutex);
		if (joining)
			throw std::runtime_error("enqueue on closing ThreadPool");

		// Add task
		tasks.emplace([task]() { (*task)(); });

		condition.notify_one();
	}

	return result;
}

inline void ThreadPool::clear(void) {
	{
		std::unique_lock<std::mutex> lock(mutex);

		while (!tasks.empty())
			tasks.pop();

		// condition.notify_all();	// useless
	}
}

inline void ThreadPool::join(void) {
	{
		std::unique_lock<std::mutex> lock(mutex);
		joining = true;
		condition.notify_all();
	}

	for (std::thread &w : workers)
		if (w.joinable())
			w.join();
}

} // namespace pla

#endif
