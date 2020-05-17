#ifndef COMPONENT_DATABASE_HPP
#define COMPONENT_DATABASE_HPP

namespace se::app {

	template <typename SizeType, typename... Types>
	ComponentDatabase<SizeType, Types...>::ComponentDatabase(SizeType maxEntities) : mMaxEntities(maxEntities)
	{
		mActiveEntities.resize(mMaxEntities, false);
		std::apply(
			[this](auto&& args) {
				args.data.active.resize(mMaxEntities, false);
				args.data.resize(mMaxEntities);
			},
			mComponentsVectors
		);
	}


	template <typename SizeType, typename... Types>
	typename ComponentDatabase<SizeType, Types...>::EntityId ComponentDatabase<SizeType, Types...>::addEntity()
	{
		EntityId ret = mMaxEntities;

		std::unique_lock lck(mEntitiesMutex);
		auto itInactiveEntity = std::find(mActiveEntities.begin(), mActiveEntities.end(), false);
		if (itInactiveEntity != mActiveEntities.end()) {
			*itInactiveEntity = true;
			ret = static_cast<EntityId>( std::distance(mActiveEntities.begin(), itInactiveEntity) );
		}

		return ret;
	}


	template <typename SizeType, typename... Types>
	void ComponentDatabase<SizeType, Types...>::removeEntity(EntityId entityId)
	{
		std::unique_lock lck(mEntitiesMutex);
		if ((entityId < mActiveEntities.size()) && mActiveEntities[entityId]) {
			mActiveEntities[entityId] = false;
			std::apply(
				[&](auto&& args) {
					std::unique_lock lck(args.mutex);
					args.active[entityId] = false;
					args.data[entityId] = {};
				},
				mComponentsVectors
			);
		}
	}


	template <typename SizeType, typename... Types>
	template <unsigned long componentId>
	bool ComponentDatabase<SizeType, Types...>::hasComponent(EntityId entityId) const
	{
		return std::get<componentId>(mComponentsVectors).active[entityId];
	}


	template <typename SizeType, typename... Types>
	template <unsigned long componentId>
	typename ComponentDatabase<SizeType, Types...>::template ComponentType<componentId>&
		ComponentDatabase<SizeType, Types...>::getComponent(EntityId entityId)
	{
		return std::get<componentId>(mComponentsVectors).data[entityId];
	}


	template <typename SizeType, typename... Types>
	template <unsigned long componentId>
	const typename ComponentDatabase<SizeType, Types...>::template ComponentType<componentId>&
		ComponentDatabase<SizeType, Types...>::cgetComponent(EntityId entityId) const
	{
		return std::get<componentId>(mComponentsVectors).data[entityId];
	}


	template <typename SizeType, typename... Types>
	template <unsigned long componentId, typename... Args>
	void ComponentDatabase<SizeType, Types...>::addComponent(EntityId entityId, Args&&... args)
	{
		std::shared_lock lck(mEntitiesMutex);
		if ((entityId < mActiveEntities.size()) && mActiveEntities[entityId]) {
			auto& components = std::get<componentId>(mComponentsVectors);
			std::unique_lock lck(components.mutex);
			components.active[entityId] = true;
			components.data[entityId] = T(std::forward<Args>(args)...);
		}
	}


	template <typename SizeType, typename... Types>
	template <unsigned long componentId>
	void ComponentDatabase<SizeType, Types...>::removeComponent(EntityId entityId)
	{
		std::shared_lock lck(mEntitiesMutex);
		if ((entityId < mActiveEntities.size()) && mActiveEntities[entityId]) {
			auto& components = std::get<componentId>(mComponentsVectors);
			std::unique_lock lck(components.mutex);
			components.active[entityId] = false;
			components.data[entityId] = {};
		}
	}


	template <typename SizeType, typename... Types>
	template <unsigned long componentId>
	void ComponentDatabase<SizeType, Types...>::lockComponentsRead()
	{
		std::get<componentId>(mComponentsVectors).mutex.lock_shared();
	}


	template <typename SizeType, typename... Types>
	template <unsigned long componentId>
	void ComponentDatabase<SizeType, Types...>::unlockComponentsRead()
	{
		std::get<componentId>(mComponentsVectors).mutex.unlock_shared();
	}


	template <typename SizeType, typename... Types>
	template <unsigned long componentId>
	void ComponentDatabase<SizeType, Types...>::lockComponentsWrite()
	{
		std::get<componentId>(mComponentsVectors).mutex.lock();
	}


	template <typename SizeType, typename... Types>
	template <unsigned long componentId>
	void ComponentDatabase<SizeType, Types...>::unlockComponentsWrite()
	{
		std::get<componentId>(mComponentsVectors).mutex.unlock();
	}


	template <typename SizeType, typename... Types>
	void ComponentDatabase<SizeType, Types...>::processEntities(const EntityCallback& callback) const
	{
		std::shared_lock lck(mEntitiesMutex);
		for (EntityId entityId = 0; entityId < mMaxEntities; ++entityId) {
			if (mActiveEntities[entityId]) {
				callback(entityId);
			}
		}
	}

}

#endif		// COMPONENT_DATABASE_HPP
