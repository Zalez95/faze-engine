#include <string>
#include <stdexcept>
#include "se/graphics/core/FrameBuffer.h"
#include "se/graphics/core/Texture.h"
#include "GLWrapper.h"

namespace se::graphics {

	FrameBuffer::FrameBuffer(FrameBufferTarget target) : mTarget(target)
	{
		// Create the FBO
		GL_WRAP( glGenFramebuffers(1, &mBufferId) );
		SOMBRA_TRACE_LOG << "Created FBO " << mBufferId;
	}


	FrameBuffer::FrameBuffer(FrameBuffer&& other) :
		mBufferId(other.mBufferId), mTarget(other.mTarget)
	{
		other.mBufferId = 0;
	}


	FrameBuffer::~FrameBuffer()
	{
		if (mBufferId != 0) {
			GL_WRAP( glDeleteFramebuffers(1, &mBufferId) );
			SOMBRA_TRACE_LOG << "Deleted FBO " << mBufferId;
		}
	}


	FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other)
	{
		if (mBufferId != 0) {
			GL_WRAP( glDeleteFramebuffers(1, &mBufferId) );
			SOMBRA_TRACE_LOG << "Deleted FBO " << mBufferId;
		}

		mBufferId = other.mBufferId;
		mTarget = other.mTarget;
		other.mBufferId = 0;

		return *this;
	}


	FrameBuffer& FrameBuffer::getDefaultFrameBuffer()
	{
		static FrameBuffer instance(0, FrameBufferTarget::Both);
		return instance;
	}


	void FrameBuffer::attach(
		const Texture& texture,
		FrameBufferAttachment attachment, unsigned int colorIndex
	) const
	{
		GLenum glAttachment = GL_COLOR_ATTACHMENT0;
		switch (attachment) {
			case FrameBufferAttachment::Stencil:	glAttachment = GL_STENCIL_ATTACHMENT;				break;
			case FrameBufferAttachment::Depth:		glAttachment = GL_DEPTH_ATTACHMENT;					break;
			case FrameBufferAttachment::Color:		glAttachment = GL_COLOR_ATTACHMENT0 + colorIndex;	break;
		}

		bind();
		GL_WRAP( glFramebufferTexture2D(
			GL_FRAMEBUFFER, glAttachment,
			GL_TEXTURE_2D, texture.getTextureId(), 0
		) );

		GL_WRAP( GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER) );
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			throw std::runtime_error("FrameBuffer error: status 0x" + std::to_string(status));
		}
		unbind();
	}


	void FrameBuffer::bind() const
	{
		GL_WRAP( glBindFramebuffer(toGLFrameBufferTarget(mTarget), mBufferId) );
	}


	void FrameBuffer::unbind() const
	{
		GL_WRAP( glBindFramebuffer(toGLFrameBufferTarget(mTarget), 0) );
	}

}
