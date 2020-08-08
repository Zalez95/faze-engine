#include <memory>
#include <vector>
#include <glm/glm.hpp>
#include "../graphics/GraphicsEngine.h"
#include "../graphics/Pass.h"
#include "../graphics/Technique.h"
#include "../graphics/FBClearNode.h"
#include "../graphics/FBCopyNode.h"
#include "../graphics/TextureUnitNode.h"
#include "../graphics/2D/Renderer2D.h"
#include "../graphics/3D/Renderer3D.h"
#include "../graphics/3D/RenderableMesh.h"
#include "../graphics/core/Texture.h"
#include "../graphics/core/FrameBuffer.h"
#include "../graphics/core/UniformBuffer.h"
#include "../graphics/core/UniformVariable.h"
#include "graphics/Camera.h"
#include "graphics/GaussianBlurNode.h"
#include "loaders/MeshLoader.h"

namespace se::app {

	static constexpr int kPosition			= 0;
	static constexpr int kNormal			= 1;
	static constexpr int kAlbedo			= 2;
	static constexpr int kMaterial			= 3;
	static constexpr int kEmissive			= 4;
	static constexpr int kIrradianceMap		= 5;
	static constexpr int kPrefilterMap		= 6;
	static constexpr int kBRDFMap			= 7;
	static constexpr int kColor				= 0;
	static constexpr int kBright			= 1;

	Camera* activeCamera;
	PassSPtr lightingPass;
	std::shared_ptr<graphics::UniformVariableValue<glm::vec3>> viewPosition;
	std::shared_ptr<graphics::UniformVariableValue<unsigned int>> numLights;
	std::shared_ptr<graphics::UniformBuffer> lightsBuffer;
	std::shared_ptr<graphics::Texture> irradianceMap, prefilterMap, brdfMap;
	std::shared_ptr<graphics::RenderableMesh> planeRenderable;


	AppRenderer::AppRenderer(graphics::GraphicsEngine& graphicsEngine, std::size_t width, std::size_t height) :
		mGraphicsEngine(graphicsEngine)
	{
		// Create the buffer used for storing the light sources data
		lightsBuffer = std::make_shared<graphics::UniformBuffer>();

		// Create the plane used for rendering the framebuffers
		RawMesh planeRawMesh;
		planeRawMesh.positions = { {-1.0f,-1.0f, 0.0f }, { 1.0f,-1.0f, 0.0f }, {-1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } };
		planeRawMesh.faceIndices = { 0, 1, 2, 1, 3, 2, };
		auto planeMesh = std::make_unique<graphics::Mesh>(MeshLoader::createGraphicsMesh(planeRawMesh));
		planeRenderable = std::make_shared<graphics::RenderableMesh>(std::move(planeMesh));

		{	// Create the FBClearNodes
			auto clearMask = graphics::FrameBufferMask::Mask().set(graphics::FrameBufferMask::kColor).set(graphics::FrameBufferMask::kDepth);
			mGraphicsEngine.getRenderGraph().addNode(std::make_unique<graphics::FBClearNode>("defaultFBClear", clearMask));
			mGraphicsEngine.getRenderGraph().addNode(std::make_unique<graphics::FBClearNode>("gFBClear", clearMask));
			mGraphicsEngine.getRenderGraph().addNode(std::make_unique<graphics::FBClearNode>("deferredFBClear", clearMask));
		}

		{	// Create the gBufferRenderer
			auto resources = dynamic_cast<graphics::BindableRenderNode*>(mGraphicsEngine.getRenderGraph().getNode("resources"));

			auto gBuffer = std::make_shared<graphics::FrameBuffer>();
			auto iGBufferResource = resources->addBindable(gBuffer);
			resources->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("gBuffer", resources, iGBufferResource) );

			auto gBufferRenderer = std::make_unique<graphics::Renderer3D>("gBufferRenderer");
			auto iGBufferBindable = gBufferRenderer->addBindable();
			gBufferRenderer->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::FrameBuffer>>("gBuffer", gBufferRenderer.get(), iGBufferBindable) );
			gBufferRenderer->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("gBuffer", gBufferRenderer.get(), iGBufferBindable) );

			auto depthTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
			depthTexture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::Depth, graphics::ColorFormat::Depth24, width, height)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
			gBuffer->attach(*depthTexture, graphics::FrameBufferAttachment::kDepth);
			auto iDepthTexBindable = gBufferRenderer->addBindable(std::move(depthTexture), false);
			gBufferRenderer->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("zBuffer", gBufferRenderer.get(), iDepthTexBindable) );

			auto positionTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
			positionTexture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB16f, width, height)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
			gBuffer->attach(*positionTexture, graphics::FrameBufferAttachment::kColor0);
			auto iPositionTexBindable = gBufferRenderer->addBindable(std::move(positionTexture), false);
			gBufferRenderer->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("position", gBufferRenderer.get(), iPositionTexBindable) );

			auto normalTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
			normalTexture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB16f, width, height)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
			gBuffer->attach(*normalTexture, graphics::FrameBufferAttachment::kColor0 + 1);
			auto iNormalTexBindable = gBufferRenderer->addBindable(std::move(normalTexture), false);
			gBufferRenderer->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("normal", gBufferRenderer.get(), iNormalTexBindable) );

			auto albedoTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
			albedoTexture->setImage(nullptr, graphics::TypeId::UnsignedByte, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB, width, height)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
			gBuffer->attach(*albedoTexture, graphics::FrameBufferAttachment::kColor0 + 2);
			auto iAlbedoTexBindable = gBufferRenderer->addBindable(std::move(albedoTexture), false);
			gBufferRenderer->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("albedo", gBufferRenderer.get(), iAlbedoTexBindable) );

			auto materialTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
			materialTexture->setImage(nullptr, graphics::TypeId::UnsignedByte, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB, width, height)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
			gBuffer->attach(*materialTexture, graphics::FrameBufferAttachment::kColor0 + 3);
			auto iMaterialTexBindable = gBufferRenderer->addBindable(std::move(materialTexture), false);
			gBufferRenderer->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("material", gBufferRenderer.get(), iMaterialTexBindable) );

			auto emissiveTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
			emissiveTexture->setImage(nullptr, graphics::TypeId::UnsignedByte, graphics::ColorFormat::RGB, graphics::ColorFormat::RGB, width, height)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
			gBuffer->attach(*emissiveTexture, graphics::FrameBufferAttachment::kColor0 + 4);
			auto iEmissiveTexBindable = gBufferRenderer->addBindable(std::move(emissiveTexture), false);
			gBufferRenderer->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("emissive", gBufferRenderer.get(), iEmissiveTexBindable) );

			mGraphicsEngine.getRenderGraph().addNode(std::move(gBufferRenderer));
		}

		{	// Create the rendererDeferredLight
			auto resources = dynamic_cast<graphics::BindableRenderNode*>(mGraphicsEngine.getRenderGraph().getNode("resources"));

			auto deferredBuffer = std::make_shared<graphics::FrameBuffer>();
			auto iDeferredBufferResource = resources->addBindable(deferredBuffer);
			resources->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("deferredBuffer", resources, iDeferredBufferResource) );

			auto rendererDeferredLight = std::make_unique<graphics::Renderer3D>("rendererDeferredLight");
			auto iTargetBindable = rendererDeferredLight->addBindable();
			auto iPositionTexBindable = rendererDeferredLight->addBindable();
			auto iNormalTexBindable = rendererDeferredLight->addBindable();
			auto iAlbedoTexBindable = rendererDeferredLight->addBindable();
			auto iMaterialTexBindable = rendererDeferredLight->addBindable();
			auto iEmissiveTexBindable = rendererDeferredLight->addBindable();
			rendererDeferredLight->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::FrameBuffer>>("target", rendererDeferredLight.get(), iTargetBindable) );
			rendererDeferredLight->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("position", rendererDeferredLight.get(), iPositionTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("normal", rendererDeferredLight.get(), iNormalTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("albedo", rendererDeferredLight.get(), iAlbedoTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("material", rendererDeferredLight.get(), iMaterialTexBindable) );
			rendererDeferredLight->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("emissive", rendererDeferredLight.get(), iEmissiveTexBindable) );

			auto depthTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
			depthTexture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::Depth, graphics::ColorFormat::Depth24, width, height)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
			deferredBuffer->attach(*depthTexture, graphics::FrameBufferAttachment::kDepth);
			rendererDeferredLight->addBindable(std::move(depthTexture), false);

			auto colorTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
			colorTexture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::RGBA, graphics::ColorFormat::RGBA16f, width, height)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
			deferredBuffer->attach(*colorTexture, graphics::FrameBufferAttachment::kColor0);
			auto iColorTexBindable = rendererDeferredLight->addBindable(std::move(colorTexture), false);
			rendererDeferredLight->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("color", rendererDeferredLight.get(), iColorTexBindable) );

			auto brightTexture = std::make_unique<graphics::Texture>(graphics::TextureTarget::Texture2D);
			brightTexture->setImage(nullptr, graphics::TypeId::Float, graphics::ColorFormat::RGBA, graphics::ColorFormat::RGBA16f, width, height)
				.setWrapping(graphics::TextureWrap::ClampToEdge, graphics::TextureWrap::ClampToEdge)
				.setFiltering(graphics::TextureFilter::Linear, graphics::TextureFilter::Linear);
			deferredBuffer->attach(*brightTexture, graphics::FrameBufferAttachment::kColor0 + 1);
			auto iBrightTexBindable = rendererDeferredLight->addBindable(std::move(brightTexture), false);
			rendererDeferredLight->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::Texture>>("bright", rendererDeferredLight.get(), iBrightTexBindable) );

			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<graphics::TextureUnitNode>("defPositionTexUnitNode", kPosition) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<graphics::TextureUnitNode>("defNormalTexUnitNode", kNormal) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<graphics::TextureUnitNode>("defAlbedoTexUnitNode", kAlbedo) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<graphics::TextureUnitNode>("defMaterialTexUnitNode", kMaterial) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<graphics::TextureUnitNode>("defEmissiveTexUnitNode", kEmissive) );
			mGraphicsEngine.getRenderGraph().addNode( std::move(rendererDeferredLight) );
		}

		{	// Nodes used for blurring the bright colors (bloom)
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<GaussianBlurNode>("hBlurNode", *this, planeRenderable, width, height, true) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<GaussianBlurNode>("vBlurNode", *this, planeRenderable, width, height, false) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<graphics::TextureUnitNode>("hBlurTexUnitNode", GaussianBlurNode::kColorTextureUnit) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<graphics::TextureUnitNode>("vBlurTexUnitNode", GaussianBlurNode::kColorTextureUnit) );
		}

		{	// Node used for combining the bloom and color
			static constexpr int kColor0 = 0;
			static constexpr int kColor1 = 1;

			class CombineNode : public graphics::BindableRenderNode
			{
			private:
				std::shared_ptr<graphics::RenderableMesh> mPlane;
			public:
				CombineNode(const std::string& name, GraphicsSystem& GraphicsSystem, std::shared_ptr<graphics::RenderableMesh> plane) :
					BindableRenderNode(name), mPlane(plane)
				{
					auto iTargetBindable = addBindable();
					addInput( std::make_unique<graphics::BindableRNodeInput<graphics::FrameBuffer>>("target", this, iTargetBindable) );
					addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("color0", this, addBindable()) );
					addInput( std::make_unique<graphics::BindableRNodeInput<graphics::Texture>>("color1", this, addBindable()) );
					addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("target", this, iTargetBindable) );

					auto programCombineHDR = TechniqueLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentCombineHDR.glsl");
					if (!programCombineHDR) {
						throw std::runtime_error("programCombineHDR not found");
					}
					auto program = GraphicsSystem.getProgramRepository().add("programCombineHDR", std::move(programCombineHDR));

					addBindable(program);
					addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *program, glm::mat4(1.0f)));
					addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", *program, glm::mat4(1.0f)));
					addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program, glm::mat4(1.0f)));
					addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uColor0", *program, kColor0));
					addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uColor1", *program, kColor1));
				};

				virtual void execute() override
				{
					bind();
					mPlane->bind();
					mPlane->draw();
				};
			};

			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<graphics::TextureUnitNode>("combine0TexUnitNode", kColor0) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<graphics::TextureUnitNode>("combine1TexUnitNode", kColor1) );
			mGraphicsEngine.getRenderGraph().addNode( std::make_unique<CombineNode>("combineBloomNode", *this, planeRenderable) );
		}

		{	// Create the renderer2D
			auto renderer2D = std::make_unique<graphics::Renderer2D>("renderer2D");
			auto targetIndex = renderer2D->addBindable();
			renderer2D->addInput( std::make_unique<graphics::BindableRNodeInput<graphics::FrameBuffer>>("target", renderer2D.get(), targetIndex) );
			renderer2D->addOutput( std::make_unique<graphics::BindableRNodeOutput<graphics::FrameBuffer>>("target", renderer2D.get(), targetIndex) );
			mGraphicsEngine.getRenderGraph().addNode( std::move(renderer2D) );
		}

		{	// Link the render graph nodes
			auto resources = mGraphicsEngine.getRenderGraph().getNode("resources"),
				defaultFBClear = mGraphicsEngine.getRenderGraph().getNode("defaultFBClear"),
				gFBClear = mGraphicsEngine.getRenderGraph().getNode("gFBClear"),
				deferredFBClear = mGraphicsEngine.getRenderGraph().getNode("deferredFBClear"),
				gBufferRenderer = mGraphicsEngine.getRenderGraph().getNode("gBufferRenderer"),
				defPositionTexUnitNode = mGraphicsEngine.getRenderGraph().getNode("defPositionTexUnitNode"),
				defNormalTexUnitNode = mGraphicsEngine.getRenderGraph().getNode("defNormalTexUnitNode"),
				defAlbedoTexUnitNode = mGraphicsEngine.getRenderGraph().getNode("defAlbedoTexUnitNode"),
				defMaterialTexUnitNode = mGraphicsEngine.getRenderGraph().getNode("defMaterialTexUnitNode"),
				defEmissiveTexUnitNode = mGraphicsEngine.getRenderGraph().getNode("defEmissiveTexUnitNode"),
				rendererDeferredLight = mGraphicsEngine.getRenderGraph().getNode("rendererDeferredLight"),
				hBlurNode = mGraphicsEngine.getRenderGraph().getNode("hBlurNode"),
				vBlurNode = mGraphicsEngine.getRenderGraph().getNode("vBlurNode"),
				hBlurTexUnitNode = mGraphicsEngine.getRenderGraph().getNode("hBlurTexUnitNode"),
				vBlurTexUnitNode = mGraphicsEngine.getRenderGraph().getNode("vBlurTexUnitNode"),
				combine0TexUnitNode = mGraphicsEngine.getRenderGraph().getNode("combine0TexUnitNode"),
				combine1TexUnitNode = mGraphicsEngine.getRenderGraph().getNode("combine1TexUnitNode"),
				combineBloomNode = mGraphicsEngine.getRenderGraph().getNode("combineBloomNode"),
				renderer2D = mGraphicsEngine.getRenderGraph().getNode("renderer2D");

			defaultFBClear->findInput("input")->connect( resources->findOutput("defaultFB") );
			gFBClear->findInput("input")->connect( resources->findOutput("gBuffer") );
			deferredFBClear->findInput("input")->connect( resources->findOutput("deferredBuffer") );
			gBufferRenderer->findInput("gBuffer")->connect( gFBClear->findOutput("output") );
			defPositionTexUnitNode->findInput("input")->connect( gBufferRenderer->findOutput("position") );
			defNormalTexUnitNode->findInput("input")->connect( gBufferRenderer->findOutput("normal") );
			defAlbedoTexUnitNode->findInput("input")->connect( gBufferRenderer->findOutput("albedo") );
			defMaterialTexUnitNode->findInput("input")->connect( gBufferRenderer->findOutput("material") );
			defEmissiveTexUnitNode->findInput("input")->connect( gBufferRenderer->findOutput("emissive") );
			rendererDeferredLight->findInput("target")->connect( deferredFBClear->findOutput("output") );
			rendererDeferredLight->findInput("position")->connect( defPositionTexUnitNode->findOutput("output") );
			rendererDeferredLight->findInput("normal")->connect( defNormalTexUnitNode->findOutput("output") );
			rendererDeferredLight->findInput("albedo")->connect( defAlbedoTexUnitNode->findOutput("output") );
			rendererDeferredLight->findInput("material")->connect( defMaterialTexUnitNode->findOutput("output") );
			rendererDeferredLight->findInput("emissive")->connect( defEmissiveTexUnitNode->findOutput("output") );
			hBlurTexUnitNode->findInput("input")->connect( rendererDeferredLight->findOutput("bright") );
			hBlurNode->findInput("input")->connect( hBlurTexUnitNode->findOutput("output") );
			vBlurTexUnitNode->findInput("input")->connect( hBlurNode->findOutput("output") );
			vBlurNode->findInput("input")->connect( vBlurTexUnitNode->findOutput("output") );
			combine0TexUnitNode->findInput("input")->connect( rendererDeferredLight->findOutput("color") );
			combine1TexUnitNode->findInput("input")->connect( vBlurNode->findOutput("output") );
			combineBloomNode->findInput("target")->connect( defaultFBClear->findOutput("output") );
			combineBloomNode->findInput("color0")->connect( combine0TexUnitNode->findOutput("output") );
			combineBloomNode->findInput("color1")->connect( combine1TexUnitNode->findOutput("output") );
			renderer2D->findInput("target")->connect( combineBloomNode->findOutput("target") );

			mGraphicsEngine.getRenderGraph().prepareGraph();
		}

		{	// Create the Techniques used for rendering to the framebuffers
			auto planeTechnique = std::make_unique<graphics::Technique>();

			// Create the pass and technique used for the deferred lighting
			auto rendererDeferredLight = dynamic_cast<graphics::Renderer3D*>(mGraphicsEngine.getRenderGraph().getNode("rendererDeferredLight"));

			auto programDeferredLighting = TechniqueLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentDeferredLighting.glsl");
			if (!programDeferredLighting) {
				throw std::runtime_error("programDeferredLighting not found");
			}
			auto program = mProgramRepository.add("programDeferredLighting", std::move(programDeferredLighting));

			lightingPass	= std::make_shared<graphics::Pass>(*rendererDeferredLight);
			viewPosition	= std::make_shared<graphics::UniformVariableValue<glm::vec3>>("uViewPosition", *program, glm::vec3(0.0f));
			numLights	= std::make_shared<graphics::UniformVariableValue<unsigned int>>("uNumLights", *program, 0);
			lightingPass->addBindable(program)
				.addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uModelMatrix", *program, glm::mat4(1.0f)))
				.addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uViewMatrix", *program, glm::mat4(1.0f)))
				.addBindable(std::make_shared<graphics::UniformVariableValue<glm::mat4>>("uProjectionMatrix", *program, glm::mat4(1.0f)))
				.addBindable(viewPosition)
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uPosition", *program, kPosition))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uNormal", *program, kNormal))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uAlbedo", *program, kAlbedo))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uMaterial", *program, kMaterial))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uEmissive", *program, kEmissive))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uIrradianceMap", *program, kIrradianceMap))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uPrefilterMap", *program, kPrefilterMap))
				.addBindable(std::make_shared<graphics::UniformVariableValue<int>>("uBRDFMap", *program, kBRDFMap))
				.addBindable(lightsBuffer)
				.addBindable(numLights)
				.addBindable(std::make_shared<graphics::UniformBlock>("LightsBlock", *program));
			planeTechnique->addPass(lightingPass);

			planeRenderable->addTechnique(std::move(planeTechnique));
			mGraphicsEngine.addRenderable(planeRenderable.get());
		}
	}


	void AppRenderer::render()
	{
		SOMBRA_INFO_LOG << "Render start";
		mGraphicsEngine.render();
		SOMBRA_INFO_LOG << "Render end";
	}

}
