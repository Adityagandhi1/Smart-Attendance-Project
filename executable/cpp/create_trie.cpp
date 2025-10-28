#include "trie.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// Trie class definition and utility functions are now provided by trie.h

int main() {
    // Corrected file paths: ../data/students.csv assumes running from executable/cpp/
    // The executable needs to go up one directory (..) to 'executable', then into 'data/'.
    const string csvFilename = "../data/students.csv";
    const string trieFilename = "../serialized/name.dat";
    
    ifstream csvFile(csvFilename);
    if (!csvFile) {
        cerr << "Error opening CSV file: " << csvFilename << endl;
        return 1;
    }
    
    Trie trie;
    string line;
    bool isFirstLine = true;
    
    while (getline(csvFile, line)) {
        if (isFirstLine) {
            isFirstLine = false;
            // Assuming header is present, skip the first line.
            continue; 
        }
        
        // parseCSVLine() is defined in trie.h
        vector<string> fields = parseCSVLine(line); 
        
        // Check for minimum expected fields (student_id, name, rn)
        if (fields.size() >= 3) {
            const string& name = fields[1];
            const string& studentId = fields[0];
            trie.insert(name, studentId);
        }
    }
    
    if (trie.serialize(trieFilename)) {
        cout << "Trie has been successfully serialized to " << trieFilename << endl;
        return 0;
    } else {
        cerr << "Failed to serialize the trie" << endl;
        return 1;
    }
}
