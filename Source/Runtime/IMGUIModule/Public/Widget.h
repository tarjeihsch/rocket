#pragma once

#include "Common/RHIDrawCommand.h"

#include <cstdint>
#include <memory>
#include <vector>

struct FRHIDrawCall;
struct ImDrawList;
class RPrimitive;

enum class EWidgetVisiblityState : uint8_t
{
    Hidden = 0,

    Visible,
};

class RWidget
{
public:
    RWidget();
    explicit RWidget(const char* InWidgetName);
    virtual ~RWidget() = default;

    virtual void Update(float DeltaTime);
    void Render(float DeltaTime);

    void AttachWidget(const std::shared_ptr<RWidget>& ChildWidget);

    template<typename TWidget>
    void AttachWidget();

protected:
    virtual void PreUpdate() {}
    virtual void UpdateImpl() {}

    void UpdatePanelDeferredImpl(const ImDrawList& DrawList);

protected:
    std::vector<std::shared_ptr<RWidget>> Children;

    FRHIDrawCall DrawCall;

    const char* WidgetName;

    EWidgetVisiblityState Visibility;

    int WindowFlags;
};

template <typename TWidget>
void RWidget::AttachWidget()
{
    AttachWidget(std::make_shared<TWidget>());
}