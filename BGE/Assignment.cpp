#include "Assignment.h"
#include "Content.h"
#include "VectorDrawer.h"
#include "LazerBeam.h"
#include "SnowEffect.h"
#include "Utils.h"


using namespace BGE;

Assignment::Assignment(void)
{
	elapsed = 10000;
	ySpeed = 5.0f;

}


Assignment::~Assignment(void)
{
}

bool Assignment::Initialise()
{
	std::shared_ptr<GameComponent> ground = make_shared<Ground>();
	Attach(ground);	

	snow =make_shared<SnowEffect>();
	snow->position.y = 100;
	Attach(snow);

	riftEnabled = false;
	fullscreen = false;
	width = 1000;
	height = 800;
	
	t = 0.0f;
	success = false;
	bond = true;
		// Set up the collision configuration and dispatcher
    collisionConfiguration = new btDefaultCollisionConfiguration();
    dispatcher = new btCollisionDispatcher(collisionConfiguration);
 
    // The world.
	btVector3 worldMin(-1000,-1000,-1000);
	btVector3 worldMax(1000,1000,1000);
	broadphase = new btAxisSweep3(worldMin,worldMax);
	solver = new btSequentialImpulseConstraintSolver();
	dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher,broadphase,solver,collisionConfiguration);
    dynamicsWorld->setGravity(btVector3(0,-9,0));

	physicsFactory2 = make_shared<PhysicsFactory>(dynamicsWorld);
	
	physicsFactory2->CreateGroundPhysics();
	watcher = make_shared<PhysicsController>();
	watcher->orientation = glm::angleAxis(90.0f, watcher->up); 
	watcher = physicsFactory2->CreateFromModel("buddha",glm::vec3(0,60,0),glm::quat(),glm::vec3(5,5,5));
	watcher->position =glm::vec3(0,60,0);
	
	
	for (int i = 0 ; i < NUM_FOUNTAINS ; i ++)
	{
		particleTheta = ((glm::pi<float>() * 2.0f) / NUM_FOUNTAINS) * i;
		watcherPartical = make_shared<FountainEffect>(500);
		if (i % 2 == 0)
		{
			watcherPartical->diffuse = glm::vec3(0,0.02*i,RandomFloat()*i);
		}
		else
		{
			watcherPartical->diffuse = glm::vec3(0,RandomFloat()*i,0.05*i);
		}

		watcherPartical->position = glm::vec3(0, 0, 0);
		
		watcherParticals.push_back(watcherPartical);
		Attach(watcherPartical);
	}

	physicsFactory2->CreateWall(glm::vec3(0,0,0), 1, 10);
	physicsFactory2->CreateKineticBox(5,3,30, glm::vec3(-25,1,0),glm::quat());
	physicsFactory2->CreateKineticBox(30,3,5, glm::vec3(0,1,25),glm::quat());
	physicsFactory2->CreateKineticBox(5,3,30, glm::vec3(25,1,0),glm::quat());
	physicsFactory2->CreateKineticBox(30,3,5, glm::vec3(0, 1 ,-25),glm::quat());


	
	
	ship1 = make_shared<PhysicsController>();
	ship1->position = glm::vec3(50, 5, 50);
	ship1->orientation = glm::quat();
	ship1 = physicsFactory2->CreateSphere(3,ship1->position,ship1->orientation);
	ship1->rigidBody->setGravity(btVector3(0,-10,0));

	
	bomb = make_shared<PhysicsController>();
	bomb = physicsFactory2->CreateSphere(1,glm::vec3(150,50,150),glm::quat());
	bomb->rigidBody->setGravity(btVector3(0,-100,0));
	bomb->diffuse = glm::vec3(0,0,0);
	
	pillar = make_shared<PhysicsController>();
	pillar = physicsFactory2->CreateKineticBox(2,50,2, glm::vec3(100,25,-100),glm::quat());
	pillar->diffuse = glm::vec3(1,1,1);

	btPoint2PointConstraint * pillarBallhinge = new btPoint2PointConstraint(*ship1->rigidBody, *pillar->rigidBody, btVector3(0,0,0),btVector3(0,29,0));
    dynamicsWorld->addConstraint(pillarBallhinge);

	chain = make_shared<PhysicsController>();
	chain = physicsFactory2->CreateBox(0.1, 15, 0.1, glm::vec3(100,50,100), glm::quat());

	btPoint2PointConstraint  * ballChainhinge = new btPoint2PointConstraint(*ship1->rigidBody, *chain->rigidBody, btVector3(3,0,3),btVector3(0,8,0));
	dynamicsWorld->addConstraint(ballChainhinge);

	
	hinge = new btPoint2PointConstraint(*bomb->rigidBody ,*chain->rigidBody,  btVector3(0,1,0),btVector3(0,-8,0));
	dynamicsWorld->addConstraint(hinge);
	
	blackMagic = make_shared<FountainEffect>(500);
	blackMagic->diffuse = glm::vec3(0,0,0);
	chain->Attach(blackMagic);
		
	spark = make_shared<FountainEffect>(500);
	spark->diffuse = glm::vec3(1,0,0);
	bomb->Attach(spark);
	Game::Initialise();

	camera->GetController()->position =  glm::vec3(150,100,-150);
	
	
	
	
	return true;
}

void Assignment::Update(float timeDelta)
{	
	 dynamicsWorld->stepSimulation(timeDelta,100);
	Assignment::Watch(camera, timeDelta);
	
	static float active = 1.0f ;
	if(elapsed > active)
	{
		if(bond == false)
		{
			
			bomb->rigidBody->clearForces();
			PrintText("Attach Bomb press B");
			if (keyState[SDL_SCANCODE_B])
			{
			
				dynamicsWorld->addConstraint(hinge);
				bond = true;
				elapsed = 0.0f;
			}
		}
		else
		{
			ship1->rigidBody->setAngularVelocity(btVector3(0,30,0));
			PrintText("Detach and press B");
		
				
			if (keyState[SDL_SCANCODE_B])
			{
				dynamicsWorld->removeConstraint(hinge);
				bond = false;
				elapsed = 0.0f;
			}
		}

	}
	
	particleTheta += timeDelta;
	if (particleTheta >= glm::pi<float>() * 2.0f)
	{
		particleTheta = 0.0f;
	}
	
	

	if(success == true)
	{
		
		PrintText("Mission Accomplished");
		for (int i = 0 ; i < watcherParticals.size() ; i ++)
		{
			watcherParticals[i]->diffuse = glm::vec3(1,0,0);
			ySpeed = 20.0f;
			watcherParticals[i]->position.z =  glm::sin(particleTheta) * -30;
			watcherParticals[i]->position.x =    glm::cos(particleTheta) * 30;
			watcherParticals[i]->position.y += timeDelta * ySpeed;
			if (watcherParticals[i]->position.y > watcher->position.y +25)
			{
				ySpeed = -ySpeed;
				watcherParticals[i]->position.y = watcher->position.y +25;
			}

			if (watcherParticals[i]->position.y < watcher->position.y -25)
			{
				ySpeed = -ySpeed;
				watcherParticals[i]->position.y = watcher->position.y -25;
			}
			if (watcherParticals[i]->position.y < 0)
			{
				ySpeed = -ySpeed;
				watcherParticals[i]->position.y = 0;
			}
		}


		

	}
	else
	{
		PrintText("Destroy The Watcher's tower");
		glm::vec3 bombDistance = bomb->position - watcher->position;
		float distLength = glm::length(bombDistance);
		if (distLength < 50)
		{
			glm::vec3 bombDistance = bomb->position - (watcher->position+glm::vec3(5,5,-5));
			bombDistance = glm::normalize(bombDistance);
			physicsFactory3 = make_shared<PhysicsFactory>(dynamicsWorld);
			shared_ptr<PhysicsController> defenceObject = physicsFactory3->CreateSphere(0.5,watcher->position+glm::vec3(5,5,-5) , watcher->orientation);
			defenceObject->rigidBody->setGravity(btVector3(0,-30,0));
			defenceObject->rigidBody->applyCentralForce(GLToBtVector(bombDistance) * 50000);
		}

		if (distLength < 5 || watcher->position.y <5)
		{
			Assignment::Detonate();
			success = true;
		}
		for (int i = 0 ; i < watcherParticals.size() ; i ++)
		{
			if (i % 2 == 0)
			{
				watcherParticals[i]->position.x = watcher->position.x + glm::sin(particleTheta) * -30;
				watcherParticals[i]->position.z =  watcher->position.z - glm::cos(particleTheta) * 30;
			}
			else
			{
				watcherParticals[i]->position.z = watcher->position.z + glm::sin(particleTheta) * -30;
				watcherParticals[i]->position.x =  watcher->position.x - glm::cos(particleTheta) * 30;
			}
			watcherParticals[i]->position.y += timeDelta * ySpeed;
			if (watcherParticals[i]->position.y > watcher->position.y +25)
			{
				ySpeed = -ySpeed;
				watcherParticals[i]->position.y = watcher->position.y +25;
			}

			if (watcherParticals[i]->position.y < watcher->position.y -25)
			{
				ySpeed = -ySpeed;
				watcherParticals[i]->position.y = watcher->position.y -25;
			}
			if (watcherParticals[i]->position.y < 0)
			{
				ySpeed = -ySpeed;
				watcherParticals[i]->position.y = 0;
			}
		}
		
	
	}
	elapsed += timeDelta;
	blackMagic->position = chain->position - glm::vec3(0,8,0);
	spark->position = bomb->position;
	Game::Update(timeDelta);
}

void Assignment::Detonate()
{	for (int i = 0; i<10;i++)
	{
		glm::vec3 pos = bomb->position;
		glm::vec3 pos2 = glm::vec3(0,0,0);
		glm::quat q(RandomFloat(), RandomFloat(), RandomFloat(), RandomFloat());
		glm::normalize(q);
		physicsFactory = make_shared<PhysicsFactory>(dynamicsWorld);
		shared_ptr<PhysicsController> detonationBox = physicsFactory->CreateBox(0.5,0.5,0.5, pos, q);
		shared_ptr<PhysicsController> detonationBox2 = physicsFactory->CreateBox(0.5,0.5,0.5, pos2, q);
		detonationForce = 10000.0f;
		detonationBox->rigidBody->applyCentralForce(GLToBtVector(glm::vec3(RandomFloat(), RandomFloat(), RandomFloat())) * detonationForce);
		detonationBox2->rigidBody->applyCentralForce(GLToBtVector(glm::vec3(0, RandomFloat(), 0)) * detonationForce);
		elapsed = 0.0f;
		
	}
	
	
}
void Assignment::Watch(shared_ptr<GameComponent> x, float timeDelta)
{
	
		fromQuaternion = x->GetController()->orientation;
		glm::vec3 toShip = ship1->position - x->GetController()->position;
		toShip = glm::normalize(toShip);
		glm::vec3 axis = glm::cross(GameComponent::basisLook,toShip);
		axis = glm::normalize(axis);

		float theta = glm::acos(glm::dot(toShip, GameComponent::basisLook));
		
		toQuaternion = glm::angleAxis(glm::degrees(theta), axis);

		x->GetController()->orientation = glm::mix(fromQuaternion,toQuaternion, t);
		t += timeDelta;

		if (t > 1.0f)
		{
			
			t = 0.0f;
		}
	
}

