#ifndef MULTI_THREAD_H
#define MULTI_THREAD_H

#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

/// @brief Check and join the thread when possible
/// @param _t The thread to guard
class thread_guard {
  public:
    thread_guard(std::thread &_t) : t(_t) {}
    ~thread_guard() {
      if (t.joinable()) {
        t.join();
      }
    }

    thread_guard(thread_guard const&) = delete;
    thread_guard& operator = (thread_guard const&) = delete;

  private:
    std::thread &t;
};

/// @brief Check for thread's condition and join when possible
/// @param _t The thread to guard
/// @param _cond The condition to check, must not be loaded to be joinable
class thread_guard_condition {
  public:
    thread_guard_condition(std::thread &_t, std::atomic_bool &_cond) : t(_t), cond(_cond) {}
    ~thread_guard_condition() {
      if (t.joinable() && !cond.load()) {
        t.join();
      }
    }

    thread_guard_condition(thread_guard_condition const&) = delete;
    thread_guard_condition& operator = (thread_guard_condition const&) = delete;

  private:
    std::thread &t;
    std::atomic_bool &cond;

};


// Class that represents a simple thread pool
class thread_pool {
	public:
		// Constructor to creates a thread pool with given number of threads
		thread_pool(size_t num_threads = std::thread::hardware_concurrency() - 4) {
			// Creating worker threads

			std::cout << "Max hardware thread count: " << std::thread::hardware_concurrency() << std::endl;
			for (size_t i = 0; i < num_threads; ++i) {
				threads_.emplace_back([this] {
					while (true) {
						std::function<void()> task;
						{
							// Locking the queue so that data
							// can be shared safely
							std::unique_lock<std::mutex> lock(queue_mutex_);

							// Waiting until there is a task to
							// execute or the pool is stopped
							cv_.wait(lock,
									[this] { return !tasks_.empty() || stop_; });

							// exit the thread in case the pool
							// is stopped and there are no tasks
							if (stop_ && tasks_.empty()) {
								return;
							}

							// Get the next task from the queue
							task = std::move(tasks_.front());
							tasks_.pop();
						}

						task();
					}
				});
			}
		}

		// Destructor to stop the thread pool
		~thread_pool() {
			{
				// Lock the queue to update the stop flag safely
				std::unique_lock<std::mutex> lock(queue_mutex_);
				stop_ = true;
			}

			// Notify all threads
			cv_.notify_all();

			// Joining all worker threads to ensure they have
			// completed their tasks
			for (auto& thread : threads_) {
				thread.join();
			}
		}

		// Enqueue task for execution by the thread pool
		void enqueue(std::function<void()> task) {
			{
				std::unique_lock<std::mutex> lock(queue_mutex_);
				tasks_.emplace(std::move(task));
			}
			cv_.notify_one();
		}

	private:
		std::vector<std::thread> threads_;
		std::queue<std::function<void()>> tasks_;
		std::mutex queue_mutex_;
		// Condition variable to signal changes in the state of the tasks queue
		std::condition_variable cv_;

		// Flag to indicate whether the thread pool should stop or not
		bool stop_ = false;
};

#endif