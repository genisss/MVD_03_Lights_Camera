//
//  Game.h
//  02-GameLoop
//
//  Copyright � 2018 Alun Evans. All rights reserved.
//
#pragma once
#include "includes.h"
#include "GraphicsSystem.h"

class Game
{
public:
	Game();
	void init();
	void update(float dt);

private:
	GraphicsSystem graphics_system_;
};
