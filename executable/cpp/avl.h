#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <map>
#include <cmath>

using namespace std;

// AVL Tree Node structure
struct AVLNode {
    int attendance;
    vector<int> studentIds;
    int height;
    shared_ptr<AVLNode> left;
    shared_ptr<AVLNode> right;

    AVLNode(int attend) 
        : attendance(attend), height(1), left(nullptr), right(nullptr) {}

    AVLNode(int attend, int studentId) 
        : attendance(attend), height(1), left(nullptr), right(nullptr) {
        studentIds.push_back(studentId);
    }
};

class AVLTree {
private:
    shared_ptr<AVLNode> root;
    bool studentFound;

    int getHeight(const shared_ptr<AVLNode>& node) {
        if (!node) return 0;
        return node->height;
    }

    int getBalanceFactor(const shared_ptr<AVLNode>& node) {
        if (!node) return 0;
        return getHeight(node->left) - getHeight(node->right);
    }

    void updateHeight(shared_ptr<AVLNode>& node) {
        if (!node) return;
        node->height = 1 + max(getHeight(node->left), getHeight(node->right));
    }

    shared_ptr<AVLNode> rightRotate(shared_ptr<AVLNode> y) {
        shared_ptr<AVLNode> x = y->left;
        shared_ptr<AVLNode> T2 = x->right;
        x->right = y;
        y->left = T2;
        updateHeight(y);
        updateHeight(x);
        return x;
    }

    shared_ptr<AVLNode> leftRotate(shared_ptr<AVLNode> x) {
        shared_ptr<AVLNode> y = x->right;
        shared_ptr<AVLNode> T2 = y->left;
        y->left = x;
        x->right = T2;
        updateHeight(x);
        updateHeight(y);
        return y;
    }

    shared_ptr<AVLNode> balanceNode(shared_ptr<AVLNode> node) {
        if (!node) return nullptr;
        updateHeight(node);
        int balance = getBalanceFactor(node);

        if (balance > 1 && getBalanceFactor(node->left) >= 0) return rightRotate(node);
        if (balance > 1 && getBalanceFactor(node->left) < 0) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }
        if (balance < -1 && getBalanceFactor(node->right) <= 0) return leftRotate(node);
        if (balance < -1 && getBalanceFactor(node->right) > 0) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }
        return node;
    }
    
    // Core insertion logic
    shared_ptr<AVLNode> insertNode(shared_ptr<AVLNode> node, int attendance, int studentId) {
        if (!node) return make_shared<AVLNode>(attendance, studentId);
        if (attendance < node->attendance) {
            node->left = insertNode(node->left, attendance, studentId);
        } else if (attendance > node->attendance) {
            node->right = insertNode(node->right, attendance, studentId);
        } else {
            if (find(node->studentIds.begin(), node->studentIds.end(), studentId) == node->studentIds.end()) {
                node->studentIds.push_back(studentId);
            }
            return node;
        }
        return balanceNode(node);
    }

    // --- Core BINARY Serialization Logic ---
    void serializeHelper(ofstream& outFile, const shared_ptr<AVLNode>& node) {
        if (!node) {
            int nullMarker = -1;
            outFile.write(reinterpret_cast<const char*>(&nullMarker), sizeof(int));
            return;
        }
        outFile.write(reinterpret_cast<const char*>(&node->attendance), sizeof(int));
        size_t numIds = node->studentIds.size();
        outFile.write(reinterpret_cast<const char*>(&numIds), sizeof(size_t));
        for (const auto& id : node->studentIds) {
            outFile.write(reinterpret_cast<const char*>(&id), sizeof(int));
        }
        outFile.write(reinterpret_cast<const char*>(&node->height), sizeof(int));
        serializeHelper(outFile, node->left);
        serializeHelper(outFile, node->right);
    }

    // --- Core BINARY Deserialization Logic ---
    shared_ptr<AVLNode> deserializeHelper(ifstream& inFile) {
        int attendance;
        inFile.read(reinterpret_cast<char*>(&attendance), sizeof(int));
        if (attendance == -1) return nullptr;
        
        auto node = make_shared<AVLNode>(attendance);
        size_t numIds;
        inFile.read(reinterpret_cast<char*>(&numIds), sizeof(size_t));
        for (size_t i = 0; i < numIds; ++i) {
            int studentId;
            inFile.read(reinterpret_cast<char*>(&studentId), sizeof(int));
            node->studentIds.push_back(studentId);
        }
        inFile.read(reinterpret_cast<char*>(&node->height), sizeof(int));
        node->left = deserializeHelper(inFile);
        node->right = deserializeHelper(inFile);
        return node;
    }
    
    // --- Threshold Search Helper Logic ---
    void collectStudentIds(const shared_ptr<AVLNode>& node, 
                          int threshold, 
                          int direction, 
                          map<int, vector<int>, greater<int>>& result) {
        if (!node) return;
        if ((direction > 0 && node->attendance >= threshold) || 
            (direction < 0 && node->attendance <= threshold)) {
            result[node->attendance].insert(result[node->attendance].end(), 
                                           node->studentIds.begin(), 
                                           node->studentIds.end());
        }
        collectStudentIds(node->left, threshold, direction, result);
        collectStudentIds(node->right, threshold, direction, result);
    }
    
    // Helper functions for updateAttendance (must be declared)
    shared_ptr<AVLNode> removeNode(shared_ptr<AVLNode> node);
    shared_ptr<AVLNode> findMin(shared_ptr<AVLNode> node);
    shared_ptr<AVLNode> removeStudentId(shared_ptr<AVLNode> node, int studentId);
    
public:
    AVLTree() : root(nullptr), studentFound(false) {}

    void insert(int attendance, int studentId) {
        root = insertNode(root, attendance, studentId);
    }
    
    // Public Binary I/O Functions
    bool serialize(const string& filename) {
        ofstream outFile(filename, ios::binary);
        if (!outFile) return false;
        serializeHelper(outFile, root);
        outFile.close();
        return true;
    }

    bool deserialize(const string& filename) {
        ifstream inFile(filename, ios::binary);
        if (!inFile) return false;
        root = deserializeHelper(inFile);
        inFile.close();
        return true;
    }
    
    // Function for update_avl.cpp
    bool updateAttendance(int studentId, int newAttendance) {
        studentFound = false;
        root = removeStudentId(root, studentId);
        root = insertNode(root, newAttendance, studentId);
        return studentFound;
    }
    
    // Function for threshold.cpp
    vector<int> getStudentIdsByThreshold(int threshold, int direction) {
        map<int, vector<int>, greater<int>> attendanceMap;
        collectStudentIds(root, threshold, direction, attendanceMap);
        vector<int> result;
        for (const auto& [attendance, ids] : attendanceMap) {
            result.insert(result.end(), ids.begin(), ids.end());
        }
        return result;
    }
};

// --- IMPLEMENTATIONS (Kept here for simplicity, typically go in a CPP file) ---

shared_ptr<AVLNode> AVLTree::removeNode(shared_ptr<AVLNode> node) {
    if (!node->left) return node->right;
    if (!node->right) return node->left;
    shared_ptr<AVLNode> successor = findMin(node->right);
    node->attendance = successor->attendance;
    node->studentIds = successor->studentIds;
    node->right = removeNode(successor);
    return balanceNode(node);
}

shared_ptr<AVLNode> AVLTree::findMin(shared_ptr<AVLNode> node) {
    if (!node) return nullptr;
    while (node->left) node = node->left;
    return node;
}

shared_ptr<AVLNode> AVLTree::removeStudentId(shared_ptr<AVLNode> node, int studentId) {
    if (!node) return nullptr;
    
    auto it = find(node->studentIds.begin(), node->studentIds.end(), studentId);
    if (it != node->studentIds.end()) {
        studentFound = true;
        node->studentIds.erase(it);
        if (node->studentIds.empty()) return removeNode(node);
        return node;
    }
    
    node->left = removeStudentId(node->left, studentId);
    node->right = removeStudentId(node->right, studentId);
    return balanceNode(node);
}

AVLTree buildAVLTree() {
    AVLTree tree;
    int attendance, studentId;
    while (cin >> attendance >> studentId) {
        if (attendance < 0 || attendance > 100) continue;
        tree.insert(attendance, studentId);
    }
    return tree;
}