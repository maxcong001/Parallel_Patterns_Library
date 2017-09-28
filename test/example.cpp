
#include "logger/logger.hpp"
#include "pplx/threadpool.h"
#include "pplx/pplxtasks.h"
#include <atomic>
#include <memory>
std::shared_ptr<pplx::scheduler_interface> __cdecl get_scheduler()
{
    return pplx::get_ambient_scheduler();
}
class TaskOptionsTestScheduler : public pplx::scheduler_interface
{
  public:
    TaskOptionsTestScheduler() : m_numTasks(0), m_scheduler(get_scheduler())
    {
    }

    virtual void schedule(pplx::TaskProc_t proc, void *param)
    {
        m_numTasks++;
        m_scheduler->schedule(proc, param);
    }

    long get_num_tasks()
    {
        return m_numTasks;
    }

  private:
    std::atomic<long> m_numTasks;
    pplx::scheduler_ptr m_scheduler;

    TaskOptionsTestScheduler(const TaskOptionsTestScheduler &);
    TaskOptionsTestScheduler &operator=(const TaskOptionsTestScheduler &);
};
int main()
{
    set_log_level(logger_iface::log_level::debug);
    __LOG(error, "hello logger!"
                     << "this is error log");
    __LOG(warn, "hello logger!"
                    << "this is warn log");
    __LOG(info, "hello logger!"
                    << "this is info log");
    __LOG(debug, "hello logger!"
                     << "this is debug log");

    {
        pplx::task_completion_event<long> tce;
        auto val = pplx::create_task(tce);

        std::atomic<long> v(0);

        auto t = val.then(
                        [&](long x) {
                            __LOG(error, " x is : " << x);
                            v.exchange(x);
                            x = 9;
                            return pplx::task_from_result(x);
                        })
                     .then([&](long x) {
                         __LOG(error, "in the second then x is " << x);
                         x = 8;
                         return pplx::task_from_result(x);

                     })
                     .then([&](long x) {
                         __LOG(error, "in the third then, x is : " << x);
                         return x;

                     });

        // Start the task
        tce.set(10);

        // Wait for the task.
        try
        {
            t.wait();
        }
        catch (pplx::invalid_operation)
        {
        }
        catch (std::exception_ptr)
        {
            v = 1;
        }
        __LOG(error, "v is : " << v << "  should be " << 10);
        __LOG(error, "t.get () returns : " << t.get());
    }
    {
        pplx::task_completion_event<void> tce;
        auto val = pplx::create_task(tce);

        auto t = val.then(
            [&]() {
                __LOG(error, " in the test task ");

            });
        tce.set();
        // Start the task
        tce.set_exception(pplx::invalid_operation());

        // Wait for the task.
        try
        {
            t.wait();
        }
        catch (pplx::invalid_operation)
        {
        }
        catch (std::exception_ptr)
        {
        }
        __LOG(error, "void test exit");
    }

#if 0
    {
        // Same as the above test but use multiple when_all
        TaskOptionsTestScheduler sched1;
        TaskOptionsTestScheduler sched2;

        std::vector<pplx::task<int>> taskVect;
        const int n = 10;
        for (int i = 0; i < n; i++)
        {
            taskVect.push_back(pplx::create_task([i]() -> int {
                __LOG(error, "in the task");
                return i;
            },
                                                 sched1));
        }

        auto t2 = pplx::create_task([]() -> int {
            __LOG(error, "in the task");
            return 0;
        },
                                    sched1);

        auto t3 = pplx::when_all(begin(taskVect), end(taskVect), sched2); // Does not run on the scheduler - it should run inline

        auto t4 = t2 && t3;
        t4.then([](std::vector<int>) { __LOG(error, "in the task t4!!!!!"); }).wait(); // run on default scheduler as the operator && breaks the chain of inheritance

        __LOG(error, "sched1.get_num_tasks() is : " << sched1.get_num_tasks() << "  should be " << (n + 1));
        __LOG(error, "sched2.get_num_tasks() is : " << sched2.get_num_tasks() << "  should be " << 1);
    }
    {
        pplx::task_completion_event<long> tce;
        auto val = pplx::create_task(tce);

        std::atomic<long> v(0);

        auto t = val.then(
            [&](long x) {
                __LOG(error, " x is : " << x);
                v.exchange(x);
                tce.set(v);
            });

        // Start the task
        tce.set_exception(pplx::invalid_operation());

        // Wait for the lambda to finish running
        while (!t.is_done())
        {
            // Yield.
        }

        // Verify that the lambda did run
        __LOG(error, "v is : " << v << "  should be " << 0);

        // Wait for the task.
        try
        {
            t.wait();
        }
        catch (pplx::invalid_operation)
        {
        }
        catch (std::exception_ptr)
        {
            v = 1;
        }
        __LOG(error, "v is : " << v << "  should be " << 0);
    }
    {
        std::atomic<long> flag(0);

        pplx::task<void> t1([&flag]() {
            while (flag == 0)
                ;
            __LOG(error, "exit task");
        });

        flag.exchange(1L);
        t1.wait();
    }
    {
        __LOG(error, "last case");
        pplx::task_completion_event<long> tce;
        auto val = pplx::create_task(tce);
        val.then([](long x) {
            __LOG(error, "in the then function");
        });

        // Start the task
        tce.set_exception(pplx::invalid_operation());
        val.wait();

        // Wait for the lambda to finish running
        while (!val.is_done())
        {
            // Yield.
        }

        __LOG(error, "exit last case");
    }

    {
        pplx::task_completion_event<long> tce;
        auto val = pplx::create_task(tce);

        std::atomic<long> v(0);

        auto t = val.then(
            [&](long x) {
                __LOG(error, " x is : " << x);
                v.exchange(x);
                return x;
            });

        // Start the task
        tce.set_exception(pplx::invalid_operation());

        // Wait for the lambda to finish running
        while (!t.is_done())
        {
            // Yield.
        }

        // Verify that the lambda did run
        __LOG(error, "v is : " << v << "  should be " << 0);

        // Wait for the task.
        try
        {
            t.wait();
        }
        catch (pplx::invalid_operation)
        {
        }
        catch (std::exception_ptr)
        {
            v = 1;
        }
        __LOG(error, "v is : " << v << "  should be " << 0);
    }
#endif

    __LOG(error, "exit example");
}
