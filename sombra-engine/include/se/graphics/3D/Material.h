#ifndef MATERIAL_H
#define MATERIAL_H

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "../PrimitiveTypes.h"

namespace se::graphics {

	/**
	 * Class Material, it holds the data of the surface material properties
	 * of a Renderable3D, used for the Phong Shading in the shaders
	 */
	class Material
	{
	protected:	// Attributes
		/** The name of the Material */
		std::string mName;

		/** The ambient color of the Material */
		glm::vec3 mAmbientColor;

		/** The color of the diffuse reflected light the Material */
		glm::vec3 mDiffuseColor;

		/** The color of the specular reflected light the Material */
		glm::vec3 mSpecularColor;

		/** The specular shininess of the Material */
		float mShininess;

	public:		// Functions
		/** Creates a new Material
		 *
		 * @param	name the name of the Material
		 * @param	diffuseColor the diffuse color of the new Material
		 * @param	ambientColor the ambient color of the new Material
		 * @param	specularColor the specular color of the new Material
		 * @param	shininess the specular shininess of the the new Material */
		Material(
			const std::string& name,
			const glm::vec3& ambientColor,
			const glm::vec3& diffuseColor,
			const glm::vec3& specularColor,
			float shininess
		) : mName(name),
			mAmbientColor(ambientColor),
			mDiffuseColor(diffuseColor),
			mSpecularColor(specularColor),
			mShininess(shininess) {}

		/** Class destructor */
		~Material() {};

		/** @return the name of the material */
		inline std::string getName() const { return mName; };

		/** @return the ambient color of the Material */
		inline glm::vec3 getAmbientColor() const { return mAmbientColor; };

		/** @return the diffuse color of the Material */
		inline glm::vec3 getDiffuseColor() const { return mDiffuseColor; };

		/** @return the specular color of the Material */
		inline glm::vec3 getSpecularColor() const { return mSpecularColor; };

		/** @return the specular shininess of the Material */
		inline float getShininess() const
		{ return mShininess; };
	};

}

#endif		// MATERIAL_H