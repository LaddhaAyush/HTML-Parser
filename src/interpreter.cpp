//This code defines an Interpreter class for interacting with a DOM tree (Document Object Model).
//  It enables executing commands such as printing, saving, and selecting DOM nodes.
//  The Interpreter class is part of a larger DOM manipulation engine, which allows users to manipulate and query the structure of a web page's DOM.
// The Interpreter class is designed to work with a Tree object, which represents the DOM tree structure.

#include "interpreter.h"
#include "util.h"

#include <fstream>

namespace dom {

    const std::regex Interpreter::SELECTOR_CMD_FORMAT{R"(\$\(".*"\))"};
    const std::regex Interpreter::CREATE_CMD_FORMAT{R"(create\s+.*)"};

    Interpreter::Interpreter(Tree* tree) : tree(tree) {}

    void Interpreter::resolveCmd(std::string& cmd) const
    {
        std::vector<std::string> cmds = util::tokenize(cmd);

        if (cmds.at(0) == "print") tree->print();
        else if (cmds.at(0) == "save")
        {
            if (cmds.size() != 2)
            {
                util::logSyntaxError("Usage: save <output-file>");
                return;
            }
            
            std::ofstream fd(cmds.at(1));
            fd << "<!DOCTYPE html>" << std::endl;
            fd << tree->toHTML();
            fd.close();
            Log("Tree saved to " << cmds.at(1));
        }
        // Handle create command
        else if (cmds.at(0) == "create")
        {
            if (cmds.size() < 2)
            {
                util::logSyntaxError("Usage: create <element-spec>");
                return;
            }
            
            std::string elementSpec = cmds.at(1);
            Node* newNode = tree->createElement(elementSpec);
            
            // If there's a parent specified (create <element-spec> within <parent-selector>)
            if (cmds.size() > 2 && cmds.at(2) == "within")
            {
                if (cmds.size() < 4)
                {
                    util::logSyntaxError("Usage: create <element-spec> within <parent-selector>");
                    return;
                }
                
                std::string parentSelector = cmds.at(3);
                // Remove quotes if present
                if (parentSelector.front() == '"' && parentSelector.back() == '"')
                {
                    parentSelector = parentSelector.substr(1, parentSelector.length() - 2);
                }
                
                Node* parent = tree->match(parentSelector);
                if (parent == nullptr)
                {
                    Log("Parent not found: " << parentSelector);
                    delete newNode; // Clean up
                    return;
                }
                
                parent->appendChild(newNode);
                Log("Created element " << newNode->toString() << " within " << parent->toString());
            }
            else
            {
                // Add to document root by default
                Node* documentElement = tree->match(std::string("@document"));
                if (documentElement == nullptr || documentElement->children == nullptr || documentElement->children->empty())
                {
                    Log("Document element not found");
                    delete newNode; // Clean up
                    return;
                }
                
                Node* bodyElement = nullptr;
                
                // Try to find body first
                std::string bodySelector = "@body";
                bodyElement = tree->match(bodySelector);
                
                if (bodyElement == nullptr)
                {
                    // If not found, add to document's first child (should be html)
                    bodyElement = documentElement->children->at(0);
                }
                
                bodyElement->appendChild(newNode);
                Log("Created element " << newNode->toString() << " in document");
            }
        }
    }

    void Interpreter::resolveSubCmd(std::string& subCmd, Node* selected) const
    {
        // Split command to handle commands with arguments
        std::vector<std::string> cmdParts = util::tokenize(subCmd, ' ');
        std::string cmd = cmdParts.at(0);
        
        if (cmd == "parent")
        {
            auto parent = selected->getParent();
            if (parent == nullptr)
            {
                Log("Root node has no parent.");
                return;
            }

            Log(selected->getParent()->toString());
        }
        else if (cmd == "children")
        {
            std::function<void(const Node* child)> lambda = [](const Node* child) {
                Log("- " << child->toString());
            };

            selected->forEachChild(lambda);
        }
        else if (cmd == "attrs")
        {
            std::function<void(const std::string&, const std::string&)> lambda = 
                [](const std::string& key, const std::string& value) {
                    std::cout << key << ": " << value << std::endl;
                };

            selected->forEachAttribute(lambda);
        }
        else if (cmd == "innerhtml")
        {
            auto innerHTML = selected->getInnerHTML();
            if (!util::isBlank(innerHTML))
                Log(innerHTML);
        }
        // New command: setattr key value
        else if (cmd == "setattr")
        {
            if (cmdParts.size() < 3)
            {
                Log("Usage: setattr <key> <value>");
                return;
            }
            
            std::string key = cmdParts.at(1);
            
            // Join the rest of the parts as the value (in case value has spaces)
            std::string value;
            for (size_t i = 2; i < cmdParts.size(); i++)
            {
                value += cmdParts.at(i);
                if (i < cmdParts.size() - 1)
                    value += " ";
            }
            
            selected->setAttribute(key, value);
            Log("Set attribute " << key << " to " << value);
        }
        // New command: sethtml <content>
        else if (cmd == "sethtml")
        {
            if (cmdParts.size() < 2)
            {
                Log("Usage: sethtml <content>");
                return;
            }
            
            // Join the rest of the parts as the content
            std::string content;
            for (size_t i = 1; i < cmdParts.size(); i++)
            {
                content += cmdParts.at(i);
                if (i < cmdParts.size() - 1)
                    content += " ";
            }
            
            selected->setInnerHTML(content);
            Log("Set inner HTML to: " << content);
        }
        else Log("Unknown sub-command: " << subCmd);
    }

    Node* Interpreter::select(std::string& cmd) const
    {
        // Check if first function call matches the selector command format
        if (regex_match(cmd.substr(0, cmd.find_first_of(')') + 1), SELECTOR_CMD_FORMAT))
        {
            std::string selector = cmd.substr(3, cmd.find_first_of(')') - 4);
            return tree->match(selector);
        }
        else
        {
            Log("Invalid syntax: " << cmd);
            return nullptr;
        }   
    }

} // namespace dom
