#ifndef ENTITY_SYSTEM_H
#define ENTITY_SYSTEM_H

#include <array>
#include "ISystem.h"
#include "AppComponentDB.h"

namespace se::app {

	/**
	 * Class EntitySystem, it's an ISystem that has access to the Entities
	 * Components to operate with them
	 */
	class EntitySystem : public ISystem
	{
	protected:	// Nested types
		using EntityCallback = std::function<void(AppComponentDB::EntityId)>;

		/** The different access policies that an EntitySystem can have to
		 * each of the Components */
		enum class AccessPolicy { NoAccess, Read, Write };

	private:	// Attributes
		/** The number of Components types in the EntitySystem */
		static constexpr unsigned long kNComponentTypes =
			static_cast<unsigned long>(ComponentId::NumComponentTypes);

		/** A reference to the AppComponentDB that holds all the Entities and
		 * their components */
		AppComponentDB& mAppComponentDB;

		/** The access policies that the EntitySystem has to each of the
		 * Components */
		std::array<AccessPolicy, kNComponentTypes> mAccessPolicies;

		/** The Entities to update */
		std::vector<AppComponentDB::EntityId> mEntities;

	public:		// Functions
		/** Creates a new EntitySystem
		 *
		 * @param	AppComponentDB a reference to the AppComponentDB that holds
		 *			all the Component of the Entities */
		EntitySystem(AppComponentDB& AppComponentDB) :
			mAppComponentDB(AppComponentDB) {};

		/** Class destructor */
		virtual ~EntitySystem() = default;

		/** Adds the given Entity to the current EntitySystem
		 *
		 * @param	entity the Entity to add to the EntitySystem */
		virtual void addEntity(AppComponentDB::EntityId entity);

		/** Removes the given Entity from the current EntitySystem
		 *
		 * @param	entity the Entity to remove from the EntitySystem */
		virtual void removeEntity(AppComponentDB::EntityId entity);
	protected:
		/** Executes the given function for the given Entity
		 *
		 * @param	entity the Entity with which we want to call the callback
		 * @param	callback the function to call */
		void executeCallback(
			AppComponentDB::EntityId entity, const EntityCallback& callback
		);

		/** Executes the given function for each Entity added to the
		 * EntitySystem
		 *
		 * @param	callback the function to call for each Entity */
		void executeForEach(const EntityCallback& callback);

		/** Returns if Entity has the requested Component
		 *
		 * @tparam	component the id of the requested Component
		 * @param	Entity the Entity that holds the Component
		 * @return	true if the Entity has the Component, false otherwise
		 * @note	this methods can only retrieve Components with read or
		 *			write access policy */
		template <ComponentId component>
		bool hasComponent(AppComponentDB::EntityId entity) const;

		/** Returns the requested Entity component for writting
		 *
		 * @tparam	component the id of the requested component Component
		 * @param	Entity the Entity that holds the Component
		 * @note	this methods can only retrieve Components with write access
		 *			policy */
		template <ComponentId component>
		AppComponentDB::ComponentType<static_cast<unsigned long>(component)>&
			getComponentW(AppComponentDB::EntityId entity);

		/** Returns the requested Entity component for reading
		 *
		 * @tparam	component the id of the requested Component
		 * @param	Entity the Entity that holds the Component
		 * @note	this methods can only retrieve Components with read or
		 *			write access policy */
		template <ComponentId component>
		const
		AppComponentDB::ComponentType<static_cast<unsigned long>(component)>&
			getComponentR(AppComponentDB::EntityId entity) const;

		/** Sets the AccessPolicy for the given ComponentId
		 *
		 * @tparam	component the ComponentId that we want to access
		 * @param	accessPolicy the AccessPolicy for accessing the Component */
		template <ComponentId component>
		void setAccessPolicy(AccessPolicy accessPolicy);
	private:
		/** Locks the Components mutexes in the ComponentDatabase that has an
		 * AccessPolicy setted */
		void lockComponents();

		/** Unlocks the Components mutexes in the ComponentDatabase that has an
		 * AccessPolicy setted */
		void unlockComponents();
	};

}

#include "EntitySystem.hpp"

#endif		// ENTITY_SYSTEM_H
