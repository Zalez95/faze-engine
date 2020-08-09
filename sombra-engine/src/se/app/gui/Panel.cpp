#include "se/app/gui/Panel.h"
#include "se/app/gui/GUIManager.h"
#include "se/app/GraphicsManager.h"

namespace se::app {

	Panel::Panel(GUIManager* guiManager) :
		mGUIManager(guiManager), mSprite(mPosition, mSize)
	{
		mSprite.setZIndex(mZIndex);
		auto technique2D = mGUIManager->getGraphicsManager().getRepository().find<std::string, graphics::Technique>("technique2D");
		if (technique2D) {
			mSprite.addTechnique(technique2D);
		}

		mIsVisible = false;
		setVisibility(true);
	}


	Panel::Panel(const Panel& other) :
		ComposedComponent(other),
		mGUIManager(other.mGUIManager),
		mSprite(other.mSprite)
	{
		mIsVisible = false;
		setVisibility(other.mIsVisible);
	}


	Panel::~Panel()
	{
		setVisibility(false);
	}


	Panel& Panel::operator=(const Panel& other)
	{
		ComposedComponent::operator=(other);
		mGUIManager = other.mGUIManager;
		mSprite = other.mSprite;

		mIsVisible = false;
		setVisibility(other.mIsVisible);

		return *this;
	}


	void Panel::setPosition(const glm::vec2& position)
	{
		ComposedComponent::setPosition(position);
		mSprite.setPosition(mPosition);
	}


	void Panel::setSize(const glm::vec2& size)
	{
		ComposedComponent::setSize(size);
		mSprite.setSize(mSize);
	}


	void Panel::setZIndex(unsigned char zIndex)
	{
		ComposedComponent::setZIndex(zIndex);
		mSprite.setZIndex(mZIndex);
	}


	void Panel::setVisibility(bool isVisible)
	{
		bool wasVisible = mIsVisible;
		ComposedComponent::setVisibility(isVisible);

		if (wasVisible && !mIsVisible) {
			mGUIManager->getGraphicsManager().getGraphicsEngine().removeRenderable(&mSprite);
		}
		else if (!wasVisible && mIsVisible) {
			mGUIManager->getGraphicsManager().getGraphicsEngine().addRenderable(&mSprite);
		}
	}


	void Panel::setColor(const glm::vec4& color)
	{
		mSprite.setColor(color);
	}

}
