#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>

using namespace std;

// Trie node structure
struct TrieNode {
    bool isEndOfName;
    unordered_map<char, shared_ptr<TrieNode>> children;
    vector<string> studentIds;

    TrieNode() : isEndOfName(false) {}
};

class Trie {
private:
    shared_ptr<TrieNode> root;

    // Helper function to serialize the trie (Binary I/O)
    void serializeHelper(ofstream& outFile, const shared_ptr<TrieNode>& node) {
        outFile.write(reinterpret_cast<const char*>(&node->isEndOfName), sizeof(bool));
        
        size_t numIds = node->studentIds.size();
        outFile.write(reinterpret_cast<const char*>(&numIds), sizeof(size_t));
        
        for (const auto& id : node->studentIds) {
            size_t idLength = id.length();
            outFile.write(reinterpret_cast<const char*>(&idLength), sizeof(size_t));
            outFile.write(id.c_str(), idLength);
        }
        
        size_t numChildren = node->children.size();
        outFile.write(reinterpret_cast<const char*>(&numChildren), sizeof(size_t));
        
        for (const auto& [ch, childNode] : node->children) {
            outFile.write(&ch, sizeof(char));
            serializeHelper(outFile, childNode);
        }
    }
    
    // Helper function to deserialize the trie (Binary I/O)
    shared_ptr<TrieNode> deserializeHelper(ifstream& inFile) {
        if (inFile.eof() || inFile.peek() == EOF) return nullptr;
        bool isEndOfName;
        inFile.read(reinterpret_cast<char*>(&isEndOfName), sizeof(bool));
        if (inFile.fail() && !inFile.eof()) return nullptr;

        auto node = make_shared<TrieNode>();
        node->isEndOfName = isEndOfName;

        size_t numIds;
        inFile.read(reinterpret_cast<char*>(&numIds), sizeof(size_t));
        for (size_t i = 0; i < numIds; ++i) {
            size_t idLength;
            inFile.read(reinterpret_cast<char*>(&idLength), sizeof(size_t));
            string studentId(idLength, '\0');
            inFile.read(&studentId[0], idLength);
            node->studentIds.push_back(studentId);
        }

        size_t numChildren;
        inFile.read(reinterpret_cast<char*>(&numChildren), sizeof(size_t));
        for (size_t i = 0; i < numChildren; ++i) {
            char ch;
            inFile.read(&ch, sizeof(char));
            node->children[ch] = deserializeHelper(inFile);
        }
        return node;
    }

    // Helper function to find the node corresponding to the prefix
    shared_ptr<TrieNode> searchNode(const string& prefix) {
        shared_ptr<TrieNode> current = root;
        for (char c : prefix) {
            if (current->children.find(c) == current->children.end()) {
                return nullptr;
            }
            current = current->children[c];
        }
        return current;
    }

    // New helper function to recursively collect all IDs under a node
    void collectIdsUnderNode(const shared_ptr<TrieNode>& node, vector<string>& result) {
        if (!node) return;

        if (node->isEndOfName) {
            result.insert(result.end(), node->studentIds.begin(), node->studentIds.end());
        }

        for (const auto& [ch, childNode] : node->children) {
            collectIdsUnderNode(childNode, result);
        }
    }

public:
    Trie() {
        root = make_shared<TrieNode>();
    }

    void insert(const string& name, const string& studentId) {
        shared_ptr<TrieNode> current = root;
        for (char c : name) {
            if (current->children.find(c) == current->children.end()) {
                current->children[c] = make_shared<TrieNode>();
            }
            current = current->children[c];
        }
        current->isEndOfName = true;
        current->studentIds.push_back(studentId);
    }

    bool deserialize(const string& filename) {
        ifstream inFile(filename, ios::binary);
        if (!inFile) return false;
        root = deserializeHelper(inFile);
        inFile.close();
        return true;
    }
    
    // Main search function to perform prefix lookup
    vector<string> search(const string& prefix) {
        shared_ptr<TrieNode> startNode = searchNode(prefix);
        vector<string> result;
        
        if (startNode) {
            collectIdsUnderNode(startNode, result);
        }
        return result;
    }

    bool serialize(const string& filename) {
        ofstream outFile(filename, ios::binary);
        if (!outFile) return false;
        serializeHelper(outFile, root);
        outFile.close();
        return true;
    }
};

vector<string> parseCSVLine(const string& line) {
    vector<string> result;
    string current;
    bool inQuotes = false;
    for (char c : line) {
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            result.push_back(current);
            current.clear();
        } else {
            current += c;
        }
    }
    result.push_back(current);
    return result;
}