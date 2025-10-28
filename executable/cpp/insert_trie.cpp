#include "trie.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

// Trie class definition and core methods are now provided by trie.h

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <student_name> <student_id>" << endl;
        return 1;
    }

    const string name = argv[1];
    const string studentId = argv[2];
    
    // Corrected path to the data file
    const string trieFilename = "../serialized/name.dat"; 
    
    Trie trie;
    
    // Load the existing trie
    if (!trie.deserialize(trieFilename)) {
        // If deserialize fails (file doesn't exist yet), start with an empty tree.
        cerr << "Warning: Data file not found. Starting with a new Trie." << endl;
    }
    
    // Insert the new name and student ID
    trie.insert(name, studentId);
    
    // Serialize the updated trie
    if (trie.serialize(trieFilename)) {
        return 0; // Success (Python process relies on a clean exit)
    } else {
        return 1; // Failure
    }
}
