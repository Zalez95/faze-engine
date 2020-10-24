#ifndef INDEX_BUFFER_H
#define INDEX_BUFFER_H

#include "Bindable.h"
#include "Constants.h"

namespace se::graphics {

	/**
	 * Class IndexBuffer, it's used for creating, binding and unbinding an
	 * Index Buffer Object.
	 *
	 * An Index Buffer Object is a buffer with the indices to the vertices
	 * that form the faces of the mesh
	 */
	class IndexBuffer : public Bindable
	{
	private:	// Attributes
		/** The id of the index buffer */
		unsigned int mBufferId;

		/** The TypeId of the indices of the buffer */
		TypeId mIndexType;

		/** The number of indices of the buffer */
		std::size_t mIndexCount;

	public:		// Functions
		/** Creates a new IndexBuffer */
		IndexBuffer();
		IndexBuffer(const IndexBuffer& other) = delete;
		IndexBuffer(IndexBuffer&& other);

		/** Class destructor */
		~IndexBuffer();

		/** Assignment operator */
		IndexBuffer& operator=(const IndexBuffer& other) = delete;
		IndexBuffer& operator=(IndexBuffer&& other);

		/** Resizes and sets the buffer data
		 *
		 * @param	data a pointer to the data of the buffer
		 * @param	type the TypeId of the elements
		 * @param	count the number of elements in the data array */
		template <typename T>
		void resizeAndCopy(const T* data, TypeId type, std::size_t count);

		/** Resizes and sets the buffer data
		 *
		 * @param	data a pointer to the data of the new buffer
		 * @param	size the size of the data buffer
		 * @param	type the TypeId of the elements
		 * @param	count the number of elements in the data array */
		void resizeAndCopy(
			const void* data, std::size_t size, TypeId type, std::size_t count
		);

		/** Sets the buffer data
		 *
		 * @param	data a pointer to the data of the buffer
		 * @param	count the number of elements in the data array
		 * @param	offset the offset into the buffer where the data will be
		 *			copied */
		template <typename T>
		void copy(const T* data, std::size_t count, std::size_t offset = 0);

		/** Sets the buffer data
		 *
		 * @param	data a pointer to the data of the new buffer
		 * @param	size the size of the data buffer
		 * @param	offset the offset into the buffer where the data will be
		 *			copied in bytes */
		void copy(const void* data, std::size_t size, std::size_t offset = 0);

		/** @return	the TypeId of the indices of the buffer */
		TypeId getIndexType() const { return mIndexType; };

		/** @return	the number of indices the buffer */
		std::size_t getIndexCount() const { return mIndexCount; };

		/** Binds te Index Buffer Object */
		void bind() const override;

		/** Unbinds the Index Buffer Object */
		void unbind() const override;
	};


	template <typename T>
	void IndexBuffer::resizeAndCopy(
		const T* data, TypeId type, std::size_t count
	) {
		resizeAndCopy(
			static_cast<const void*>(data), count * sizeof(T), type, count
		);
	}


	template <typename T>
	void IndexBuffer::copy(const T* data, std::size_t count, std::size_t offset)
	{
		copy(
			static_cast<const void*>(data), count * sizeof(T),
			offset * sizeof(T)
		);
	}

}

#endif		// INDEX_BUFFER_H