#include <algorithm>
#include "se/app/systems/EntitySystem.h"

namespace se::app {

	void EntitySystem::addEntity(AppComponentDB::EntityId entity)
	{
		mEntities.push_back(entity);
	}


	void EntitySystem::removeEntity(AppComponentDB::EntityId entity)
	{
		auto itEntity = std::find(mEntities.begin(), mEntities.end(), entity);
		if (itEntity != mEntities.end()) {
			if (mEntities.size() > 1) {
				std::swap(*itEntity, mEntities.back());
			}
			mEntities.pop_back();
		}
	}

// Private functions
	void EntitySystem::executeCallback(AppComponentDB::EntityId entity, const EntityCallback& callback)
	{
		lockComponents();
		callback(entity);
		unlockComponents();
	}


	void EntitySystem::executeForEach(const EntityCallback& callback)
	{
		lockComponents();
		for (auto entity : mEntities) {
			callback(entity);
		}
		unlockComponents();
	}


	void EntitySystem::lockComponents()
	{
		// TODO:
	}


	void EntitySystem::unlockComponents()
	{
		// TODO:
	}

}
