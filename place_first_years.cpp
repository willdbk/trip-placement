#include "header.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <random>
#include <assert.h>
#include <fstream>

#include <vector>
#include <algorithm>

using namespace std;

/********************************************************************/
/* global variables */
vector<trip> trips;
vector<trip> best_trips;
vector<student> students;
int students_placed;
vector<trip*> open_trips;


/********************************************************************/
/* forward declarations of functions */
void read_trips();
void read_students();
void write_placements();

int gender_string_to_int(string gender_string);
string gender_int_to_string(int gender_int);
int trip_name_to_index(string name);
string get_csv_row_for_student(int index);

void assign_students();
int get_score();
void assign_least_requested_first();
void count_trip_requests();
int get_random_student();
int get_best_student_for_trip(int trip_index);
bool place_student_on_trip(int student_index, int trip_index);
void assign_students_randomly();
void reset_placements();


int request_ratio_cmp(trip* t1, trip* t2);
int trip_buffer_cmp(trip* t1, trip* t2);

void print_trips();
void print_open_trips();
void print_students();

/********************************************************************/
/* functions */
/********************************************************************/

int main(int argc, char** argv) {
    reset_placements();
    assign_students();
    write_placements();
}

/********************************************************************/
/* filestream functions */

// reads data from "trips.csv" and fills in trips vector
void read_trips() {
    CSVReader reader("trips.csv");
    vector<vector<string>> trip_data = reader.getData();

    trip t;

    // ignore first row
    for(int i = 1; i < trip_data.size(); i++) {
        vector<string> row = trip_data[i];
        t.index = i-1;
        t.name = row[0];
        t.capacity = stoi(row[1]);
        t.num_of_requests = {0,0,0,0,0,0};
        t.full = false;
        trips.push_back(t);
    }

    for(int i = 0; i < trips.size(); i++) {
        open_trips.push_back(&trips[i]);
    }

}

// reads data from "OrientationChoices2018.csv" and fills in students vector
void read_students() {
    CSVReader reader("OrientationChoices2018.csv");
    vector<vector<string>> student_data = reader.getData();

    // ignore first row
    for(int i = 1; i < student_data.size(); i++) {
        student student_to_add;
        vector<string> row = student_data[i];
        student_to_add.placed = false;
        student_to_add.year = row[0];
        student_to_add.lastName = row[1];
        student_to_add.firstName = row[2];
        student_to_add.gender = gender_string_to_int(row[3]);
        student_to_add.swimmingAbility = stoi(row[4]);
        student_to_add.activityLevel = stoi(row[5]);
        student_to_add.activityIntensity = stoi(row[6]);
        for(int j = 7; j < 13; j++) {
            int trip_index = trip_name_to_index(row[j]);
            student_to_add.pref.push_back(trip_index);
        }
        students.push_back(student_to_add);
    }

}

// outputs placement to "OrientationPlacement.csv"
void write_placements() {
    ofstream output_file;
    output_file.open("OrientationPlacement.csv");
    output_file << "OrientationYear,LastName,FirstName,Gender,SwimmingAbility,ActivityLevel,ActivityIntensity,choice1,choice2,choice3,choice4,choice5,choice6,TripAssignment\n";
    for(int i = 0; i < best_trips.size(); i++) {
        // cout << trips[i].name << endl;
        for(int j = 0; j < best_trips[i].participants.size(); j++) {
            int student_index = best_trips[i].participants[j];
            output_file << get_csv_row_for_student(student_index);
            output_file << best_trips[i].name + ",";
            if(students[student_index].got_choice == false) {
                output_file << "Student didn't get choice,";
            }
            output_file << "\n";
            // cout << get_csv_row_for_student(student_index) << endl;
        }
    }
    output_file.close();
}


/********************************************************************/
/* data processing functions */

// parses user input to convert gender to -1 (male), 1 (female), or 0 (non-binary)
int gender_string_to_int(string gender_string) {
    if(gender_string[0] == 'M' || gender_string[0] == 'm') {
        return -1;
    }
    else if(gender_string[0] == 'F' || gender_string[0] == 'f') {
        return 1;
    }
    return 0;
}

// converts gender int to a  string
string gender_int_to_string(int gender_int) {
    if(gender_int == -1) {
        return "Male";
    }
    else if(gender_int == 1) {
        return "Female";
    }
    return "Non-binary";
}

// returns trip_index based on a trip name
int trip_name_to_index(string name) {
    for(int i = 0; i < trips.size(); i++) {
        if(name == trips[i].name) {
            return i;
        }
    }
    return -1;
}

//returns csv row for a student
string get_csv_row_for_student(int index) {
    student s = students[index];
    string genderString = gender_int_to_string(s.gender);
    string studentString;
    studentString += s.year + ",";
    studentString += s.lastName + ",";
    studentString += s.firstName + ",";
    studentString += genderString + ",";
    studentString += s.swimmingAbility;
    studentString += ",";
    studentString += s.activityLevel;
    studentString += ",";
    studentString += s.activityIntensity;
    studentString += ",";
    for(int j = 0; j < s.pref.size(); j++) {
        int trip_index = s.pref[j];
        if(trip_index != -1) {
            studentString += trips[trip_index].name + ",";
        }
        else {
            studentString += "Trip not Found,";
        }
    }
    return studentString;
}


/********************************************************************/
/* placement functions */

// places students  on trips
void assign_students() {
    int best_score = 10000000;
    best_trips = trips;
    for(int i = 0; i < 100; i++) {
        reset_placements();
        assign_students_randomly();
        int score = get_score();
        if(score < best_score) {
            best_score = score;
            best_trips = trips;
            cout << best_score << endl;
        }
    }
}

int get_score() {
    int score = 0;
    for(int i = 0; i < trips.size(); i++) {
        for(int j = 0; j < trips[i].participants.size(); j++) {
            int student_index = trips[i].participants[j];
            if(students[student_index].got_choice == false) {
                score += 1000;
            }
            for(int k = 0; k < students[student_index].pref.size(); k++) {
                if(students[student_index].pref[k] == i) {
                    score += k;
                    break;
                }
            }
        }
    }
    return score;
}

// find the trip with the smallest difference between capacity and requests
// if requests > capacity, then add the student with the highest priority for that trip (randomly if needed)
// if requests <= capacity, then add a random student
// repeat until no students are left to be placed
void assign_least_requested_first() {
    while(students_placed < students.size()) {
        // cout << "size of open_trips: " << open_trips.size() << endl;
        // cout << "students unassigned: " << students.size() - students_placed << endl;
        sort(open_trips.begin(), open_trips.end(), trip_buffer_cmp);
        //print_open_trips();
        int index_of_best_trip = open_trips[open_trips.size()-1]->index;

        int index_of_best_student;

        if(trips[index_of_best_trip].total_requests < trips[index_of_best_trip].capacity) {
            index_of_best_student = get_random_student();
        }
        else {
            index_of_best_student = get_best_student_for_trip(index_of_best_trip);
        }

        place_student_on_trip(index_of_best_student, index_of_best_trip);

        if(trips[index_of_best_trip].full) {
            open_trips.pop_back();
        }
        //sleep(1);
    }
}

int get_random_open_trip() {
    srand(time(NULL));
    int rand_index = rand() % trips.size();
    while(trips[rand_index].full == true) {
        rand_index = rand() % trips.size();
    }
    return rand_index;
}

int get_random_student() {
    srand(time(NULL));
    int best_rand_index = -1;
    int best_request_score = 100000;
    int rand_index;
    for(int i = 0; i < 5; i++) {
        rand_index = rand() % students.size();
        while(students[rand_index].placed == true) {
            rand_index = rand() % students.size();
        }
        if(students[rand_index].request_score < best_request_score) {
            best_rand_index = rand_index;
            best_request_score = students[rand_index].request_score;
        }
    }
    return best_rand_index;
}

// gets the best student for a trip and also removes it from unassigned_students
int get_best_student_for_trip(int trip_index) {
    int index_of_best_student = -1;
    int best_priority = 6;
    for(int i = 0; i < students.size(); i++) {
        student s = students[i];
        if(s.placed == false) {
            for(int j = 0; j < best_priority; j++) {
                if(s.pref[j] == trip_index) {
                    index_of_best_student = i;
                    best_priority = j;
                    // cout << "best priority: " << best_priority << endl;
                    // cout << get_csv_row_for_student(i) << endl;
                    break;
                }
            }
        }
    }
    if(index_of_best_student != -1) {
        return index_of_best_student;
    }
    return get_random_student();
}


// counts the number of unplaced students who have requested each trip
void count_trip_requests() {
    for(int i = 0; i < students.size(); i++) {
        for(int j = 0; j < students[i].pref.size(); j++) {
            int trip_index = students[i].pref[j];
            trips[trip_index].num_of_requests[j]++;
            trips[trip_index].total_requests++;
        }
    }
    for(int i = 0; i < students.size(); i++) {
        for(int j = 0; j < students[i].pref.size(); j++) {
            int trip_index = students[i].pref[j];
            students[i].request_score += trips[trip_index].total_requests/trips[trip_index].capacity;
        }
    }
}

// assigns student to trip
bool place_student_on_trip(int student_index, int trip_index) {
    // cout << "placing student " << student_index << " on trip " << trip_index << endl;

    if(trips[trip_index].full) {
        return false;
    }

    assert(students[student_index].placed == false);

    trips[trip_index].participants.push_back(student_index);
    if(trips[trip_index].participants.size() >= trips[trip_index].capacity) {
        // cout << trips[trip_index].participants.size() << endl;
        // cout << trips[trip_index].capacity << endl;
        assert(trips[trip_index].participants.size() == trips[trip_index].capacity);
        trips[trip_index].full = true;
    }

    for(int i = 0; i < students[student_index].pref.size(); i++) {
        int trip_index = students[student_index].pref[i];
        trips[trip_index].num_of_requests[i]--;
        trips[trip_index].total_requests--;
    }

    students[student_index].placed = true;
    students_placed++;

    return true;
}

// places student on a trip randomly
void assign_students_randomly() {
    for(int i = 0; i < students.size(); i++) {

        int random_student = get_random_student();

        int trip_index;
        bool all_trips_full = true;
        for(int j = 0; j < students[random_student].pref.size(); j++) {
            trip_index = students[random_student].pref[j];
            if(trips[trip_index].full == false) {
                students[random_student].got_choice = true;
                place_student_on_trip(random_student, trip_index);
                all_trips_full = false;
                break;
            }
        }

        if(all_trips_full) {
            students[random_student].got_choice = false;
            place_student_on_trip(random_student, get_random_open_trip());
        }
    }
}

void reset_placements() {
    //cout << "resetting placements" << endl;
    trips = {};
    students = {};
    read_trips();
    read_students();
    count_trip_requests();
}


/********************************************************************/
/* miscellaneous functions */
int request_ratio_cmp(trip* t1, trip* t2) {
    assert(t1->capacity-t1->total_requests!=0);
    int ratio1 = t1->total_requests/(t1->capacity-t1->total_requests);
    int ratio2 = t2->total_requests/(t2->capacity-t2->total_requests);
    return ratio1 > ratio2;
}

int trip_buffer_cmp(trip* t1, trip* t2) {
    int diff1 = t1->total_requests - t1->capacity;
    int diff2 = t2->total_requests - t2->capacity;
    return diff1 > diff2;
}


/********************************************************************/
/* print/debug functions */

void print_trips() {
    for(int i = 0; i < trips.size(); i++) {
        assert(trips[i].num_of_requests.size() == 6);
        printf("trip index: %d, capacity: %d, total requests: %d, c1: %d, c2: %d, c3: %d, c4: %d, c5: %d, c6: %d, name: %s\n", i, trips[i].capacity, trips[i].total_requests, trips[i].num_of_requests[0], trips[i].num_of_requests[1], trips[i].num_of_requests[2], trips[i].num_of_requests[3], trips[i].num_of_requests[4], trips[i].num_of_requests[5], trips[i].name.c_str());
        // trips[i].name.c_str()
    }
}

void print_open_trips() {
    for(int i = 0; i < open_trips.size(); i++) {
        printf("trip index: %d, capacity: %d, total requests: %d, c1: %d, c2: %d, c3: %d, c4: %d, c5: %d, c6: %d, name: %s\n", open_trips[i]->index, open_trips[i]->capacity, open_trips[i]->total_requests, open_trips[i]->num_of_requests[0], open_trips[i]->num_of_requests[1], open_trips[i]->num_of_requests[2], open_trips[i]->num_of_requests[3], open_trips[i]->num_of_requests[4], open_trips[i]->num_of_requests[5], open_trips[i]->name.c_str());
        // trips[i].name.c_str()
    }
}

void print_students() {
    cout << "Choice 1: " << students[0].pref[0] << endl;
    cout << "Choice 6: " << students[0].pref[5] << endl;

    for(int i = 0; i < students.size(); i++) {
        cout << get_csv_row_for_student(i) << endl;
    }
}
