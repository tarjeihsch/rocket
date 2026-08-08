#pragma once

struct Event;

class EventSink
{
public:
    virtual ~EventSink() = default;

    virtual void Enqueue(const Event& event) = 0;
};