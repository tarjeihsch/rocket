#include "Utils/Thread.h"

#if 0

std::atomic<uint32_t> G_Counter;

class GameThread : public Thread
{
public:
    explicit GameThread(uint32_t P_Id)
        : Thread(P_Id, "GameThread")
    {
    }

    virtual void Run() override
    {
        std::printf("Thread: %s %d\n", GetName(), GetId());

        while (IsRunning())
        {
            Counter++;

            if ((Counter % 100000000) == 0)
            {
                std::printf("Lap: %s # %d\n", GetName(), GetId());

                G_Counter.fetch_add(1);
            }
        }
    }

    uint32_t Counter = 0;
};

void Test_ThreadPool()
{
    ThreadPool<GameThread> Thread_Pool(8);

    for (;;)
    {
        if (G_Counter.load(std::memory_order_relaxed) == 100)
        {
            break;
        }
    }
}

#endif

static uint64_t G_Frame_Counter = 0;

int main()
{
    for (;;)
    {
        G_Frame_Counter++;
    }
}