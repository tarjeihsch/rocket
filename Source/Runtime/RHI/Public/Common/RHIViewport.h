#pragma once

class ICamera;

class IRHIViewport
{
public:
    virtual ~IRHIViewport() = default;

    virtual void Init();
    virtual void Shutdown();

    virtual void AttachCamera(ICamera* Camera);
    virtual void DetachCamera();
    virtual ICamera* GetCachedCamera() const;
};