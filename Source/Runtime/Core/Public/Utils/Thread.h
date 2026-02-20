#pragma once

#include <cstdint>
#include <thread>
#include <atomic>
#include <type_traits>

#include "Platform.h"

enum class ThreadState
{
    Stopped,
    Starting,
    Running,
    Stopping
};

class Thread
{
public:
    explicit Thread(const uint32_t P_Id, const char* P_Name = "Thread")
        : M_Id(P_Id)
        , M_Name(P_Name)
        , M_State(ThreadState::Stopped)
    {
    }

    Thread(const Thread&) = delete;
    Thread& operator=(const Thread&) = delete;

    void Start()
    {
        if (M_State.load(std::memory_order_relaxed) != ThreadState::Stopped)
        {
            return;
        }

        M_State.store(ThreadState::Starting, std::memory_order_release);
        M_Thread = std::thread([this]
        {
            Platform::SetThreadName(M_Name);

            M_State.store(ThreadState::Running, std::memory_order_release);
            OnStart();
            Run();
            OnStop();
            M_State.store(ThreadState::Stopped, std::memory_order_release);
        });
    }

    void Stop()
    {
        while (M_State.load(std::memory_order_acquire) == ThreadState::Starting)
        {
            std::this_thread::yield();
        }

        ThreadState Current = M_State.load(std::memory_order_acquire);
    
        if (Current == ThreadState::Stopped || Current == ThreadState::Stopping)
        {
            return;
        }

        M_State.store(ThreadState::Stopping, std::memory_order_release);

        if (M_Thread.joinable())
        {
            M_Thread.join();
        }
    }

    inline const uint32_t GetId() const
    {
        return M_Id;
    }

    inline const char* GetName() const
    {
        return M_Name;
    }

    inline bool IsRunning() const 
    {
        return M_State.load(std::memory_order_acquire) == ThreadState::Running;
    }

protected:
    virtual void Run() = 0;
    
    virtual void OnStart() {};
    virtual void OnStop() {};

private:
    const uint32_t              M_Id;
    const char*                 M_Name;
    std::atomic<ThreadState>    M_State;
    std::thread                 M_Thread;
};

template<typename Thread_T>
class ThreadPool
{
    static_assert(std::is_base_of_v<Thread, Thread_T>, "T must derive from Thread class");

public:
    ThreadPool(const uint32_t P_Thread_Count)
    {
        Resize(P_Thread_Count);
    }

    ~ThreadPool()
    {
        Resize(0);
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    void Resize(const uint32_t P_Thread_Count)
    {
        const uint32_t Current_Thread_Count = static_cast<uint32_t>(M_Thread_Data.size());
        const uint32_t Max_Thread_Count = std::thread::hardware_concurrency();
        const uint32_t New_Thread_Count = Math::Min(P_Thread_Count, Max_Thread_Count);

        if (New_Thread_Count < Current_Thread_Count)
        {
            for (uint32_t Index = New_Thread_Count; Index < Current_Thread_Count; ++Index)
            {
                std::unique_ptr<Thread_T>& Old_Thread = M_Thread_Data[Index];
                Old_Thread->Stop();
            }

            M_Thread_Data.resize(New_Thread_Count);
        }
        else
        {
            M_Thread_Data.reserve(New_Thread_Count);

            for (uint32_t Index = Current_Thread_Count; Index < New_Thread_Count; ++Index)
            {
                std::unique_ptr<Thread_T>& New_Thread = M_Thread_Data.emplace_back(std::make_unique<Thread_T>(Index));
                New_Thread->Start();
            }
        }
    }

private:
    std::vector<std::unique_ptr<Thread_T>> M_Thread_Data;
};