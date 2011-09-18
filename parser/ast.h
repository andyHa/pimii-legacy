#ifndef AST_H
#define AST_H

#include <vector>
#include <map>
#include "vm/env.h"

class Node {
private:
    std::map<const char*, std::wstring> stringAttributes;
    std::map<const char*, const char*> charAttributes;
    std::map<const char*, int> intAttributes;
    std::map<const char*, Node*> children;
    std::map<const char*, std::vector<Node*>* > childCollection;

public:

    const char* getCharAttribute(const char* name) {
        return charAttributes[name];
    }

    Node* setCharAttribute(const char* name, const char* value) {
        charAttributes[name] = value;
        return this;
    }

    int getIntAttribute(const char* name) {
        return intAttributes[name];
    }

    Node* setIntAttribute(const char* name, int value) {
        intAttributes[name] = value;
        return this;
    }

    std::wstring getStringAttribute(const char* name) {
        return stringAttributes[name];
    }

    Node* setStringAttribute(const char* name, const std::wstring& value) {
        stringAttributes[name] = value;
        return this;
    }

    Node* getChild(const char* name) {
       return children[name];
    }

    Node* setChild(const char* name, Node* child) {
        children[name] = child;
        return this;
    }

    Node* addChild(const char* collection, Node* child) {
        getChildren(collection)->push_back(child);
        return this;
    }

    std::vector<Node*>* getChildren(const char* collection) {
        std::vector<Node*>* children = childCollection[collection];
        if (children == NULL) {
            children = new std::vector<Node*>();
            childCollection[collection] = children;
        }
        return children;
    }

    void print(int intendation,  std::wstringstream& buf) {
        buf << std::wstring(intendation, ' ')
            << std::wstring(L"[")
            << std::endl;
        intendation += 3;
        for(std::map<const char*, int>::iterator
            iter = intAttributes.begin();
            iter != intAttributes.end();
            ++iter)
        {
            buf << std::wstring(intendation, ' ')
                << asString(std::string(iter->first))
                << std::wstring(L": ")
                << iter->second
                << std::endl;
        }
        for(std::map<const char*, const char*>::iterator
            iter = charAttributes.begin();
            iter != charAttributes.end();
            ++iter)
        {
            buf << std::wstring(intendation, ' ')
                << asString(std::string((iter->first)))
                << std::wstring(L": ")
                << asString(std::string((iter->second)))
                << std::endl;
        }
        for(std::map<const char*, std::wstring>::iterator
            iter = stringAttributes.begin();
            iter != stringAttributes.end();
            ++iter)
        {
            buf << std::wstring(intendation, ' ')
                << asString(std::string((iter->first)))
                << std::wstring(L": ")
                << iter->second
                << std::endl;
        }
        for(std::map<const char*, Node*>::iterator
            iter = children.begin();
            iter != children.end();
            ++iter)
        {
            buf << std::wstring(intendation, ' ')
                << asString(std::string((iter->first)))
                << std::wstring(L": ")
                << std::endl;
            if ( iter->second != NULL) {
                iter->second->print(intendation+3+strlen(iter->first), buf);
            }
        }
        for(std::map<const char*, std::vector<Node*>*>::iterator
            iter = childCollection.begin();
            iter != childCollection.end();
            ++iter)
        {
            buf << std::wstring(intendation, ' ')
                << asString(std::string((iter->first)))
                << std::wstring(L": {")
                << std::endl;
            std::vector<Node*>* children = iter->second;
            for(std::vector<Node*>::iterator
                listIter = children->begin();
                listIter != children->end();
                ++listIter) {
                if (*listIter != NULL) {
                    (*listIter)->print(intendation+4+strlen(iter->first), buf);
                }
            }
            buf << std::wstring(intendation, ' ')
                << std::wstring(L"}")
                << std::endl;
        }
        buf << std::wstring(intendation - 3, ' ')
            << std::wstring(L"]")
            << std::endl;
    }

    std::wstring toString() {
        std::wstringstream buffer;
        print(0, buffer);
        return buffer.str();
    }
};


class AST {
private:
    std::vector<Node*> children;
    Node* root;
public:
    AST() {
        root = createChild();
    }

    ~AST() {
        for(std::vector<Node*>::iterator
            iter = children.begin();
            iter != children.end();
            ++iter) {
            delete *iter;
        }
    }

    Node* createChild() {
        Node* result = new Node();
        children.push_back(result);
        return result;
    }

    void makeRoot(Node* root) {
        this->root = root;
    }

    std::wstring toString() {
        if (root == NULL) {
            return std::wstring(L"");
        }
        return root->toString();
    }
};

#endif // AST_H
