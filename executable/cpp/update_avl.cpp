#include "avl.h" // Includes AVLNode and AVLTree definitions
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <map>

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <dat_file_name> <new_attendance> <student_id>" << endl;
        return 1;
    }

    const string datFilename = argv[1];
    int newAttendance, studentId;
    
    try {
        newAttendance = stoi(argv[2]);
        studentId = stoi(argv[3]);
        
        if (newAttendance < 0 || newAttendance > 100) {
            cerr << "Attendance should be between 0 and 100" << endl;
            return 1;
        }
    } catch (const exception& e) {
        cerr << "Error parsing arguments: " << e.what() << endl;
        return 1;
    }
    
    AVLTree avlTree;
    
    // Attempt to deserialize (load) the existing file
    bool deserialize_success = avlTree.deserialize(datFilename);
    
    if (!deserialize_success) {
        // If file doesn't exist, treat it as a new file and insert the student
        cout << "File not found or failed to load. Initializing new AVL tree for student ID " << studentId << "." << endl;
    } 

    // Update the attendance for the student ID. 
    // This handles both inserting a new student and updating an existing one.
    bool studentFoundBeforeUpdate = avlTree.updateAttendance(studentId, newAttendance);
    
    if (studentFoundBeforeUpdate) {
        cout << "Updated attendance for student ID " << studentId << " to " << newAttendance << endl;
    } else {
        cout << "Inserted new entry for student ID " << studentId << " with attendance " << newAttendance << endl;
    }
    
    // Serialize the updated AVL tree back to the file
    if (!avlTree.serialize(datFilename)) {
        cerr << "Failed to serialize the updated AVL tree to " << datFilename << endl;
        return 1;
    }
    
    return 0;
}

