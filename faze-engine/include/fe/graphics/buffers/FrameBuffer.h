#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

namespace fe { namespace graphics {

	/**
	 * Class FrameBuffer, it's used for creating, binding and unbinding Frame
	 * Buffer Objects
	 */
	class FrameBuffer
	{
	private:	// Attributes
		/** The ID of the Frame Buffer Object */
		unsigned int mBufferID;

	public:		// Functions
		/** Creates a new FrameBuffer */
		FrameBuffer();

		/** Class destructor */
		~FrameBuffer();

		/** Binds the Frame Buffer Object for reading data from it */
		void bindForReading() const;

		/** Binds the Frame Buffer Object for writing data to it */
		void bindForWriting() const;
	};

}}

#endif		// FRAME_BUFFER_H