#include<thread>
#include<condition_variable>
#include<vector>
#include<queue>
#include<atomic>
#include<iostream>
class ThreadPool final
{
public:
    ThreadPool(const unsigned thread_num = 4):thread_num_(thread_num)
    {}
    ~ThreadPool(){stop();}
    template<typename Functor>
    void pushTask(Functor&& functor) noexcept
    {
        tasks_.emplace(std::forward<Functor>(functor));
        std::lock_guard<std::mutex> lck(mtx_);
        cond_var_.notify_all();
    }
    template<typename Functor, typename... Args>
    void pushTask(Functor&& functor, Args&&... args) noexcept
    {
         pushTask(std::bind(functor, std::forward<Args>(args)...));
    }
    void clearTask() noexcept
    {
        std::lock_guard<std::mutex> lck(mtx_);
        decltype(tasks_) empty_queue;
        std::swap(tasks_, empty_queue);
    }
    void start() noexcept
    {
        is_running_ = true;
        for(unsigned i = 0; i < thread_num_; i++)
        {
             workers_.emplace_back(&ThreadPool::work, this, i);
        }
    }
    void stop() noexcept
    {
         if(!is_running_) return;
         is_running_ = false;
         clearTask();
         cond_var_.notify_all();
         for(auto& worker : workers_)
         {
              worker.join();
         }
    }
    void restart() noexcept
    {
         stop();
         start();
    }
private:
    void work(const unsigned thread_id)
    {
        while(is_running_)
        {
            std::function<void()> task = nullptr;
            {
                auto lck = std::unique_lock<std::mutex>(mtx_);
                if(tasks_.empty()) cond_var_.wait(lck);
                std::cout << "Thread " << thread_id << " woke up" << std::endl;
                if(tasks_.empty()) continue;
                task = tasks_.front();
                tasks_.pop();
            }
            try
            {
                 task();

            }
            catch(...)
            {
                 throw;
            }
        }
    }

private:
    size_t thread_num_;
    std::vector<std::thread> workers_;
    std::mutex mtx_;
    std::condition_variable cond_var_;
    std::atomic<bool> is_running_;
    std::queue<std::function<void()> > tasks_;
};
