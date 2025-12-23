#pragma once

#include <memory>

#include "Event.h"

class Window;

/*  */
class MessageController
{
public:
    virtual ~MessageController() = default;

    virtual void OnExit()
    {
    }

    virtual void OnWindowFocusGained() 
    {
    }

    virtual void OnWindowFocusLost()
    {
    }

    virtual bool OnKeyChar(uint32_t CharCode, bool bIsRepeat) 
    { 
        return false; 
    }
    
    virtual bool OnKeyDown(Key key, uint32_t CharCode, bool bIsRepeat) 
    { 
        return false; 
    }
    
    virtual bool OnKeyUp(Key key, uint32_t CharCode, bool bIsRepeat) 
    { 
        return false; 
    }

    virtual bool OnMouseMove(int32_t x, int32_t y)
    { 
        return false; 
    }
    
    virtual bool OnRawMouseMove(int32_t DeltaX, int32_t DeltaY) 
    { 
        return false; 
    }

    virtual bool OnMouseDown(const std::shared_ptr<Window>& Window, Button Button) 
    { 
        return false; 
    }
    
    virtual bool OnMouseUp(Button Button) 
    { 
        return false; 
    }
    
    virtual bool OnMouseWheel(float Delta) 
    { 
        return false; 
    }

    virtual bool OnWindowSizeChanged(const std::shared_ptr<Window>& Window, int32_t Width, int32_t Height, bool bWasMinimized) 
    { 
        return false; 
    }
    
    virtual void OnResizingWindow(const std::shared_ptr<Window>& Window) 
    {
    }
    
    virtual void OnWindowClose(const std::shared_ptr<Window>& Window) 
    {
    }
    
    virtual void OnWindowMoved(const std::shared_ptr<Window>& Window, int32_t X, int32_t Y) 
    {
    }
};