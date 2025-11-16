#pragma once

#include <cstdint>

enum class EPanelVisiblityState : uint8_t
{
    Hidden = 0,

    Visible,
};

class RPanel
{
public:
    explicit RPanel(const char* InPanelName);
    virtual ~RPanel() = default;

    void UpdatePanel();

protected:
    // Implementation entrypoint as per child class
    virtual void UpdatePanelImpl() = 0;

protected:
    const char* PanelName;

    EPanelVisiblityState Visibility;
};