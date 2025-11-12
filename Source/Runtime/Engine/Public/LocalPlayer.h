#pragma once

#include <cstdint>

class RPlayer;

class RLocalPlayer final
{
public:
    RLocalPlayer();

    RPlayer* GetControlledPlayer() const;

private:
    RPlayer* ControlledPlayer;
};