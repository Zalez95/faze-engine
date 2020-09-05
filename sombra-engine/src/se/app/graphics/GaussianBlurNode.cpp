#include <glm/gtc/matrix_transform.hpp>
#include "se/app/graphics/GaussianBlurNode.h"
#include "se/app/loaders/TechniqueLoader.h"
#include "se/graphics/core/Texture.h"
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/UniformVariable.h"
#include "se/graphics/core/GraphicsOperations.h"

namespace se::app {

	GaussianBlurNode::GaussianBlurNode(
		const std::string& name, utils::Repository& repository,
		std::shared_ptr<graphics::RenderableMesh> plane,
		std::size_t width, std::size_t height, bool horizontal
	) : BindableRenderNode(name), mPlane(plane)
	{
		auto iColorTexBindable = addBindable();
		addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("input", this, iColorTexBindable) );

		auto frameBuffer = std::make_unique<graphics::FrameBuffer>();
		auto outputTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
		outputTexture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::RGBA, graphics::ColorFormat::RGBA16f, width, height)
			.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
			.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
		frameBuffer->attach(*outputTexture, graphics::FrameBufferAttachment::kColor0);
		auto iOutputTexBindable = addBindable(std::move(outputTexture), false);
		addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("output", this, iOutputTexBindable) );

		auto program = repository.find<std::string, graphics::Program>("programGaussianBlur");
		if (!program) {
			program = TechniqueLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentGaussianBlur.glsl");
			repository.add(std::string("programGaussianBlur"), program);
		}

		addBindable(std::move(frameBuffer));
		addBindable(program);
		addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *program, glm::mat4(1.0f)));
		addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", *program, glm::mat4(1.0f)));
		addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program, glm::mat4(1.0f)));
		addBindable(std::make_shared<graphics::UniformVariableValue<bool>>("uHorizontal", *program, horizontal));
		addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uColor", *program, kColorTextureUnit));
	}


	void GaussianBlurNode::execute()
	{
		graphics::GraphicsOperations::setDepthTest(false);
		graphics::GraphicsOperations::setDepthMask(false);

		bind();
		mPlane->bind();

		auto mask = graphics::FrameBufferMask::Mask().set(graphics::FrameBufferMask::kColor);
		graphics::GraphicsOperations::clear(mask);

		mPlane->draw();

		graphics::GraphicsOperations::setDepthMask(true);
		graphics::GraphicsOperations::setDepthTest(true);
	}

}