#ifndef NODE_H
#define NODE_H

#include <iostream>
#include <vector>
#include <map>
#include <functional>

namespace dom {

	struct Selector
	{
		std::string type;
		std::string id;
		std::vector<std::string> classNames;

		std::string toString() const;
	};

	class Node
	{
	private:
		std::string type;
		std::string innerHTML;
		Node *parent;
		std::vector<Node*> *children;
		std::map<std::string, std::string> *attributes;

		static void toHTML(Node* node, std::ostringstream& out, std::string prefix);

		friend class Tree;
		friend class Parser;
	public:
		Node(const std::string& type);

		std::string& getInnerHTML();
		
		void setInnerHTML(const std::string& html);

		Node* getParent();

		void setParent(Node* parent);

		void appendChild(Node* child);

		std::string getOpeningTag() const;

		std::string getClosingTag() const;

		std::string toString() const;

		std::string toHTML();
		
		bool matches(const Selector& selector);

		void forEachChild(std::function<void(const Node* child)>& lambda) const;

		void forEachAttribute(std::function<void(const std::string&, const std::string&)>& lambda) const;
		
		void setAttribute(const std::string& key, const std::string& value);
		std::string getAttribute(const std::string& key) const;
	};

} // namespace dom

#endif  // NODE_H