#include "fe/graphics/GLWrapper.h"
#include "fe/graphics/3D/Mesh.h"
#include "fe/graphics/3D/Camera.h"
#include "fe/graphics/3D/Renderer3D.h"
#include "fe/graphics/3D/Renderable3D.h"

namespace fe { namespace graphics {

	Renderer3D::Renderer3D(const glm::mat4& projectionMatrix) :
		mProjectionMatrix(projectionMatrix),
		mDefaultMaterial(
			"3D renderer default material",
			RGBColor(0.25f, 0.25f, 0.25f), RGBColor(1.0f, 0.0f, 1.0f),
			RGBColor(1.0f, 1.0f, 1.0f), 0.25f
		)
	{
		const float pixels[] = {
			1.0f, 0.0f, 0.86f,	0.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.0f,	1.0f, 0.0f, 0.86f
		};

		mDefaultTexture.setImage(
			pixels, fe::graphics::TexturePixelType::FLOAT,
			fe::graphics::TextureFormat::RGB, 2, 2
		);
	}


	void Renderer3D::submit(const Renderable3D* renderable3D)
	{
		if (renderable3D) {
			mRenderable3Ds.push(renderable3D);
		}
	}


	void Renderer3D::render(const Camera* camera, const std::vector<const PointLight*>& pointLights)
	{
		glm::mat4 viewMatrix = (camera)? camera->getViewMatrix() : glm::mat4(1.0f);

		mProgram.enable();
		mProgram.setViewMatrix(viewMatrix);
		mProgram.setProjectionMatrix(mProjectionMatrix);
		mProgram.setLights(pointLights);

		while (!mRenderable3Ds.empty()) {
			const Renderable3D* renderable3D = mRenderable3Ds.front();
			mRenderable3Ds.pop();

			auto mesh				= renderable3D->getMesh();
			auto material			= renderable3D->getMaterial();
			auto texture			= renderable3D->getTexture();
			glm::mat4 modelMatrix	= renderable3D->getModelMatrix();
			RenderFlags flags		= renderable3D->getRenderFlags();

			if (!mesh) { continue; }

			// Set the mesh transparency
			if (flags & RenderFlags::DISABLE_DEPTH_TEST) {
				GL_WRAP( glEnable(GL_BLEND) );
				GL_WRAP( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
				GL_WRAP( glDisable(GL_DEPTH_TEST) );
			}

			// Bind the program data
			mProgram.setModelMatrix(modelMatrix);
			mProgram.setColorTexture(0);
			mProgram.setMaterial( (material)? material.get() : &mDefaultMaterial );
			if (texture) { texture->bind(0); } else { mDefaultTexture.bind(0); };

			// Draw
			GLenum renderMode = (flags & RenderFlags::WIREFRAME)? GL_LINES : GL_TRIANGLES;

			mesh->bind();
			GL_WRAP( glDrawElements(renderMode, mesh->getIndexCount(), GL_UNSIGNED_SHORT, nullptr) );
			mesh->unbind();

			// Unbind the program data
			if (texture) { texture->unbind(); }

			// Unset the mesh transparency
			if (flags & RenderFlags::DISABLE_DEPTH_TEST) {
				GL_WRAP( glEnable(GL_DEPTH_TEST) );
				GL_WRAP( glDisable(GL_BLEND) );
			}
		}

		mProgram.disable();
	}

}}
