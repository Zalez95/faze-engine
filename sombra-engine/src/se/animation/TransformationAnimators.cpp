#include "se/animation/TransformationAnimators.h"

namespace se::animation {

	float TransformationAnimator::getLoopTime() const
	{
		return mLoopTime;
	}


	void TransformationAnimator::setLoopTime(float loopTime)
	{
		mLoopTime = loopTime;
	}


	void TransformationAnimator::restartAnimation()
	{
		mAccumulatedTime = 0.0f;
	}


	void TransformationAnimator::resetNodesAnimatedState()
	{
		for (AnimatedNode& animatedNode : mNodes) {
			animatedNode.node->getData().animated = false;
		}
	}


	void TransformationAnimator::updateNodesWorldTransforms()
	{
		for (AnimatedNode& animatedNode : mNodes) {
			if (!animatedNode.node->getData().worldTransformsUpdated) {
				updateWorldTransforms(*animatedNode.node);
			}
		}
	}


	void TransformationAnimator::addNode(TransformationType type, AnimationNode* node)
	{
		mNodes.push_back({ type, node });
	}


	Vec3Animator::Vec3Animator(Vec3AnimationSPtr animation) : mAnimation(animation)
	{
		setLoopTime(mAnimation->getLength());
	}


	void Vec3Animator::animate(float elapsedTime)
	{
		mAccumulatedTime = std::fmod(mAccumulatedTime + elapsedTime, getLoopTime());
		glm::vec3 transformation = mAnimation->interpolate(mAccumulatedTime);

		for (auto& animationNode : mNodes) {
			switch (animationNode.type) {
				case TransformationType::Translation:
					animationNode.node->getData().localTransforms.position = transformation;
					animationNode.node->getData().animated = true;
					animationNode.node->getData().worldTransformsUpdated = false;
					break;
				case TransformationType::Scale:
					animationNode.node->getData().localTransforms.scale = transformation;
					animationNode.node->getData().animated = true;
					animationNode.node->getData().worldTransformsUpdated = false;
					break;
				default:
					break;
			}
		}
	}


	QuatAnimator::QuatAnimator(QuatAnimationSPtr animation) : mAnimation(animation)
	{
		setLoopTime(mAnimation->getLength());
	}


	void QuatAnimator::animate(float elapsedTime)
	{
		mAccumulatedTime = std::fmod(mAccumulatedTime + elapsedTime, getLoopTime());
		glm::quat transformation = mAnimation->interpolate(mAccumulatedTime);

		for (auto& animationNode : mNodes) {
			switch (animationNode.type) {
				case TransformationType::Rotation:
					animationNode.node->getData().localTransforms.orientation = transformation;
					animationNode.node->getData().animated = true;
					animationNode.node->getData().worldTransformsUpdated = false;
					break;
				default:
					break;
			}
		}
	}

}
