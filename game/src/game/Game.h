#ifndef GAME_H
#define GAME_H

#include <memory>
#include <vector>
#include <string>
#include <fe/app/Application.h>
#include <fe/graphics/2D/Layer2D.h>
#include <fe/graphics/2D/Renderable2D.h>
#include <fe/audio/Buffer.h>
#include <fe/physics/Constraint.h>

namespace game {

	/**
	 * Class Engine
	 */
	class Game : public fe::app::Application
	{
	private:	// Constants
		static const std::string sTitle;
		static const unsigned int sWidth;
		static const unsigned int sHeight;
		static const float sUpdateTime;
		static const unsigned int sNumCubes;

	private:	// Attributes
		fe::graphics::Layer2D mLayer2D;
		std::vector<fe::graphics::Renderable2D> mRenderable2Ds;
		fe::audio::Buffer buffer1;
		fe::physics::Constraint* constraint;

	public:		// Functions
		/** Creates a new Game */
		Game() : fe::app::Application(sTitle, sWidth, sHeight, sUpdateTime) {};

		/** Class destructor */
		~Game() {};
	private:
		/**
		 * Function used to initialize the application's entities data
		 */
		void init() override;

		/**
		 * Function used to remove the application's entities data
		 */
		void end() override;
	};

}

#endif		// GAME_H
