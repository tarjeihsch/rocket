#pragma once

#include <cstdint>
#include <memory>
#include <vector>

enum class EGUIVisibilityMode : uint8_t
{
    Hidden = 0,

    HiddenSelf = 1,

    Visible = 2,

    VisibleSelf = 3
};

enum class EGUIState : uint8_t
{

};

class RGUI
{
public:
    RGUI();
    explicit RGUI(const char* InName);
    virtual ~RGUI() = default;

    virtual void Update(float DeltaTime);

    void AttachWidget(const std::shared_ptr<RGUI>& ChildWidget);
    void DetachWidget(const std::shared_ptr<RGUI>& ChildWidget);

protected:
    std::vector<std::shared_ptr<RGUI>> Children;

    const char* Name;

private:
    EGUIVisibilityMode VisibilityMode;
};