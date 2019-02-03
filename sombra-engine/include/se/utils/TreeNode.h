#ifndef TREE_NODE_H
#define TREE_NODE_H

#include <deque>
#include <memory>

namespace se::utils {

	/** The algorithms to use for visiting the TreeNodes */
	enum class Traversal { BFS, DFSPreOrder };


	/**
	 * Class TreeNode, it's a hierarchical data structure in which each TreeNode
	 * holds part of the data. A tree is a kind of acyclic graph with just one
	 * root node. Each node of the tree can be seen as a subtree, and its
	 * represented in a parent-child-sibling structure for minimizing the memory
	 * storage requirements.
	 * The TreeNode also has functions for iterating it with different
	 * algorithms and also finding Nodes by the data that each hold
	 */
	template <typename T>
	class TreeNode
	{
	public:		// Nested types
		/** Class TNIterator, the class used to iterate through the
		 * descendant TreeNodes of a TreeNode */
		template <bool isConst, Traversal t>
		class TNIterator
		{
		public:		// Nested types
			template <bool isConst2, Traversal t2>
			friend class TNIterator;

			using TreeNodeType = std::conditional_t<isConst,
				const TreeNode, TreeNode
			>;

			using difference_type	= long;
			using value_type		= TreeNodeType;
			using pointer			= value_type*;
			using reference			= value_type&;
			using iterator_category = std::forward_iterator_tag;

		private:	// Attributes
			/** A pointer to the current TreeNode of the iterator */
			TreeNodeType* mTreeNode;

			/** The deque needed for traversing the TreeNodes */
			std::deque<TreeNodeType*> mTreeNodeDeque;

		public:		// Functions
			/** Creates a new TNIterator located at the given TreeNode
			 *
			 * @param	treeNode a pointer to the TreeNode of the iterator */
			TNIterator(TreeNodeType* treeNode) : mTreeNode(treeNode) {};

			/** Implicit conversion operator between const TNIterator and non
			 * const TNIterator
			 *
			 * @return	the new TNIterator with a different template const
			 *			type */
			operator TNIterator<!isConst, t>() const;

			/** @return	a reference to the current TreeNode that the iterator is
			 *			pointing at */
			TreeNodeType& operator*() const { return *mTreeNode; };

			/** @return	a pointer to the current TreeNode that the iterator is
			 *			pointing at */
			TreeNodeType* operator->() { return mTreeNode; };

			/** Compares the given TNIterators
			 *
			 * @param	it1 the first TNIterator to compare
			 * @param	it2 the second TNIterator to compare
			 * @return	true if both iterators are equal, false otherwise */
			friend bool operator==(const TNIterator& it1, const TNIterator& it2)
			{ return it1.mTreeNode == it2.mTreeNode; };

			/** Compares the given TNIterators
			 *
			 * @param	it1 the first TNIterator to compare
			 * @param	it2 the second TNIterator to compare
			 * @return	true if both iterators are different, false otherwise */
			friend bool operator!=(const TNIterator& it1, const TNIterator& it2)
			{ return !(it1 == it2); };

			/** Preincrement operator
			 *
			 * @return	a reference to the current iterator after its
			 *			incrementation */
			TNIterator& operator++();

			/** Postincrement operator
			 *
			 * @return	a copy of the current iterator with the previous value
			 *			to the incrementation */
			TNIterator operator++(int);
		private:
			/** Calculates the next TreeNode to point using the BFS algorithm */
			void nextBFS();

			/** Calculates the next TreeNode to point using the DFS pre-order
			 * algorithm */
			void nextDFSPreOrder();
		};

		using value_type = T;
		using size_type = std::size_t;
		template <Traversal t> using iterator = TNIterator<false, t>;
		template <Traversal t> using const_iterator = TNIterator<true, t>;

	private:	// Attributes
		/** A pointer to the parent TreeNode of the current one */
		TreeNode* mParent;

		/** A pointer to the child TreeNode of the current one */
		std::unique_ptr<TreeNode> mChild;

		/** A pointer to the sibling TreeNode of the current one */
		std::unique_ptr<TreeNode> mSibling;

		/** The data of the TreeNode */
		T mData;

	public:		// Functions
		/** Creates a new TreeNode
		 *
		 * @param	data the data of the TreeNode */
		TreeNode(const T& data) : mParent(nullptr), mData(data) {};

		/** Creates a new TreeNode
		 *
		 * @param	data the data of the TreeNode */
		TreeNode(T&& data) : mParent(nullptr), mData(std::move(data)) {};

		/** Copy constructor
		 *
		 * @param	other the other TreeNode to copy */
		TreeNode(const TreeNode& other);
		TreeNode(TreeNode&& other) = default;

		/** Class destructor */
		~TreeNode() = default;

		/** Assignment operator
		 *
		 * @param	other the other TreeNode to copy */
		TreeNode& operator=(const TreeNode& other);
		TreeNode& operator=(TreeNode&& other) = default;

		/** Compares the given TreeNodes
		 *
		 * @param	tn1 the first TreeNode to compare
		 * @param	tn2 the second TreeNode to compare
		 * @return	true if both TreeNodes are equal, false otherwise */
		friend bool operator==(const TreeNode& tn1, const TreeNode& tn2)
		{ return tn1.mData == tn2.mData; };

		/** Compares the given TreeNodes
		 *
		 * @param	tn1 the first TreeNode to compare
		 * @param	tn2 the second TreeNode to compare
		 * @return	true if both TreeNodes are different, false otherwise */
		friend bool operator!=(const TreeNode& tn1, const TreeNode& tn2)
		{ return !(tn1 == tn2); };

		/** @return	the initial iterator of the TreeNode */
		template <Traversal t = Traversal::BFS>
		iterator<t> begin() { return iterator<t>(this); }

		/** @return	the initial iterator of the TreeNode */
		template <Traversal t = Traversal::BFS>
		const_iterator<t> begin() const { return const_iterator<t>(this); }

		/** @return	the final iterator of the TreeNode */
		template <Traversal t = Traversal::BFS>
		iterator<t> end() { return iterator<t>(nullptr); }

		/** @return	the final iterator of the TreeNode */
		template <Traversal t = Traversal::BFS>
		const_iterator<t> end() const { return const_iterator<t>(nullptr); }

		/** @return	the number of TreeNodes in the current tree
		 *			(current node + descendants) */
		template <Traversal t = Traversal::BFS>
		size_type size() const;

		/** @return	a pointer to the parent TreeNode of the current one */
		TreeNode* getParent() { return mParent; };

		/** @return	a pointer to the parent TreeNode of the current one */
		const TreeNode* getParent() const { return mParent; };

		/** @return	the data of the TreeNode */
		T& getData() { return mData; };

		/** @return	the data of the TreeNode */
		const T& getData() const { return mData; };

		/** Searchs a descendant TreeNode with the same data than the given one
		 *
		 * @param	data the data to search in the TreeNode
		 * @return	an iterator to the TreeNode where the data has been found,
		 *			or to end if it wasn't found */
		template <Traversal t = Traversal::BFS>
		iterator<t> find(const T& data);

		/** Searchs a descendant TreeNode with the same data than the given one
		 *
		 * @param	data the data to search in the TreeNode
		 * @return	a const_iterator to the TreeNode where the data has been
		 *			found, or to end if it wasn't found */
		template <Traversal t = Traversal::BFS>
		const_iterator<t> find(const T& data) const;

		/** Adds a descendant TreeNode as a child of the TreeNode pointed by
		 * the given iterator
		 *
		 * @param	parentIt an iterator to the parent TreeNode of the one to
		 *			add
		 * @param	data the data of the TreeNode to add
		 * @return	an iterator to the new TreeNode, end if the TreeNode
		 *			couldn't be added
		 * @note	if the parentIt isn't valid the TreeNode won't be added */
		template <Traversal t = Traversal::BFS>
		iterator<t> insert(iterator<t> parentIt, const T& data);

		/** Adds a descendant TreeNode as a child of the TreeNode pointed by
		 * the given iterator
		 *
		 * @param	parentIt an iterator to the parent TreeNode of the one to
		 *			add
		 * @param	data the data to move to the new TreeNode
		 * @return	an iterator to the new TreeNode, end if the TreeNode
		 *			couldn't be added
		 * @note	if the parentIt isn't valid the TreeNode won't be added */
		template <Traversal t = Traversal::BFS>
		iterator<t> insert(iterator<t> parentIt, T&& data);

		/** Removes the TreeNode pointed by the given iterator
		 *
		 * @param	it an iterator to the TreeNode to remove
		 * @return	an iterator to the following TreeNode of the one erased */
		template <Traversal t = Traversal::BFS>
		iterator<t> erase(iterator<t> it);
	};

}

#include "TreeNode.inl"

#endif		// TREE_NODE_H
