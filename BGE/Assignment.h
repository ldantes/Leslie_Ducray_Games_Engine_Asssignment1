#pragma once
#include "Game.h"
#include "GameComponent.h"
#include "SnowEffect.h"
#include "FountainEffect.h"
#include "PhysicsFactory.h"
#include "PhysicsController.h"
#include <btBulletDynamicsCommon.h>


#define NUM_FOUNTAINS 20
#define FOUNTAIN_RADIUS 100.0f
#define FOUNTAIN_HEIGHT 10.0f

using namespace std;

namespace BGE
{
	class Assignment :
		public Game
	{

	private:
		btBroadphaseInterface* broadphase;
 
		// Set up the collision configuration and dispatcher
		btDefaultCollisionConfiguration * collisionConfiguration;
		btCollisionDispatcher * dispatcher;
 
		// The actual physics solver
		btSequentialImpulseConstraintSolver * solver;

	public:
		Assignment(void);
		~Assignment(void);
		btDiscreteDynamicsWorld * dynamicsWorld;
		shared_ptr<PhysicsController> watcher;
		shared_ptr<PhysicsController> ship1;
		shared_ptr<PhysicsController> bomb;
		shared_ptr<PhysicsController> pillar;
		shared_ptr<PhysicsController> chain;
		btPoint2PointConstraint  * hinge;
				
		shared_ptr<SnowEffect> snow;
		bool Initialise();
		void Detonate();
		void Watch(shared_ptr<GameComponent>,float timeDelta);
		void Update(float timeDelta);
		float detonationForce;
		float mass;
		glm::vec3 force;
		shared_ptr<FountainEffect> spark;
		shared_ptr<FountainEffect> blackMagic;
		vector<shared_ptr<FountainEffect>> watcherParticals;
		shared_ptr<FountainEffect> watcherPartical;
		glm::quat fromQuaternion;
		glm::quat toQuaternion;
		float t;
		float particleTheta;
		float ySpeed;
		bool success;
		bool bond;

		// The world.
		
		std::shared_ptr<PhysicsFactory> physicsFactory;
		std::shared_ptr<PhysicsFactory> physicsFactory2;
		std::shared_ptr<PhysicsFactory> physicsFactory3;
		
		
	};
}