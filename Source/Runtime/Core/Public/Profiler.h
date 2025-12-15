#ifndef PROFILER_H
#define PROFILER_H

#include <chrono>

struct Event
{
    Event()
    {
        Start();
    }

    ~Event()
    {
        End();
    }

    void Start()
    {
        start = std::chrono::high_resolution_clock().now();
    }

    void End()
    {
        end = std::chrono::high_resolution_clock().now();
    }

    const char* name;
    const char* description;
    std::chrono::time_point start;
    std::chrono::time_point end;
};

/*
* Event object streamed as byte buffer to output file during flush.
* A parser is required to read corresponding bytes and interpret and visualize the data correctly.
*
*/

struct EventBuffer
{
    void Push(Event&& event)
    {
        data.push_back(std::move(event));
    }

    void Flush()
    {
        for (Event& event : data)
        {

        }
    }

    std::vector<Event> data;
};

#define SCOPE(name, description)

#endif
