#ifndef ENTITY_SYSTEM_HPP
#define ENTITY_SYSTEM_HPP

namespace se::app {

	template <ComponentId component>
	bool EntitySystem::hasComponent(AppComponentDB::EntityId entity) const
	{
		assert(mAccessPolicies[static_cast<unsigned long>(component)] != AccessPolicy::NoAccess);
		return mAppComponentDB.hasComponent<static_cast<unsigned long>(component)>(entity);
	}


	template <ComponentId component>
	AppComponentDB::ComponentType<static_cast<unsigned long>(component)>&
		EntitySystem::getComponentW(AppComponentDB::EntityId entity)
	{
		assert(mAccessPolicies[static_cast<unsigned long>(component)] == AccessPolicy::Write);
		return mAppComponentDB.getComponent<static_cast<unsigned long>(component)>(entity);
	}


	template <ComponentId component>
	const AppComponentDB::ComponentType<static_cast<unsigned long>(component)>&
		EntitySystem::getComponentR(AppComponentDB::EntityId entity) const
	{
		assert(mAccessPolicies[static_cast<unsigned long>(component)] != AccessPolicy::NoAccess);
		return mAppComponentDB.cgetComponent<static_cast<unsigned long>(component)>(entity);
	}


	template <ComponentId component>
	void EntitySystem::setAccessPolicy(AccessPolicy accessPolicy)
	{
		mAccessPolicies[static_cast<unsigned long>(component)] = accessPolicy;
	}

}

#endif		// ENTITY_SYSTEM_HPP
