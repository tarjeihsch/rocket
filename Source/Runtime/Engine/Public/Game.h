#pragma once
#include <vector>

// Persistent game data seamlessly carried between map changes

class RViewport;
class RLocalPlayer;

class RGame
{
public:
    // Every LocalPlayer is tied to the duration of this game
    std::vector<RLocalPlayer*> LocalPlayers;

    // One viewport represents one view into game world
    RViewport* Viewport;
};