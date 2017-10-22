#pragma once
#include "Module.h"
#include "p2List.h"
#include "p2Point.h"
#include "Globals.h"

class PhysBody;

class ModuleSceneIntro : public Module
{
public:
	ModuleSceneIntro(Application* app, bool start_enabled = true);
	~ModuleSceneIntro();

	bool Start();
	update_status Update();
	bool CleanUp();
	void OnCollision(PhysBody* bodyA, PhysBody* bodyB);

public:


	p2List<PhysBody*> circles;


	// Pinball bodies
	PhysBody* Lpinball;
	PhysBody* Rpinball;
	PhysBody* Bpinball;
	PhysBody* Lflipper;
	PhysBody* Rflipper;
	PhysBody* wall;

	//Pinball sensors
	PhysBody* ball_throw;
	PhysBody* wall_sensor;

	//Bools to controls cositas
	bool allow_throw;


	bool sensed;

	uint bonus_fx;
	p2Point<int> ray;
	bool ray_on;
};
