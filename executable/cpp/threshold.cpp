#include "avl.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <dat_file_name> <threshold> <direction>" << endl;
        cerr << "  direction: 1 for above threshold, -1 for below threshold" << endl;
        return 1;
    }

    const string datFilename = argv[1];
    int threshold, direction;

    try {
        threshold = stoi(argv[2]);
        direction = stoi(argv[3]);
        if (direction != 1 && direction != -1) {
            cerr << "Direction must be 1 (above) or -1 (below)" << endl;
            return 1;
        }
    } catch (const exception& e) {
        cerr << "Error parsing arguments: " << e.what() << endl;
        return 1;
    }

    ifstream fileCheck(datFilename);
    if (!fileCheck.is_open()) {
        cerr << "File not found: " << datFilename << endl;
        return 1;
    }
    fileCheck.close();

    AVLTree avlTree;
    if (!avlTree.deserialize(datFilename)) {
        cerr << "Failed to deserialize the AVL tree from " << datFilename << endl;
        return 1;
    }

    vector<int> studentIds = avlTree.getStudentIdsByThreshold(threshold, direction);

    if (studentIds.empty()) {
        cout << "-1" << endl;
    } else {
        for (int id : studentIds) {
            cout << id << endl;
        }
    }

    return 0;
}