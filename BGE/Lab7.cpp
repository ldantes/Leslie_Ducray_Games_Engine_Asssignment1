#include "Lab7.h"
#include "Content.h"
#include "VectorDrawer.h"
#include "LazerBeam.h"
#include "FountainEffect.h"

using namespace BGE;

Lab7::Lab7(void)
{
	elapsed = 10000;
}


Lab7::~Lab7(void)
{
}

bool Lab7::Initialise()
{
	std::shared_ptr<GameComponent> ground = make_shared<Ground>();
	Attach(ground);	

	ship1 = make_shared<GameComponent>();
	ship1->Attach(Content::LoadModel("fish", glm::rotate(glm::mat4(1), 270.0f, glm::vec3(0,1,0))));
	ship1->position = glm::vec3(-10, 2, -10);
	ship1->Attach(make_shared<VectorDrawer>());
	Attach(ship1);

	ship2 = make_shared<GameComponent>();

	ship2->Attach(Content::LoadModel("fish", glm::rotate(glm::mat4(1), 90.0f, glm::vec3(1,1,0))));
	ship2->Attach(make_shared<VectorDrawer>());
	ship2->diffuse= glm::vec3(1.0f,0.0f,0.0f);
	ship2->specular = glm::vec3(1.2f, 1.2f, 1.2f);

	ship2->position = glm::vec3(10, 2, -10);
	Attach(ship2);

	riftEnabled = false;
	fullscreen = false;
	width = 800;
	height = 600;

	slerping = false;
	t = 0.0f;

	Game::Initialise();

	camera->GetController()->position = glm::vec3(0, 4, 20);
	return true;
}

void Lab7::Update(float timeDelta)
{	
	// Movement of ship2
	if (keyState[SDL_SCANCODE_UP])
	{
		ship2->Walk(speed * timeDelta);
	}
	if (keyState[SDL_SCANCODE_DOWN])
	{
		ship2->Walk(-speed * timeDelta);
	}
	if (keyState[SDL_SCANCODE_LEFT])
	{
		ship2->Yaw(timeDelta * speed * speed);
	}
	if (keyState[SDL_SCANCODE_RIGHT])
	{
		ship2->Yaw(-timeDelta * speed * speed);
	}

	if (keyState[SDL_SCANCODE_O])
	{
		ship2->Fly(timeDelta * speed);
	}

	if (keyState[SDL_SCANCODE_L])
	{
		if(ship2->position.y>2)
		{
		ship2->Fly(-timeDelta * speed);
		}
	}

	// Put code for ship1 here!!!
	if (keyState[SDL_SCANCODE_SPACE] && !slerping)
	{
		slerping = true;

		fromQuaternion = ship1->orientation;
		glm::vec3 toShip = ship2->position - ship1->position;
		toShip = glm::normalize(toShip);
		glm::vec3 axis = glm::cross(GameComponent::basisLook,toShip);
		axis = glm::normalize(axis);

		float theta = glm::acos(glm::dot(toShip, GameComponent::basisLook));
		
		toQuaternion = glm::angleAxis(glm::degrees(theta), axis);
		
		
		/*glm::quat w;
		w.x = toShip.x;
		w.y = toShip.y;
		w.z = toShip.z;
		w.w = 0;

		glm::quat qInv;
		qInv.x = -q.x;
		qInv.y = -q.y;
		qInv.z = -q.z;

		w = q*w*qInv;
		toShip.x =w.x;
		toShip.y =w.y;
		toShip.z =w.z;*/
		
	}
	if(keyState[SDL_SCANCODE_F])
	{
	shared_ptr<LazerBeam> lazer = make_shared<LazerBeam>();
			lazer->position = ship1->position;
			lazer->look = ship1->look;
			Attach(lazer);
			elapsed = 0.0f;
	}
	if (slerping)
	{
		ship1->orientation = glm::mix(fromQuaternion,toQuaternion, t);
		t += timeDelta;

		if (t > 1.0f)
		{
			slerping=false;
			t = 0.0f;
		}
	}

	// End code for ship 1	
	Game::Update(timeDelta);

}
