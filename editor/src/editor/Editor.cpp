#include <se/utils/Log.h>
#include <se/graphics/Renderer.h>
#include <se/graphics/GraphicsEngine.h>
#include <se/app/loaders/MeshLoader.h>
#include <se/app/loaders/ShaderLoader.h>
#include <se/app/EntityDatabase.h>
#include <se/app/RenderableShader.h>
#include <se/app/TransformsComponent.h>
#include <se/app/CameraComponent.h>
#include <se/app/events/ContainerEvent.h>
#include "Editor.h"

namespace editor {

	Editor::Editor() :
		se::app::Application(
			{ kTitle, kWidth, kHeight },
			{	kMaxManifolds, kMinFDifference, kMaxCollisionIterations,
				kContactPrecision, kContactSeparation, kMaxRayCasterIterations
			},
			kUpdateTime
		),
		mImGuiContext(nullptr), mImGuiInput(nullptr), mImGuiRenderer(nullptr),
		mMenuBar(nullptr), mEntityPanel(nullptr), mComponentPanel(nullptr), mRepositoryPanel(nullptr),
		mViewportEntity(se::app::kNullEntity), mViewportControl(nullptr),
		mScene(nullptr)
	{
		if (mState != AppState::Error) {
			mEventManager->subscribe(this, se::app::Topic::Close);

			// Create the ImGui context and renderer
			IMGUI_CHECKVERSION();
			mImGuiContext = ImGui::CreateContext();
			ImGui::StyleColorsDark();

			ImGuiIO& io = ImGui::GetIO();
			io.DisplaySize = ImVec2(kWidth, kHeight);
			io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
			io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

			mImGuiInput = new ImGuiInput(*mEventManager);

			mImGuiRenderer = new ImGuiRenderer("ImGuiRenderer");
			auto& renderGraph = mExternalTools->graphicsEngine->getRenderGraph();
			mImGuiRenderer->findInput("target")->connect( renderGraph.getNode("renderer2D")->findOutput("target") );
			renderGraph.addNode( std::unique_ptr<ImGuiRenderer>(mImGuiRenderer) );

			// Add the GUI components
			mMenuBar = new MenuBar(*this);
			mEntityPanel = new EntityPanel(*this);
			mComponentPanel = new ComponentPanel(*this);
			mRepositoryPanel = new RepositoryPanel(*this);

			// Create the Entity used for controlling the viewport
			mViewportEntity = mEntityDatabase->addEntity();
			mEntityDatabase->emplaceComponent<se::app::TransformsComponent>(mViewportEntity);

			se::app::CameraComponent camera;
			camera.setPerspectiveProjection(glm::radians(kFOV), kWidth / static_cast<float>(kHeight), kZNear, kZFar);
			mEntityDatabase->addComponent(mViewportEntity, std::move(camera));

			mEventManager->publish(new se::app::ContainerEvent<se::app::Topic::Camera, se::app::Entity>(mViewportEntity));

			mViewportControl = new ViewportControl(*this, mViewportEntity);
		}
		else {
			SOMBRA_FATAL_LOG << "Couldn't create the Editor: The Application has errors";
		}
	}


	Editor::~Editor()
	{
		destroyScene();

		if (mViewportControl) { delete mViewportControl; }
		if (mViewportEntity != se::app::kNullEntity) { mEntityDatabase->removeEntity(mViewportEntity); }

		if (mRepositoryPanel) { delete mRepositoryPanel; }
		if (mComponentPanel) { delete mComponentPanel; }
		if (mEntityPanel) { delete mEntityPanel; }
		if (mMenuBar) { delete mMenuBar; }

		mExternalTools->graphicsEngine->getRenderGraph().removeNode(mImGuiRenderer);
		if (mImGuiInput) { delete mImGuiInput; }
		if (mImGuiContext) { ImGui::DestroyContext(mImGuiContext); }

		if (mEventManager) { mEventManager->unsubscribe(this, se::app::Topic::Close); }
	}


	void Editor::createScene(const char* name)
	{
		mScene = new se::app::Scene(name, *this);

		// Default Scene resources
		auto cubeRawMesh = se::app::MeshLoader::createBoxMesh("cube", glm::vec3(1.0f));
		auto mesh = std::make_shared<se::graphics::Mesh>(se::app::MeshLoader::createGraphicsMesh(cubeRawMesh));
		mScene->repository.add<std::string, se::graphics::Mesh>("cube", mesh);

		float pixels[] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
		auto chessTexture = std::make_shared<se::graphics::Texture>(se::graphics::TextureTarget::Texture2D);
		chessTexture->setImage(pixels, se::graphics::TypeId::Float, se::graphics::ColorFormat::RGB, se::graphics::ColorFormat::RGB, 2, 2)
			.setFiltering(se::graphics::TextureFilter::Nearest, se::graphics::TextureFilter::Nearest)
			.setWrapping(se::graphics::TextureWrap::Repeat, se::graphics::TextureWrap::Repeat);
		mScene->repository.add(std::string("chessTexture"), chessTexture);

		std::shared_ptr<se::graphics::Program> programShadow;
		programShadow = se::app::ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, nullptr);
		if (!programShadow) {
			throw std::runtime_error("programShadow not found");
		}
		mScene->repository.add(std::string("programShadow"), programShadow);

		std::shared_ptr<se::graphics::Program> programShadowSkinning;
		programShadowSkinning = se::app::ShaderLoader::createProgram("res/shaders/vertex3DSkinning.glsl", nullptr, nullptr);
		if (!programShadowSkinning) {
			throw std::runtime_error("programShadowSkinning not found");
		}
		mScene->repository.add(std::string("programShadow"), programShadowSkinning);

		std::shared_ptr<se::graphics::Program> programShadowTerrain;
		programShadowTerrain = se::app::ShaderLoader::createProgram("res/shaders/vertexTerrain.glsl", "res/shaders/geometryTerrain.glsl", nullptr);
		if (!programShadowTerrain) {
			throw std::runtime_error("programShadowTerrain not found");
		}
		mScene->repository.add(std::string("programShadowTerrain"), programShadowTerrain);

		std::shared_ptr<se::graphics::Program> programSky;
		programSky = se::app::ShaderLoader::createProgram("res/shaders/vertex3D.glsl", nullptr, "res/shaders/fragmentSkyBox.glsl");
		if (!programSky) {
			throw std::runtime_error("programSky not found");
		}
		mScene->repository.add(std::string("programSky"), programSky);

		std::shared_ptr<se::graphics::Program> programGBufMaterial;
		programGBufMaterial = se::app::ShaderLoader::createProgram("res/shaders/vertexNormalMap.glsl", nullptr, "res/shaders/fragmentGBufMaterial.glsl");
		if (!programGBufMaterial) {
			throw std::runtime_error("programGBufMaterial not found");
		}
		mScene->repository.add(std::string("programGBufMaterial"), programGBufMaterial);

		std::shared_ptr<se::graphics::Program> programGBufMaterialSkinning;
		programGBufMaterialSkinning = se::app::ShaderLoader::createProgram("res/shaders/vertexNormalMapSkinning.glsl", nullptr, "res/shaders/fragmentGBufMaterial.glsl");
		if (!programGBufMaterialSkinning) {
			throw std::runtime_error("programGBufMaterialSkinning not found");
		}
		mScene->repository.add(std::string("programGBufMaterialSkinning"), programGBufMaterialSkinning);

		std::shared_ptr<se::graphics::Program> programGBufSplatmap;
		programGBufSplatmap = se::app::ShaderLoader::createProgram("res/shaders/vertexTerrain.glsl", "res/shaders/geometryTerrain.glsl", "res/shaders/fragmentGBufSplatmap.glsl");
		if (!programGBufSplatmap) {
			throw std::runtime_error("programGBufSplatmap not found");
		}
		mScene->repository.add(std::string("programGBufSplatmap"), programGBufSplatmap);

		auto shadowRenderer = static_cast<se::graphics::Renderer*>(mExternalTools->graphicsEngine->getRenderGraph().getNode("shadowRenderer"));
		auto gBufferRenderer = static_cast<se::graphics::Renderer*>(mExternalTools->graphicsEngine->getRenderGraph().getNode("gBufferRenderer"));

		auto passShadow = std::make_shared<se::graphics::Pass>(*shadowRenderer);
		passShadow->addBindable(programShadow);
		mScene->repository.add(std::string("passShadow"), passShadow);

		auto passShadowSkinning = std::make_shared<se::graphics::Pass>(*shadowRenderer);
		passShadowSkinning->addBindable(programShadowSkinning);
		mScene->repository.add(std::string("passShadowSkinning"), passShadowSkinning);

		auto passDefault = std::make_shared<se::graphics::Pass>(*gBufferRenderer);
		passDefault->addBindable(programGBufMaterial);
		se::app::ShaderLoader::addMaterialBindables(
			passDefault,
			se::app::Material{
				se::app::PBRMetallicRoughness{ glm::vec4(1.0f, 0.0f, 0.862f, 1.0f), chessTexture, 0.2f, 0.5f, {} },
				{}, 1.0f, {}, 1.0f, {}, glm::vec3(0.0f), se::graphics::AlphaMode::Opaque, 0.5f, false
			},
			programGBufMaterial
		);
		mScene->repository.add(std::string("passDefault"), passDefault);

		auto shaderDefault = std::make_shared<se::app::RenderableShader>(*mEventManager);
		shaderDefault->addPass(passShadow)
			.addPass(passDefault);
		mScene->repository.add(std::string("shaderDefault"), shaderDefault);
	}


	void Editor::destroyScene()
	{
		if (mScene) {
			delete mScene;
			mScene = nullptr;
		}
	}


	void Editor::notify(const se::app::IEvent& event)
	{
		tryCall(&Editor::onCloseEvent, event);
	}

// Private functions
	void Editor::onInput()
	{
		Application::onInput();
		mViewportControl->update();
	}


	void Editor::onUpdate(float deltaTime)
	{
		SOMBRA_DEBUG_LOG << "Init (" << deltaTime << ")";

		Application::onUpdate(deltaTime);

		const auto& windowData = mExternalTools->windowManager->getWindowData();
		ImGuiIO& io = ImGui::GetIO();
		io.DeltaTime = deltaTime;
		io.DisplaySize = ImVec2(static_cast<float>(windowData.width), static_cast<float>(windowData.height));
	}


	void Editor::onRender()
	{
		ImGui::SetCurrentContext(mImGuiContext);
		ImGui::NewFrame();

		// TODO: Remove ImGui demo window
		static bool show = true;
		ImGui::ShowDemoWindow(&show);

		mMenuBar->render();
		mEntityPanel->render();
		mComponentPanel->render();
		mRepositoryPanel->render();

		Application::onRender();
	}


	void Editor::onCloseEvent(const se::app::Event<se::app::Topic::Close>&)
	{
		stop();
	}

}
