#include "parallel_al.h"

#include <atomic>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <vector>
#ifdef HAVE_LIBNX
#include <switch.h>
#include <memory>
#else
#include <condition_variable>
#include <mutex>
#include <thread>
#endif

static int thread_counter = 0;
class Parallel
{
  public:
    Parallel(std::uint32_t num_workers) : m_num_workers(std::min(num_workers, 64U))
    {
        // mask for m_tasks_done when all workers have finished their task
        // except for worker 0, which runs in the main thread
        m_all_tasks_done = ((1LL << m_num_workers) - 1) & ~1;

        // give workers an empty task
        m_task = [](std::uint32_t) {};

#ifdef HAVE_LIBNX
        // initialize thread stuff
        mutexInit(&m_signal_mutex);
        condvarInit(&m_signal_work);
        condvarInit(&m_signal_done);
#endif
        m_accept_work = true;
        start_work();

        // create worker threads
        for (std::uint32_t worker_id = 1; worker_id < m_num_workers; worker_id++)
        {
#ifdef HAVE_LIBNX
            struct args_struct *args = (args_struct *)malloc(sizeof(struct args_struct));
            args->parallel = this;
            args->worker_id = worker_id;
            Thread thread;
            u32 prio = 0;
            svcGetThreadPriority(&prio, CUR_THREAD_HANDLE);
            if (thread_counter >= 3)
                thread_counter = 0;
            threadCreate(&thread, &Parallel::do_work, args, 0x40000, prio - 10, thread_counter);
            thread_counter++;
            threadStart(&thread);
            m_workers.emplace_back(thread);
#else
            m_workers.emplace_back(std::thread(&Parallel::do_work, this, worker_id));
#endif
        }

        // synchronize workers to prepare them for real tasks
        wait();
    }

    ~Parallel()
    {
        // wait for all workers to finish their current work
        wait();

        // exit worker main loops
        m_accept_work = false;
        start_work();

        // join worker threads to make sure they have finished
        for (auto &thread : m_workers)
        {
#ifdef HAVE_LIBNX
            threadWaitForExit(&thread);
            threadClose(&thread);
#else
            thread.join();
#endif
        }

        // destroy all worker threads
        m_workers.clear();
    }

    void run(std::function<void(std::uint32_t)> &&task)
    {
        // don't allow more tasks if workers are stopping
        if (!m_accept_work)
        {
            throw std::runtime_error("Workers are exiting and no longer accept work");
        }

        // prepare task for workers and send signal so they start working
        m_task = task;
        start_work();

        // run worker 0 directly on main thread
        m_task(0);

        // wait for all workers to finish
        wait();
    }

    std::uint32_t num_workers()
    {
        return m_num_workers;
    }

  private:
#ifdef HAVE_LIBNX
    std::vector<Thread> m_workers;
    Mutex m_signal_mutex;
    CondVar m_signal_work;
    CondVar m_signal_done;
    struct args_struct
    {
        Parallel *parallel;
        int worker_id;
    };
#else
    std::vector<std::thread> m_workers;
    std::mutex m_signal_mutex;
    std::condition_variable m_signal_work;
    std::condition_variable m_signal_done;
#endif
    std::function<void(std::uint32_t)> m_task;
    std::atomic<uint64_t> m_tasks_done;
    std::uint64_t m_all_tasks_done;
    std::atomic<bool> m_accept_work;
    const std::uint32_t m_num_workers;

    void start_work()
    {
#ifdef HAVE_LIBNX
        mutexLock(&m_signal_mutex);

        // clear task bits for all workers
        m_tasks_done = 0;

        // wake up all workers
        condvarWakeAll(&m_signal_work);

        mutexUnlock(&m_signal_mutex);
#else
        std::unique_lock<std::mutex> ul(m_signal_mutex);

        // clear task bits for all workers
        m_tasks_done = 0;

        // wake up all workers
        m_signal_work.notify_all();
#endif
    }

#ifdef HAVE_LIBNX
    static void do_work(void *arg)
    {
        struct args_struct *args = (struct args_struct *)arg;

        const std::uint64_t worker_mask = 1LL << args->worker_id;

        while (args->parallel->m_accept_work)
        {
            // do the work
            args->parallel->m_task(args->worker_id);

            {
                mutexLock(&args->parallel->m_signal_mutex);

                // mark task as done
                args->parallel->m_tasks_done |= worker_mask;

                // notify main thread
                condvarWakeOne(&args->parallel->m_signal_done);

                // take a break and wait for more work
                while ((args->parallel->m_tasks_done & worker_mask) != 0)
                {
                    condvarWait(&args->parallel->m_signal_work, &args->parallel->m_signal_mutex);
                }

                mutexUnlock(&args->parallel->m_signal_mutex);
            }
        }
    }
#else
    void do_work(std::uint32_t worker_id)
    {
        const std::uint64_t worker_mask = 1LL << worker_id;

        while (m_accept_work)
        {
            // do the work
            m_task(worker_id);

            {
                std::unique_lock<std::mutex> ul(m_signal_mutex);

                // mark task as done
                m_tasks_done |= worker_mask;

                // notify main thread
                m_signal_done.notify_one();

                // take a break and wait for more work
                m_signal_work.wait(ul, [worker_mask, this] {
                    return (m_tasks_done & worker_mask) == 0;
                });
            }
        }
    }
#endif

    void wait()
    {
        // wait for all workers to set their task bits
#ifdef HAVE_LIBNX
        mutexLock(&m_signal_mutex);
        while (m_tasks_done != m_all_tasks_done)
        {
            condvarWait(&m_signal_done, &m_signal_mutex);
        }
        mutexUnlock(&m_signal_mutex);
#else
        std::unique_lock<std::mutex> ul(m_signal_mutex);
        m_signal_done.wait(ul, [this] {
            return m_tasks_done == m_all_tasks_done;
        });
#endif
    }

    void operator=(const Parallel &) = delete;
    Parallel(const Parallel &) = delete;
};

// C interface for the Parallel class
static std::unique_ptr<Parallel> parallel;

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args)
{
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

void parallel_alinit(uint32_t num)
{
    // auto-select number of workers based on the number of cores
    if (num == 0)
    {
#ifdef HAVE_LIBNX
        num = 5; // sweet spot
#else
        num = std::thread::hardware_concurrency();
#endif
    }

    parallel = make_unique<Parallel>(num);
}

void parallel_run(void task(uint32_t))
{
    parallel->run(task);
}

uint32_t parallel_num_workers()
{
    return parallel->num_workers();
}

void parallel_close()
{
    parallel.reset();
}
