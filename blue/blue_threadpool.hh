#pragma once

#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

namespace BlueThreading {
template <typename T>
class ThreadSafeQueue {
public:
    ~ThreadSafeQueue() { invalidate(); }

    // Attempt to get the first value in the queue.
    // Returns true if a value was successfully written to the out parameter, false otherwise.
    auto try_pop(T &out) {
        std::lock_guard<std::mutex> lock{mutex};

        if (queue.empty() || !valid) return false;

        out = std::move(queue.front());
        queue.pop();

        return true;
    }

    // Get the first value in the queue.
    // Will block until a value is available unless clear is called or the instance is destructed.
    // Returns true if a value was successfully written to the out parameter, false otherwise.
    auto wait_pop(T &out) {
        std::unique_lock<std::mutex> lock{mutex};

        condition.wait(lock, [this]() { return !queue.empty() || !valid; });

        // Using the condition in the predicate ensures that spurious wakeups with a valid
        // but empty queue will not proceed, so only need to check for validity before proceeding.
        if (!valid) return false;

        out = std::move(queue.front());
        queue.pop();

        return true;
    }

    //Push a new value onto the queue.
    auto push(T value) {
        std::lock_guard<std::mutex> lock{mutex};
        queue.push(std::move(value));
        condition.notify_one();
    }

    auto empty() const {
        std::lock_guard<std::mutex> lock{mutex};
        return queue.empty();
    }

    // Clear all items from the queue.
    //Check whether or not the queue is empty.
    auto clear() {
        std::lock_guard<std::mutex> lock{mutex};
        while (!queue.empty()) queue.pop();

        condition.notify_all();
    }

    // Invalidate the queue.
    // Used to ensure no conditions are being waited on in wait_pop when
    // a thread or the application is trying to exit.
    // The queue is invalid after calling this method and it is an error
    // to continue using a queue after this method has been called.
    auto invalidate() {
        std::lock_guard<std::mutex> lock{mutex};
        valid = false;
        condition.notify_all();
    }

    // Returns whether or not this queue is valid.
    auto is_valid() const {
        std::lock_guard<std::mutex> lock{mutex};
        return valid;
    }

private:
    std::atomic_bool        valid{true};
    mutable std::mutex      mutex;
    std::queue<T>           queue;
    std::condition_variable condition;
};

class ThreadPool {
private:
    class IThreadTask {
    public:
        IThreadTask()          = default;
        virtual ~IThreadTask() = default;

        IThreadTask &operator=(const IThreadTask &rhs) = delete;
        IThreadTask(const IThreadTask &rhs)            = delete;

        IThreadTask &operator=(IThreadTask &&other) = default;
        IThreadTask(IThreadTask &&other)            = default;

        virtual auto execute() -> void = 0;
    };

    template <typename Func>
    class ThreadTask : public IThreadTask {
    public:
        ThreadTask(Func &&func) : func{std::move(func)} {}

        ~ThreadTask() override = default;

        ThreadTask &operator=(const ThreadTask &rhs) = delete;
        ThreadTask(const ThreadTask &rhs)            = delete;

        ThreadTask &operator=(ThreadTask &&other) = default;
        ThreadTask(ThreadTask &&other)            = default;

        auto execute() -> void override { func(); }

    private:
        Func func;
    };

public:
    // A wrapper around a std::future that adds the behavior of futures returned from std::async.
    // Specifically, this object will block and wait for execution to finish before going out of scope.
    template <typename T>
    class TaskFuture {
    public:
        TaskFuture(std::future<T> &&future) : future{std::move(future)} {
        }

        TaskFuture &operator=(const TaskFuture &rhs) = delete;
        TaskFuture(const TaskFuture &rhs)            = delete;

        TaskFuture &operator=(TaskFuture &&other) = default;
        TaskFuture(TaskFuture &&other)            = default;

        ~TaskFuture() {
            if (future.valid()) future.get();
        }

        auto get() { return future.get(); }

    private:
        std::future<T> future;
    };

public:
    ThreadPool()
        : ThreadPool{std::max(std::thread::hardware_concurrency(), 2u) - 1u} {
    }

    explicit ThreadPool(const std::uint32_t num_threads)
        : done{false},
          work_queue{},
          threads{} {
        for (std::uint32_t i = 0u; i < num_threads; ++i) {
            threads.emplace_back(&ThreadPool::worker, this);
        }
    }

    ThreadPool &operator=(const ThreadPool &rhs) = delete;
    ThreadPool(const ThreadPool &rhs)            = delete;

    ~ThreadPool() { destroy(); }

    template <typename Func, typename... Args>
    auto submit(Func &&func, Args &&... args) {
        auto bound_task = std::bind(std::forward<Func>(func), std::forward<Args>(args)...);

        using ResultType   = std::result_of_t<decltype(bound_task)()>;
        using PackagedTask = std::packaged_task<ResultType()>;
        using TaskType     = ThreadTask<PackagedTask>;

        PackagedTask           task{std::move(bound_task)};
        TaskFuture<ResultType> result{task.get_future()};
        work_queue.push(std::make_unique<TaskType>(std::move(task)));
        return result;
    }

private:
    void worker() {
        while (!done) {
            std::unique_ptr<IThreadTask> pTask{nullptr};
            if (work_queue.wait_pop(pTask)) {
                pTask->execute();
            }
        }
    }

    void destroy() {
        done = true;
        work_queue.invalidate();
        for (auto &thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

private:
    std::atomic_bool                              done;
    ThreadSafeQueue<std::unique_ptr<IThreadTask>> work_queue;
    std::vector<std::thread>                      threads;
};

namespace DefaultThreadPool {

inline ThreadPool &get_thread_pool(void) {
    static ThreadPool default_pool;
    return default_pool;
}

template <typename Func, typename... Args>
inline auto submit_job(Func &&func, Args &&... args) {
    return get_thread_pool().submit(std::forward<Func>(func), std::forward<Args>(args)...);
}
} // namespace DefaultThreadPool
} // namespace BlueThreading
