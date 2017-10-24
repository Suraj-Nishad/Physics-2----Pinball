#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleSceneIntro.h"
#include "ModuleInput.h"
#include "ModuleTextures.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"

ModuleSceneIntro::ModuleSceneIntro(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	ray_on = false;
	sensed = false;
	allow_throw = false;
}

ModuleSceneIntro::~ModuleSceneIntro()
{}

// Load assets
bool ModuleSceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	App->renderer->camera.x = App->renderer->camera.y = 0;

	//Load Assets
	bonus_fx = App->audio->LoadFx("pinball/bonus.wav");
	boing = App->audio->LoadFx("pinball/boing.wav");
	paw = App->audio->LoadFx("pinball/paw.wav");
	background = App->textures->Load("pinball/background.png");
	sprites = App->textures->Load("pinball/sprites.png");
	ball_sprite = App->textures->Load("pinball/wheel.png");

	// Set up pinball board
	int Pinball_box_1[10] = {
		170, 390,
		170, 342, // Flipper 1
		51, 273,
		0, 193,
		371, 0,
	

	};	
	
	int Pinball_box_2[8] = {
		416, 30,
		384, 286,
		300, 342, // Flipper 2
		300, 390,
	};

	int Pinball_ball_throw[14] = {
		416, 30,
		455, 150,
		1135/2.5, 744/2.5,
		1192/2.5, 744/2.5,
		474, 150,
		435,0,
		371,0
	};

	int Wall[8] = {
		416, 0,
		416, 100,
		430, 100,
		430, 0,
	};


	for (int i = 0; i != 10; i++)
	{
		Pinball_box_1[i] = Pinball_box_1[i] * 2.5;
	}
	
	for (int i = 0; i != 8; i++)
	{
		Pinball_box_2[i] = Pinball_box_2[i] * 2.5;
	}


	for (int i = 0; i != 14; i++)
	{
		Pinball_ball_throw[i] = Pinball_ball_throw[i] * 2.5;
	}

	for (int i = 0; i != 8; i++)
	{
		Wall[i] = Wall[i] * 2.5;
	}

	Lpinball = App->physics->CreateChain(0, 0, Pinball_box_1, 10, b2_staticBody, -1, true);

	Rpinball = App->physics->CreateChain(0, 0, Pinball_box_2, 8, b2_staticBody,-1, true);

	Bpinball = App->physics->CreateChain(0, 0, Pinball_ball_throw, 14, b2_staticBody,-1, true);

	Lflipper = App->physics->CreateRectangle(170 * 2.5, (352 * 2.5), 180, 35,b2_dynamicBody,-1);

	Rflipper = App->physics->CreateRectangle(300 * 2.5, (352 * 2.5), 180, 35, b2_dynamicBody ,-1);

	wall = App->physics->CreateChain(0, 0, Wall, 8, b2_staticBody,-1, true);

	//Set up bouncers
	bouncer1 = App->physics->CreateCircle(760, 279, 25, b2_staticBody,1);
	bouncer2 = App->physics->CreateCircle(767, 362, 25, b2_staticBody, 1);
	bouncer3 = App->physics->CreateCircle(867, 320, 25, b2_staticBody, 1);

	//Set up barrels
	int Barrels_1[14] = {
		276,647,
		326,641, 
		381,634,
		391,681,
		425,705,
		420,764,
		275,696

	};

	barrels_1 = App->physics->CreateChain(0, 0, Barrels_1, 14, b2_staticBody, 2, false, 2);

	int Barrels_2[10] = {
		756, 698,
		874, 634,
		901, 662,
		898,699,
		758,753
	};

	barrels_2 = App->physics->CreateChain(0, 0, Barrels_2, 10, b2_staticBody, 2, false, 2);

	//Set up cowboys
	for (int i = 0; i <= 4; i++) // First row
	{
		cowboys[i] = App->physics->CreateRectangle(502 + i*35, 440-i*20, 30, 80, b2_staticBody, 3);
		cowboys[i]->alive = true;
	}

	for (int i = 5; i < 11; i++) //Second row
	{
		cowboys[i] = App->physics->CreateRectangle(440 + (i-5) * 35, 396 - (i-5) * 20, 30, 80, b2_staticBody, 3);
		cowboys[i]->alive = true;
	}
	
	//Set up joints
	b2RevoluteJointDef first_joint;
	b2RevoluteJointDef second_joint;
	
	first_joint.bodyA = Lflipper->body; // Pala
	first_joint.bodyB = Lpinball->body; // Tablero
	first_joint.collideConnected = false;
	first_joint.localAnchorA.Set(PIXEL_TO_METERS(-50), PIXEL_TO_METERS(0));
	first_joint.localAnchorB.Set(PIXEL_TO_METERS(170 * 2.5), PIXEL_TO_METERS((352* 2.5)));
	first_joint.enableLimit = true;
	first_joint.lowerAngle = -30 * DEGTORAD;
	first_joint.upperAngle = 30 * DEGTORAD;

	second_joint.bodyA = Rflipper->body; // Pala
	second_joint.bodyB = Rpinball->body; // Tablero
	second_joint.collideConnected = false;
	second_joint.localAnchorA.Set(PIXEL_TO_METERS(50), PIXEL_TO_METERS(0));
	second_joint.localAnchorB.Set(PIXEL_TO_METERS(300 * 2.5), PIXEL_TO_METERS((352 * 2.5)));
	second_joint.enableLimit = true;
	second_joint.lowerAngle = -30 * DEGTORAD;
	second_joint.upperAngle = 30 * DEGTORAD;

	App->physics->world->CreateJoint(&first_joint);
	App->physics->world->CreateJoint(&second_joint);

	//Set up sensors 
	ball_throw = App->physics->CreateRectangleSensor(1162, 738, 90, 35, 0);
	ball_throw->listener = this;

	wall_sensor = App->physics->CreateRectangleSensor(400 * 2.5, (30 * 2.5), 10, 300, 1);
	wall_sensor->listener = this;

	//Set up first ball
	ball = App->physics->CreateCircle(1164, 633, 18, b2_dynamicBody,0, true);
	ball->listener = this;



	return ret;
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");
	App->textures->Unload(background);
	App->textures->Unload(sprites);
	App->textures->Unload(ball_sprite);

	return true;
}

// Update: draw background
update_status ModuleSceneIntro::Update()
{
	//Ball should allways have 0 inclination
	ball->body->SetTransform(b2Vec2(ball->body->GetPosition().x, ball->body->GetPosition().y), 0);

	//Wall collision change management
	if (wall_collision)
	{
		wall->body->SetActive(true);
	}
	else
	{
		wall->body->SetActive(false);
	}




	for (int i = 0; i < 11; i++) //JUST A TEST
	{
		if(!cowboys[i]->alive)
		cowboys[i]->body->SetActive(false);
	}




	//Ball restart position
	if (App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
	{ 
		ball->body->SetTransform(b2Vec2(PIXEL_TO_METERS(1164), PIXEL_TO_METERS(633)), 0);
		ball->body->SetLinearVelocity(b2Vec2(0, 0));
	}



	//Pallets controller
	if (App->input->GetKey(SDL_SCANCODE_Q) == KEY_REPEAT)
		Lflipper->body->ApplyAngularImpulse(-190, true);
	else
		Lflipper->body->ApplyAngularImpulse(1, true);

	if (App->input->GetKey(SDL_SCANCODE_E) == KEY_REPEAT)
		Rflipper->body->ApplyAngularImpulse(190, true);
	else
		Rflipper->body->ApplyAngularImpulse(-1, true);


	if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
		idk2 -= 1;

	if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
		idk -= 1;

	if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
		idk += 1;

	if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
		idk2 += 1;

	if (App->input->GetKey(SDL_SCANCODE_O) == KEY_DOWN)
	{
		if (idkk)
			idkk = false;
		else
			idkk = true;
	}


	//Ball thrower
	if (allow_throw && App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
	{
		ball->body->ApplyForceToCenter(b2Vec2(0, -10000), true);
	}
	allow_throw = false;
	// Prepare for raycast ------------------------------------------------------


	//Blit everything
	App->renderer->Blit(background, 0, 0, 1.66);

	if (idkk)
		idkk;

	App->renderer->Blit(sprites, 589, 271, 1.66, &N);
	App->renderer->Blit(sprites, 557, 288, 1.66, &O2);
	App->renderer->Blit(sprites, 525, 305, 1.66, &O);
	App->renderer->Blit(sprites, 489, 323, 1.66, &L);
	App->renderer->Blit(sprites, 453, 341, 1.66, &A);
	App->renderer->Blit(sprites, 416, 359, 1.66, &S2);

	int x, y;
	
	ball->GetPosition(x, y, true);
	App->renderer->Blit(ball_sprite,x,y, 0.7);
	
	return UPDATE_CONTINUE;
}

//If there is a sensor, body a is the sensor

//If there is no sensor, body b is the ball
void ModuleSceneIntro::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
	//App->audio->PlayFx(bonus_fx);
	if (bodyA->body->GetFixtureList()->IsSensor()) //Sensors management
	{
		if (bodyA->type == 0)  //Ball throwing position
		{
			allow_throw = true;
			wall_collision = false;
		}
		else if (bodyA->type == 1) //Temporal barrier
		{
			wall_collision = true;
		}
	}
	else                                          //Body management
	{
		if (bodyA->type == 1) //bouncer
		{
			iPoint ball_position;
			iPoint bouncer_position;
			bodyA->GetPosition(bouncer_position.x, bouncer_position.y);
 			bodyB->GetPosition(ball_position.x, ball_position.y);


			b2Vec2 force((ball_position.x - bouncer_position.x), (ball_position.y - bouncer_position.y));
			float32 Length = sqrt(pow(force.x, 2) + pow(force.y, 2));
			force.x = (force.x / Length) * 200;
			force.y = (force.y / Length) * 200;

            App->audio->PlayFx(bonus_fx);
			bodyB->body->ApplyLinearImpulse(force,b2Vec2(0,0), true);
		}
		if (bodyA->type == 2)
		{
			App->audio->PlayFx(boing);
		}
		if (bodyA->type == 3)
		{
			App->audio->PlayFx(paw);
			//bodyA->body->SetActive(false); Program dies for unknown reason, ask sensei
			bodyA->alive = false;
		}
	}

}
