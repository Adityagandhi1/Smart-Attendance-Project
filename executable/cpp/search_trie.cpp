#include "trie.h" // Includes TrieNode and Trie definitions
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <map>

using namespace std;

int main(int argc, char* argv[]) {
if (argc != 2) {
cerr << "Usage: " << argv[0] << " <name_to_search>" << endl;
return 1;
}

const string nameToSearch = argv[1];
// Correcting the file path: ../serialized/name.dat from the cpp directory
const string trieFilename = "../serialized/name.dat";

Trie trie;
// Check if the file exists and deserialize
ifstream fileCheck(trieFilename);
if (!fileCheck.is_open()) {
cerr << "Error: Trie data file not found at " << trieFilename << endl;
return 1;
}
fileCheck.close();
// Deserialize the trie
if (!trie.deserialize(trieFilename)) {
cerr << "Failed to deserialize the trie from " << trieFilename << endl;
return 1;
}
// Search for the name
vector<string> studentIds = trie.search(nameToSearch);
// Output student IDs
if (studentIds.empty()) {
cout << "-1" << endl;
} else {
for (const auto& id : studentIds) {
cout << id << endl;
}
}

return 0;
}