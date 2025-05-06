#ifndef TREE_H
#define TREE_H

#include "node.h"

namespace dom {

	class Tree
	{
	private:
		Node* root;
	public:
		Tree(Node* root);

		Node* match(std::string& identifier);

		Node* createElement(std::string& elementSpec);

		void print();

		std::string toHTML();

		bool isBuilt();
	private:
		static std::string generateTreePrefix(std::string prefix, bool isTail);

		static void print(Node* current, std::string prefix, bool isTail);

		static Selector tokenizeSelector(std::string& selector);

		Node* match(Node* node, Selector& selPair);
	};

} // namespace dom

#endif // TREE_H