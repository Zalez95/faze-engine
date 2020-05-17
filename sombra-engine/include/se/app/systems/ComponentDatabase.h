#ifndef COMPONENT_DATABASE_H
#define COMPONENT_DATABASE_H

#include <tuple>
#include <vector>
#include <functional>
#include <shared_mutex>

namespace se::app {

	/**
	 * Class ComponentDatabase. It holds all the Components of the Entities
	 */
	template <typename SizeType = unsigned long, typename... Types>
	class ComponentDatabase
	{
	public:		// Nested types
		using EntityId = SizeType;
		using size_type = SizeType;
		using EntityCallback = std::function<void(EntityId)>;
	private:
		/** Holds a vector of Components and a mutex used for accessing them */
		template <typename T>
		struct ComponentVector
		{
			using value_type = T;

			std::shared_mutex mutex;	///< The mutex used for R/W the data
			std::vector<bool> active;	///< Tells which components are active
			std::vector<T> data;		///< The vector of Components
		};

		using ComponentVectorsTuple = std::tuple<ComponentVector<Types>...>;
	public:
		template <unsigned long componentId>
		using ComponentType = typename std::tuple_element_t<
				componentId, ComponentVectorsTuple
			>::value_type;

	private:	// Attributes
		/** The maximum number of entities allowed in the ComponentDatabase */
		const SizeType mMaxEntities;

		/** The mutex used for accessing @see mActiveEntities */
		std::shared_mutex mEntitiesMutex;

		/** Stores which Entities are active or has been released. The Entities
		 * aren't actually removed from the Database, they are released for
		 * later usage. This way the pointers and indexes to the Entities don't
		 * get invalidated */
		std::vector<bool> mActiveEntities;

		/** The components of the Entities */
		ComponentVectorsTuple mComponentsVectors;

	public:		// Functions
		/** Creates a new ComponentDatabase
		 *
		 * @param	maxEntities the maximum number of Entities allowed in the
		 *			Database */
		ComponentDatabase(SizeType maxEntities);

		/** Class destructor */
		virtual ~ComponentDatabase() = default;

		/** @return	the maximum number of entities allowed in the
		 *			ComponentDatabase */
		SizeType getMaxEntities() const { return mMaxEntities; };

		/** Adds a new Entity to the ComponentDatabase
		 *
		 * @return	The EntityId of the new Entity, mMaxEntities if it couldn't
		 *			be added */
		EntityId addEntity();

		/** Removes the given Entity and its active Components from the
		 * ComponentDatabase
		 *
		 * @return	entityId the EntityId of the Entity to remove */
		void removeEntity(EntityId entityId);

		/** Returns if the given Entity has the requested Component or not
		 *
		 * @param	entityId the id of the Entity to check for the Component
		 * @return	true if the Entity has the Component, false otherwise
		 * @note	this function isn't thread safe */
		template <unsigned long componentId>
		bool hasComponent(EntityId entityId) const;

		/** Returns the requested Component of the given Entity
		 *
		 * @param	entityId the id of the Entity whose Component we want to get
		 * @return	a reference to the requested Component
		 * @note	this function isn't thread safe */
		template <unsigned long componentId>
		ComponentType<componentId>& getComponent(EntityId entityId);

		/** Returns the requested Component of the given Entity
		 *
		 * @param	entityId the id of the Entity whose Component we want to get
		 * @return	a reference to the requested Component
		 * @note	this function isn't thread safe */
		template <unsigned long componentId>
		const ComponentType<componentId>&
			cgetComponent(EntityId entityId) const;

		/** Adds a Component to the given Entity
		 *
		 * @param	entityId the id of the Entity to add the Component
		 * @return	args the constructor arguments for creating the Component */
		template <unsigned long componentId, typename... Args>
		void addComponent(EntityId entityId, Args&&... args);

		/** Removes the given Component from the given Entity
		 *
		 * @param	entityId the id of the Entity which we want to remove the
		 *			Component */
		template <unsigned long componentId>
		void removeComponent(EntityId entityId);

		/** Locks all the Components with the given TypeId for reading */
		template <unsigned long componentId>
		void lockComponentsRead();

		/** Unlocks all the Components with the given TypeId from reading */
		template <unsigned long componentId>
		void unlockComponentsRead();

		/** Locks all the Components with the given TypeId for writting */
		template <unsigned long componentId>
		void lockComponentsWrite();

		/** Unlocks all the Components with the given TypeId from writting */
		template <unsigned long componentId>
		void unlockComponentsWrite();

		/** Function used for iterating the Entities added to the
		 * ComponentDatabase
		 *
		 * @param	callback the function to call for each Entity */
		void processEntities(const EntityCallback& callback) const;
	};

}

#include "ComponentDatabase.hpp"

#endif		// COMPONENT_DATABASE_H
