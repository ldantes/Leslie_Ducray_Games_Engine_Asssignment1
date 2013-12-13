#include "Assignment.h"
#include "Content.h"
#include "VectorDrawer.h"
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

	//Create the watcher on top of the tower.
	watcher = make_shared<PhysicsController>();
	watcher->orientation = glm::angleAxis(90.0f, watcher->up); 
	watcher = physicsFactory2->CreateFromModel("buddha",glm::vec3(0,60,0),glm::quat(),glm::vec3(5,5,5)); //Load model and place
	watcher->position =glm::vec3(0,60,0);
	
	// The particles around the watcher's tower are declared
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

	gameCount =1;
	// The watcher's tower and the 4 kinematic barriers are created and placed
	physicsFactory2->CreateWall(glm::vec3(0,0,0), gameCount, 10);
	physicsFactory2->CreateKinematicBox(5,3,30, glm::vec3(-25,1,0),glm::quat());
	physicsFactory2->CreateKinematicBox(30,3,5, glm::vec3(0,1,25),glm::quat());
	physicsFactory2->CreateKinematicBox(5,3,30, glm::vec3(25,1,0),glm::quat());
	physicsFactory2->CreateKinematicBox(30,3,5, glm::vec3(0, 1 ,-25),glm::quat());

	//Create the spinning mechanism	
	spinBall = make_shared<PhysicsController>();
	spinBall->position = glm::vec3(50, 5, 50);
	spinBall->orientation = glm::quat();
	spinBall = physicsFactory2->CreateSphere(3,spinBall->position,spinBall->orientation);
	spinBall->rigidBody->setGravity(btVector3(0,-10,0));

	// create bomb
	bomb = make_shared<PhysicsController>();
	bomb = physicsFactory2->CreateSphere(1,glm::vec3(150,50,150),glm::quat());
	bomb->rigidBody->setGravity(btVector3(0,-75,0));
	bomb->diffuse = glm::vec3(0,0,0);
	// create pillar
	pillar = make_shared<PhysicsController>();
	pillar = physicsFactory2->CreateKinematicBox(2,50,2, glm::vec3(100,25,-100),glm::quat());
	pillar->diffuse = glm::vec3(1,1,1);
	// joints for pillar to spinBall
	// 2 * ball in socket/point to point joints
	btPoint2PointConstraint * pillarBallball2chain = new btPoint2PointConstraint(*spinBall->rigidBody, *pillar->rigidBody, btVector3(0,1.5,0),btVector3(0,30.5,0));
	btPoint2PointConstraint * pillarBallball2chain2 = new btPoint2PointConstraint(*spinBall->rigidBody, *pillar->rigidBody, btVector3(0,-1.5,0),btVector3(0,27.5,0));
    dynamicsWorld->addConstraint(pillarBallball2chain);
	dynamicsWorld->addConstraint(pillarBallball2chain2);
	// Create solidBody link (chain) 
	chain = make_shared<PhysicsController>();
	chain = physicsFactory2->CreateBox(0.1, 15, 0.1, glm::vec3(100,50,100), glm::quat());

	// SpinBall to chain constraint - point to point
	btPoint2PointConstraint  * ballChainball2chain = new btPoint2PointConstraint(*spinBall->rigidBody, *chain->rigidBody, btVector3(3,0,0),btVector3(0,8,0));
	dynamicsWorld->addConstraint(ballChainball2chain);

	//contraint connecting the 'bomb' to the 'chain'
	ball2chain = new btPoint2PointConstraint(*bomb->rigidBody ,*chain->rigidBody,  btVector3(0,1,0),btVector3(0,-8,0));
	dynamicsWorld->addConstraint(ball2chain);
	// fountain effect attached to the end of the 'chain'
	blackMagic = make_shared<FountainEffect>(500);
	blackMagic->diffuse = glm::vec3(0,0,0);
	chain->Attach(blackMagic);
	// fountain effect attached to the end of the 'bomb'
	spark = make_shared<FountainEffect>(500);
	spark->diffuse = glm::vec3(1,0,0);
	bomb->Attach(spark);
	Game::Initialise();
	
	//set camera position
	camera->GetController()->position =  glm::vec3(150,100,-150);
	
	return true;
}

void Assignment::Update(float timeDelta)
{	
	dynamicsWorld->stepSimulation(timeDelta,100);
	// Assignment::Watch is a function setting the camera's view to always be on the playable object (the spinBall)
	Assignment::Watch(camera, timeDelta);
	
	static float active = 1.0f ;
	//'elapsed' and the 'bond' bool are used to limit the action of a button click to one single action
	if(elapsed > active)
	{
		if(bond == false)
		{
			
			bomb->rigidBody->clearForces();
			PrintText("Attach Bomb press B");
			if (keyState[SDL_SCANCODE_B])
			{
				//activates/adds the constraint to the dynamicWorld
				dynamicsWorld->addConstraint(ball2chain);
				bond = true;
				elapsed = 0.0f;
			}
		}
		else
		{
			//angular velocity is applied to the spinBall when the 'bomb' is connected
			spinBall->rigidBody->setAngularVelocity(btVector3(0,30,0));
			PrintText("Detach and press B");
		
				
			if (keyState[SDL_SCANCODE_B])
			{
				//removes constarint from dynamicworld
				dynamicsWorld->removeConstraint(ball2chain);
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
	
	
	//success is a bool used to determin if the game is still in play or the objective has been achieved.
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
		// Creates a new tower with buddha to be played on again.
		if (keyState[SDL_SCANCODE_SPACE])
		{
			if(elapsed > active)
			{
				
					
					physicsFactory2->CreateWall(glm::vec3(0,0,0), gameCount, 10);
					
					watcher = physicsFactory2->CreateFromModel("buddha",glm::vec3(0,60,0),glm::quat(),glm::vec3(5,5,5));
					success=false;
					
			}
		}
		PrintText("*Spacebar* to play again");

	}
	else
	{
		PrintText("Destroy The Watcher's tower");
		glm::vec3 bombDistance = bomb->position - watcher->position;
		float distLength = glm::length(bombDistance);
		// the the length between the bomb and the watcher/budha is length than 50 units, the tower has a form of self defence
		if (distLength < 50)
		{
			glm::vec3 offset = glm::vec3(5,5,-5);
			glm::vec3 bombDistance = bomb->position - (watcher->position+offset);
			bombDistance = glm::normalize(bombDistance);
			physicsFactory3 = make_shared<PhysicsFactory>(dynamicsWorld);
			shared_ptr<PhysicsController> defenceObject = physicsFactory3->CreateSphere(0.5,watcher->position+offset , watcher->orientation);
			defenceObject->rigidBody->setGravity(btVector3(0,-30,0));
			defenceObject->rigidBody->applyCentralForce(GLToBtVector(bombDistance) * 50000);
		}

		// If the bomb is close enough to the 'watcher' it can detonate
		//and if the watcher is near the ground (fallen), then it will cause the detonation
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
				watcherPartical->diffuse = glm::vec3(0,0.02*i,RandomFloat()*i);
			}
			else
			{
				watcherParticals[i]->position.z = watcher->position.z + glm::sin(particleTheta) * -30;
				watcherParticals[i]->position.x =  watcher->position.x - glm::cos(particleTheta) * 30;
				watcherPartical->diffuse = glm::vec3(0,RandomFloat()*i,0.05*i);
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
//when Detonate is called, it initates 2 sets of 'shrapnel' rigidbodies which explode out with applied force.
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
		detonationBox2->rigidBody->applyCentralForce(GLToBtVector(glm::vec3(RandomFloat(), RandomFloat(), RandomFloat())) * detonationForce);
		elapsed = 0.0f;
		
	}
	
	
}
void Assignment::Watch(shared_ptr<GameComponent> x, float timeDelta)
{
	
		fromQuaternion = x->GetController()->orientation;
		glm::vec3 toShip = spinBall->position - x->GetController()->position;
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

