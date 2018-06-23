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
vector<student> students;
vector<trip> trips;


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
void assign_student_random(int student_id);
bool assign_student_to_trip(int student_id, int trip_id);

void print_trips();
void print_students();

/********************************************************************/
/* functions */
/********************************************************************/

int main(int argc, char** argv) {
    read_trips();
    read_students();
    assign_students();
    write_placements();

    // print_trips();
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
        t.num_of_requests = 0;
        t.full = false;
        trips.push_back(t);
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

// places students randomly on trips
void assign_students() {
    for(int i = 0; i < students.size(); i++) {
        assign_student_random(i);
    }
}

// places student on a trip randomly
void assign_student_random(int student_id) {
    bool placed = false;
    while(!placed) {
        int random_trip = rand() % trips.size();
        placed = assign_student_to_trip(student_id, random_trip);
    }
}

// assigns student to trip
bool assign_student_to_trip(int student_id, int trip_id) {
    if(trips[trip_id].full) {
        return false;
    }
    if(students[student_id].placed) {
        return false;
    }

    // make sure placing student on trip won't leave a trip with not enough requests

    trips[trip_id].participants.push_back(student_id);
    if(trips[trip_id].participants.size() >= trips[trip_id].capacity) {
        assert(trips[trip_id].participants.size() == trips[trip_id].capacity);
        trips[trip_id].full = true;
    }
    return true;
}


/********************************************************************/
/* print/debug functions */

void print_trips() {
    for(int i = 0; i < trips.size(); i++) {
        printf("trip name: %s, capacity: %d\n", trips[i].name.c_str(), trips[i].capacity);
    }
}

void print_students() {
    cout << "Choice 1: " << students[0].pref[0] << endl;
    cout << "Choice 6: " << students[0].pref[5] << endl;

    for(int i = 0; i < students.size(); i++) {
        cout << get_csv_row_for_student(i) << endl;
    }
}
