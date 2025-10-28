from flask import Flask, request, jsonify, send_from_directory
import pandas as pd
import numpy as np
import cv2
import dlib
import os
import subprocess
from flask_cors import CORS

# --- CRITICAL PATH SETTINGS ---
PROJECT_ROOT = os.path.dirname(os.path.abspath(__file__))
# Executables are INSIDE executable/cpp, so we reference the directory directly
EXECUTABLE_DIR = os.path.join(PROJECT_ROOT, 'executable', 'cpp')
SERIALIZED_DIR = os.path.join(PROJECT_ROOT, 'executable', 'serialized')
DATA_DIR = os.path.join(PROJECT_ROOT, 'executable', 'data')

# --- LOAD DATAFRAMES ---
try:
    students_df = pd.read_csv(os.path.join(DATA_DIR, 'students.csv'))
    attendance_df = pd.read_csv(os.path.join(DATA_DIR, 'attendance.csv'))
except FileNotFoundError as e:
    print(f"[FATAL] Missing initial CSV files: {e}")
    students_df = pd.DataFrame(columns=['student_id', 'name', 'rn', 'facial_vector'])
    attendance_df = pd.DataFrame(columns=[
        'student_id', 'name', 'maths', 'english', 'chemistry',
        'physics', 'datastructure', 'total_attendance'
    ])

# Ensure folders exist
os.makedirs(SERIALIZED_DIR, exist_ok=True)

# --- CREATE AVL TREE FOR EACH SUBJECT ---
def create_avl_tree_for_subject(subject):
    # Data is loaded from CSV
    subject_data = attendance_df[['student_id', subject]].dropna()
    serialized_filename = os.path.join(SERIALIZED_DIR, f"{subject}.dat")

    # Command: Call executable directly inside its directory
    command = ['./create_avl', serialized_filename]
    input_lines = [f"{row[subject]} {row['student_id']}\n" for _, row in subject_data.iterrows()]
    input_str = ''.join(input_lines)

    try:
        # Run executable from its directory (executable/cpp)
        subprocess.run(command, input=input_str.encode(), cwd=EXECUTABLE_DIR, check=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        print(f"[✓] AVL tree for '{subject}' saved as '{serialized_filename}'")
    except subprocess.CalledProcessError as e:
        print(f"[✗] Error creating AVL for '{subject}': {e.stderr.decode()}")
    except Exception as e:
        print(f"[✗] Unexpected error: {str(e)}")

# --- INITIALIZE TREES AND TRIE ---
subjects = ['maths', 'english', 'chemistry', 'physics', 'datastructure', 'total_attendance']
print("\n--- INITIALIZING DATA STRUCTURES ---")

for subject in subjects:
    create_avl_tree_for_subject(subject)

try:
    # Run create_trie executable from its directory (executable/cpp)
    result = subprocess.run(
        ['./create_trie'],
        cwd=EXECUTABLE_DIR,
        check=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )
    print("[✓] Trie creation completed successfully.")
except subprocess.CalledProcessError as e:
    print(f"[✗] Error running create_trie: {e.stderr.decode()}")
except Exception as e:
    print(f"[✗] Unexpected error running create_trie: {str(e)}")

print("--- INITIALIZATION COMPLETE ---\n")

# --- FLASK APP SETUP ---
app = Flask(__name__)
CORS(app)

# --- LOAD DLIB MODELS (PLACEHOLDER) ---
# Placeholder function to bypass the webcam requirement for this environment
def capture_face_vector():
    # TEMPORARY FIX: Returns the vector of the first student (Aarav Mehta, ID 836004)
    if not students_df.empty and 'facial_vector' in students_df.columns and students_df.iloc[0]['facial_vector']:
        vector_str = students_df.iloc[0]['facial_vector']
        print(f"[WARN] Camera Bypassed. Using vector for ID {students_df.iloc[0]['student_id']}.")
        return np.array([float(x) for x in vector_str.split(',')])
    return np.zeros(128)

# --- ROUTES ---
@app.route('/')
def home():
    # Serves the frontend file directly
    return send_from_directory('frontend', 'attendance.html')

@app.route('/add_student', methods=['POST'])
def add_student():
    global students_df, attendance_df
    try:
        name = request.form['name']
        rn = request.form['rn']
        student_id = int(request.form['student_id'])

        face_vector = capture_face_vector()
        face_vector_str = ','.join(map(str, face_vector))

        # Update students.csv
        new_student_row = pd.DataFrame({'student_id': [student_id], 'name': [name], 'rn': [rn], 'facial_vector': [face_vector_str]})
        students_df = pd.concat([students_df, new_student_row], ignore_index=True)
        students_df.to_csv(os.path.join(DATA_DIR, 'students.csv'), index=False)

        # Insert into Trie (C++)
        subprocess.run(['./insert_trie', name, str(student_id)], cwd=EXECUTABLE_DIR, check=True)

        # Initialize attendance for the student
        new_attendance_row = pd.DataFrame({
            'student_id': [student_id], 'name': [name],
            'maths': [0], 'english': [0], 'chemistry': [0],
            'physics': [0], 'datastructure': [0], 'total_attendance': [0]
        })
        attendance_df = pd.concat([attendance_df, new_attendance_row], ignore_index=True)
        attendance_df.to_csv(os.path.join(DATA_DIR, 'attendance.csv'), index=False)

        # Update all AVL trees
        for subject in subjects:
            subprocess.run(
                ['./update_avl', os.path.join(SERIALIZED_DIR, f"{subject}.dat"), "0", str(student_id)],
                cwd=EXECUTABLE_DIR, check=True
            )

        return jsonify({'status': 'success', 'message': 'Student added successfully'})
    except subprocess.CalledProcessError as e:
        print(f"[ERROR] Add student failed: {e.stderr.decode()}")
        return jsonify({'status': 'error', 'message': 'Failed to insert in Trie'}), 500
    except Exception as e:
        print(f"[ERROR] Add student failed: {str(e)}")
        return jsonify({'status': 'error', 'message': str(e)}), 500

@app.route('/verify', methods=['POST'])
def verify():
    # This endpoint demonstrates face recognition (distance) and AVL update logic.
    try:
        subject = request.form['subject']
        
        # Capture facial vector (uses placeholder)
        face_vector = capture_face_vector() 
        face_vector_str = [str(x) for x in face_vector]
        
        # Call C++ distance executable
        command = ['./distance'] + face_vector_str
        result = subprocess.run(
            command,
            cwd=EXECUTABLE_DIR,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=True,
            text=True
        )
        student_id = int(result.stdout.strip())

        if student_id != -1:
            # 1. Update attendance in CSV
            student_id_str = str(student_id)
            idx = attendance_df.index[attendance_df['student_id'].astype(str) == student_id_str]
            
            if not idx.empty:
                attendance_df.loc[idx, subject] = attendance_df.loc[idx, subject] + 1
                attendance_df.loc[idx, 'total_attendance'] = attendance_df.loc[idx, 'total_attendance'] + 1
                attendance_df.to_csv(os.path.join(DATA_DIR, 'attendance.csv'), index=False)

                # Retrieve new attendance values for AVL update
                attendance_val = int(attendance_df.loc[idx, subject].iloc[0])
                total_val = int(attendance_df.loc[idx, 'total_attendance'].iloc[0])

                # 2. Call C++ update_avl (DSA: Update AVL trees)
                subprocess.run(
                    ['./update_avl', os.path.join(SERIALIZED_DIR, f"{subject}.dat"), str(attendance_val), student_id_str],
                    cwd=EXECUTABLE_DIR, check=True
                )
                subprocess.run(
                    ['./update_avl', os.path.join(SERIALIZED_DIR, "total_attendance.dat"), str(total_val), student_id_str],
                    cwd=EXECUTABLE_DIR, check=True
                )
                
            return jsonify({'status': 'success', 'message': f'Attendance marked for ID {student_id}'})
        else:
            return jsonify({'status': 'error', 'message': 'No student found'}), 404
            
    except subprocess.CalledProcessError as e:
        print(f"[ERROR] Verification failed (C++ distance or update failed): {e.stderr.decode()}")
        return jsonify({'status': 'error', 'message': 'Verification failed due to backend error'}), 500
    except Exception as e:
        print(f"[ERROR] Verification failed: {str(e)}")
        return jsonify({'status': 'error', 'message': str(e)}), 500

@app.route('/get_attendance', methods=['GET'])
def get_attendance():
    data = attendance_df.to_dict(orient='records')
    return jsonify({'status': 'success', 'data': data})

@app.route('/search_students', methods=['POST'])
def search_students():
    try:
        query = request.form.get('query', '')
        if not query:
            return jsonify({'status': 'error', 'message': 'Query is required'}), 400

        # Call C++ search_trie executable
        result = subprocess.run(
            ['./search_trie', query],
            cwd=EXECUTABLE_DIR,
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        ids = [str(x) for x in result.stdout.strip().split('\n') if x.isdigit()]
        matches = attendance_df[attendance_df['student_id'].astype(str).isin(ids)].to_dict(orient='records')
        return jsonify({'status': 'success', 'data': matches})
    except subprocess.CalledProcessError as e:
        print(f"[ERROR] Trie search failed: {e.stderr.decode()}")
        return jsonify({'status': 'error', 'message': 'Trie search failed'}), 500
    except Exception as e:
        print(f"[ERROR] General search error: {str(e)}")
        return jsonify({'status': 'error', 'message': str(e)}), 500

@app.route('/threshold_attendance', methods=['POST'])
def threshold_attendance():
    try:
        subject = request.form.get('subject', '')
        threshold = request.form.get('threshold', '')
        direction = request.form.get('direction', '')
        if not all([subject, threshold, direction]):
            return jsonify({'status': 'error', 'message': 'All fields required'}), 400

        dat_file = os.path.join(SERIALIZED_DIR, f"{subject}.dat")
        result = subprocess.run(
            ['./threshold', dat_file, str(threshold), direction],
            cwd=EXECUTABLE_DIR,
            check=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True
        )
        # Output is student IDs separated by newlines
        ids = [str(x) for x in result.stdout.strip().split('\n') if x.isdigit()]

        matches = attendance_df[attendance_df['student_id'].astype(str).isin(ids)].to_dict(orient='records')
        return jsonify({'status': 'success', 'data': matches})
        
    except subprocess.CalledProcessError as e:
        print(f"[ERROR] Threshold search failed: {e.stderr.decode()}")
        return jsonify({'status': 'error', 'message': 'Threshold search failed'}), 500
    except Exception as e:
        print(f"[ERROR] General threshold error: {str(e)}")
        return jsonify({'status': 'error', 'message': str(e)}), 500

if __name__ == '__main__':
    app.run(debug=True)
