#include "avl.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

// Main function: builds AVL from stdin data and serializes to file
int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <output_dat_filename>" << endl;
        return 1;
    }

    const string output_filename = argv[1];
    AVLTree avlTree;

    // Read input (attendance + student_id) from stdin
    // Example input lines: "75 101", "80 102"
    string line;
    int attendance, studentId;
    vector<pair<int, int>> data;

    while (getline(cin, line)) {
        if (line.empty()) continue;
        stringstream ss(line);
        if (ss >> attendance >> studentId) {
            avlTree.insert(attendance, studentId);
        } else {
            cerr << "Invalid input line: " << line << endl;
        }
    }

    // Serialize the constructed AVL tree to file
    if (!avlTree.serialize(output_filename)) {
        cerr << "Error: Failed to serialize AVL tree to " << output_filename << endl;
        return 1;
    }

    cout << "AVL tree created and serialized successfully: " << output_filename << endl;
    return 0;
}