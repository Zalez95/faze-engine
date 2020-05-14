#include <algorithm>
#include "se/graphics/Technique.h"
#include "se/graphics/Pass.h"

namespace se::graphics {

	Technique& Technique::addPass(PassSPtr pass)
	{
		mPasses.emplace_back(std::move(pass));
		return *this;
	}


	void Technique::processPasses(const PassCallback& callback)
	{
		for (auto& pass : mPasses) {
			callback(pass);
		}
	}


	Technique& Technique::removePass(PassSPtr pass)
	{
		mPasses.erase(std::remove(mPasses.begin(), mPasses.end(), pass), mPasses.end());

		return *this;
	}


	void Technique::submit(Renderable& renderable)
	{
		for (auto& pass : mPasses) {
			pass->submit(renderable);
		}
	}

}