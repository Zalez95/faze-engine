#include <string>
#include <sstream>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include "se/graphics/GLWrapper.h"
#include "se/graphics/Texture.h"
#include "se/graphics/2D/Renderer2D.h"
#include "se/graphics/2D/Renderable2D.h"

namespace se::graphics {

	constexpr float Renderer2D::Quad2D::kPositions[];


	Renderer2D::Quad2D::Quad2D() :
		mPositionsBuffer(kPositions, kNumVertices * kNumComponentsPerVertex)
	{
		mVAO.bind();
		mPositionsBuffer.bind();
		mVAO.setVertexAttribute(0, TypeId::Float, false, kNumComponentsPerVertex, 0);
		mVAO.unbind();
	}


	void Renderer2D::submit(const Renderable2D* renderable2D)
	{
		if (renderable2D) {
			mRenderable2Ds.push(renderable2D);
		}
	}


	void Renderer2D::render()
	{
		GL_WRAP( glEnable(GL_BLEND) );
		GL_WRAP( glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) );
		GL_WRAP( glDisable(GL_DEPTH_TEST) );

		mProgram.enable();
		mQuad.bind();

		while (!mRenderable2Ds.empty()) {
			const Renderable2D* renderable2D = mRenderable2Ds.front();
			mRenderable2Ds.pop();

			glm::mat4 transforms = glm::translate(glm::mat4(1.0f), glm::vec3(renderable2D->getPosition(), 0));
			transforms *= glm::scale(glm::mat4(1.0f), glm::vec3(renderable2D->getScale(), 1));
			auto texture = renderable2D->getTexture();

			mProgram.setModelMatrix(transforms);
			mProgram.setTextureSampler(0);

			if (texture) { texture->bind(0); }
			GL_WRAP( glDrawArrays(GL_TRIANGLE_STRIP, 0, mQuad.getNumVertices()) );
			if (texture) { texture->unbind(); }
		}

		mQuad.unbind();
		mProgram.disable();

		GL_WRAP( glEnable(GL_DEPTH_TEST) );
		GL_WRAP( glDisable(GL_BLEND) );
	}

}
