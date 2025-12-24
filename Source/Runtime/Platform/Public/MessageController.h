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

    virtual bool OnMouseDown(RWindow* Window, Button Button)
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

    virtual bool OnWindowSizeChanged(RWindow* Window, int32_t Width, int32_t Height, bool bWasMinimized)
    { 
        return false; 
    }
    
    virtual void OnResizingWindow(RWindow* Window)
    {
    }
    
    virtual void OnWindowClose(RWindow* Window)
    {
    }
    
    virtual void OnWindowMoved(RWindow* Window, int32_t X, int32_t Y)
    {
    }
};