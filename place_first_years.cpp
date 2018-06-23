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
vector<student> students;
vector<trip*> open_trips;
vector<student*> unassigned_students;


/********************************************************************/
/* forward declarations of functions */
void read_trips();
void read_students();
void write_placements();

int gender_string_to_int(string gender_string);
string gender_int_to_string(int gender_int);
int trip_name_to_id(string name);
string get_csv_row_for_student(int id);

void assign_students();
void assign_least_requested_first();
void count_trip_requests();
int get_random_student();
int get_best_student_for_trip(int trip_id);
bool place_student_on_trip(int student_id, int trip_id);
void assign_students_randomly();

int trip_buffer_cmp(trip* t1, trip* t2);

void print_trips();
void print_open_trips();
void print_students();

/********************************************************************/
/* functions */
/********************************************************************/

int main(int argc, char** argv) {
    read_trips();
    read_students();
    count_trip_requests();
    //print_trips();
    assign_students();
    //print_trips();
    write_placements();

    //print_students();
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
        t.id = i - 1;
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
        student_to_add.id = i;
        student_to_add.year = row[0];
        student_to_add.lastName = row[1];
        student_to_add.firstName = row[2];
        student_to_add.gender = gender_string_to_int(row[3]);
        student_to_add.swimmingAbility = stoi(row[4]);
        student_to_add.activityLevel = stoi(row[5]);
        student_to_add.activityIntensity = stoi(row[6]);
        for(int j = 7; j < 13; j++) {
            int trip_id = trip_name_to_id(row[j]);
            student_to_add.pref.push_back(trip_id);
        }
        students.push_back(student_to_add);
    }

    for(int i = 0; i < students.size(); i++) {
        unassigned_students.push_back(&students[i]);
    }

}

// outputs placement to "OrientationPlacement.csv"
void write_placements() {
    ofstream output_file;
    output_file.open("OrientationPlacement.csv");
    output_file << "OrientationYear,LastName,FirstName,Gender,SwimmingAbility,ActivityLevel,ActivityIntensity,choice1,choice2,choice3,choice4,choice5,choice6,TripAssignment\n";
    for(int i = 0; i < trips.size(); i++) {
        for(int j = 0; j < trips[i].participants.size(); j++) {
            int student_id = trips[i].participants[j];
            output_file << get_csv_row_for_student(student_id);
            output_file << trips[i].name + "," + "\n";
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


// returns trip_id based on a trip name
int trip_name_to_id(string name) {
    for(int i = 0; i < trips.size(); i++) {
        if(name == trips[i].name) {
            assert(i == trips[i].id);
            return trips[i].id;
        }
    }
    return -1;
}

// // returns name based on a trip id
// String trip_name_to_id(int trip_id) {
//     return trips[trip_id].name;
// }

string get_csv_row_for_student(int id) {
    student s = students[id];
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
        int trip_id = s.pref[j];
        if(trip_id != -1) {
            studentString += trips[trip_id].name + ",";
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
    assign_least_requested_first();
}

// find the trip with the smallest difference between capacity and requests
// if requests > capacity, then add the student with the highest priority for that trip (randomly if needed)
// if requests <= capacity, then add a random student
// repeat until no students are left to be placed
void assign_least_requested_first() {
    sort(open_trips.begin(), open_trips.end(), trip_buffer_cmp);
    print_open_trips();
    while(unassigned_students.size() > 0) {
        // cout << "size of open_trips: " << open_trips.size() << endl;
        // cout << "size of unassigned_students: " << unassigned_students.size() << endl;
        sort(open_trips.begin(), open_trips.end(), trip_buffer_cmp);
        int best_trip_id = open_trips[open_trips.size()-1]->id;

        int best_student_id;

        if(trips[best_trip_id].total_requests < trips[best_trip_id].capacity) {
            best_student_id = get_random_student();
        }
        else {

            best_student_id = get_best_student_for_trip(best_trip_id);
        }

        place_student_on_trip(best_student_id, best_trip_id);

        if(trips[best_trip_id].full) {
            open_trips.pop_back();
        }
    }
}

int get_random_student() {
    int rand_index = rand() % unassigned_students.size();
    unassigned_students.erase(unassigned_students.begin()+rand_index);
    return unassigned_students[rand_index]->id;
}

// gets the best student for a trip and also removes it from unassigned_students
int get_best_student_for_trip(int trip_id) {
    int best_student_id = -1;
    int best_priority = 6;
    int index_in_available = -1;
    for(int i = 0; i < unassigned_students.size(); i++) {
        student* s = unassigned_students[i];
        for(int j = 0; j < best_priority; j++) {
            if(s->pref[j] == trip_id) {
                best_student_id = s->id - 1;
                best_priority = j;
                index_in_available = i;
                cout << "best priority: " << best_priority << endl;
                cout << get_csv_row_for_student(i) << endl;
                break;
            }
        }
    }
    unassigned_students.erase(unassigned_students.begin()+index_in_available);
    if(best_student_id != -1) {
        cout << "placing student " << best_student_id << " on trip " << trip_id << endl;
        return best_student_id;
    }
    else {
        return get_random_student();
    }
}

// counts the number of unplaced students who have requested each trip
void count_trip_requests() {
    for(int i = 0; i < students.size(); i++) {
        for(int j = 0; j < students[i].pref.size(); j++) {
            int trip_id = students[i].pref[j];
            trips[trip_id].num_of_requests[j] = trips[trip_id].num_of_requests[j] + 1;
            trips[trip_id].total_requests++;
        }
    }
}

// assigns student to trip
bool place_student_on_trip(int student_id, int trip_id) {
    if(trips[trip_id].full) {
        return false;
    }
    assert(students[student_id].placed == false);

    trips[trip_id].participants.push_back(student_id);
    if(trips[trip_id].participants.size() >= trips[trip_id].capacity) {
        assert(trips[trip_id].participants.size() == trips[trip_id].capacity);
        trips[trip_id].full = true;
    }

    for(int i = 0; i < students[student_id].pref.size(); i++) {
        int trip_id = students[student_id].pref[i];
        trips[trip_id].num_of_requests[i]--;
        trips[trip_id].total_requests--;
    }

    return true;
}

// places student on a trip randomly
void assign_students_randomly() {
    for(int i = 0; i < students.size(); i++) {
        bool placed = false;
        while(!placed) {
            int random_trip = rand() % trips.size();
            placed = place_student_on_trip(i, random_trip);
        }
    }
}

/********************************************************************/
/* miscellaneous functions */
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
        printf("trip id: %d, capacity: %d, total requests: %d, c1: %d, c2: %d, c3: %d, c4: %d, c5: %d, c6: %d, name: %s\n", i, trips[i].capacity, trips[i].total_requests, trips[i].num_of_requests[0], trips[i].num_of_requests[1], trips[i].num_of_requests[2], trips[i].num_of_requests[3], trips[i].num_of_requests[4], trips[i].num_of_requests[5], trips[i].name.c_str());
        // trips[i].name.c_str()
    }
}

void print_open_trips() {
    for(int i = 0; i < open_trips.size(); i++) {
        printf("trip id: %d, capacity: %d, total requests: %d, c1: %d, c2: %d, c3: %d, c4: %d, c5: %d, c6: %d, name: %s\n", open_trips[i]->id, open_trips[i]->capacity, open_trips[i]->total_requests, open_trips[i]->num_of_requests[0], open_trips[i]->num_of_requests[1], open_trips[i]->num_of_requests[2], open_trips[i]->num_of_requests[3], open_trips[i]->num_of_requests[4], open_trips[i]->num_of_requests[5], open_trips[i]->name.c_str());
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
