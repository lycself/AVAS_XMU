#pragma once
// Fork-join worker pool shared by the tracker and the space-charge solver.
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace avas {

// Minimal fork-join pool: run(n, fn) calls fn(0..n-1) on the workers and waits.
class WorkerPool {
public:
    explicit WorkerPool(int threads) {
        for (int i = 0; i < threads; ++i) workers_.emplace_back([this] { loop(); });
    }
    ~WorkerPool() {
        {
            std::lock_guard<std::mutex> g(m_);
            stop_ = true;
        }
        cv_.notify_all();
        for (auto& t : workers_) t.join();
    }
    void run(int n, const std::function<void(int)>& fn) {
        if (workers_.empty() || n <= 1) {
            for (int i = 0; i < n; ++i) fn(i);
            return;
        }
        std::unique_lock<std::mutex> g(m_);
        fn_ = &fn;
        next_ = 0;
        total_ = n;
        pending_ = n;
        ++generation_;
        cv_.notify_all();
        done_.wait(g, [this] { return pending_ == 0; });
        fn_ = nullptr;
    }

private:
    void loop() {
        unsigned seen = 0;
        std::unique_lock<std::mutex> g(m_);
        while (true) {
            cv_.wait(g, [&] { return stop_ || (generation_ != seen && next_ < total_); });
            if (stop_) return;
            while (next_ < total_) {
                const int k = next_++;
                const std::function<void(int)>* fn = fn_;
                g.unlock();
                (*fn)(k);
                g.lock();
                if (--pending_ == 0) done_.notify_all();
            }
            seen = generation_;
        }
    }
    std::vector<std::thread> workers_;
    std::mutex m_;
    std::condition_variable cv_, done_;
    const std::function<void(int)>* fn_ = nullptr;
    int next_ = 0, total_ = 0, pending_ = 0;
    unsigned generation_ = 0;
    bool stop_ = false;
};

}  // namespace avas
