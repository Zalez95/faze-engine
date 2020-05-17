#ifndef APP_COMPONENT_DB_H
#define APP_COMPONENT_DB_H

#include <memory>
#include <string>
#include "../../graphics/3D/Renderable3D.h"
#include "../../audio/Source.h"
#include "../../animation/AnimationNode.h"
#include "../../physics/RigidBody.h"
#include "../../collision/Collider.h"
#include "../graphics/Camera.h"
#include "../graphics/Lights.h"
#include "TransformsComponent.h"
#include "ComponentDatabase.h"

namespace se::app {

	/** The Ids of the Components in the AppComponentDB */
	enum class ComponentId : unsigned long
	{
		Name = 0,
		Transforms,
		Camera,
		Renderable3D,
		Light,
		AudioSource,
		AnimationNode,
		RigidBody,
		Collider,
		NumComponentTypes
	};


	/** The ComponentDatabase that holds all the Components of the
	 * Entities */
	using AppComponentDB = ComponentDatabase<
		unsigned short,
		std::string,
		TransformsComponent,
		Camera,
		std::shared_ptr<graphics::Renderable3D>,
		std::shared_ptr<ILight>,
		std::shared_ptr<audio::Source>,
		animation::AnimationNode,
		physics::RigidBody,
		std::shared_ptr<collision::Collider>
	>;

}

#endif		// APP_COMPONENT_DB_H
