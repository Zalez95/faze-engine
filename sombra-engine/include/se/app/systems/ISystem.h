#ifndef I_SYSTEM_H
#define I_SYSTEM_H

namespace se::app {

	/**
	 * Class ISystem, TODO:
	 */
	class ISystem
	{
	protected:	// Attributes
		/** The elapsed time since the last update call */
		float mDeltaTime;

	public:		// Functions
		/** Class destructor */
		virtual ~ISystem() = default;

		/** Updates the ISystem
		 *
		 * @param	deltaTime the elapsed time since the last update */
		void update(float deltaTime) { mDeltaTime = deltaTime; execute(); };
	protected:
		/** The function that is going to be called each time the ISystem has
		 * to be updated */
		virtual void execute() = 0;
	};

}

#endif		// I_SYSTEM_H
