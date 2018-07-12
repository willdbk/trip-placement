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
#include <cctype>
#include <string.h>


#include <vector>
#include <algorithm>

using namespace std;

/********************************************************************/
/* global variables */
vector<trip> trips;
vector<trip> best_trips;
int num_of_spaces;
vector<student> students;
int students_placed;
double average_percent_filled;
vector<trip*> open_trips;
int num_of_students_didnt_get_choice;



/********************************************************************/
/* forward declarations of functions */
void read_trips();
void read_students();
void write_placements();

int gender_string_to_int(string gender_string);
string gender_int_to_string(int gender_int);
int trip_name_to_index(string name);
bool strings_similar(string s1, string s2);
string get_csv_row_for_student(int index);

void assign_students();
int get_score();
void assign_least_requested_first();
int get_best_trip();
void count_trip_requests();
int get_random_student();
int get_open_trip_for_student(int student_index);
void remove_from_open_trips(int trip_index);
int get_random_open_trip();
int get_best_student_for_trip(int trip_index);
bool place_student_on_trip(int student_index, int trip_index);
void assign_students_randomly();
void reset_placements();
void assign_students_random_n_iterations(int n);


int request_ratio_cmp(trip* t1, trip* t2);
int trip_buffer_cmp(trip* t1, trip* t2);

void print_trips();
void print_open_trips();
void print_students();

/********************************************************************/
/* functions */
/********************************************************************/

int main(int argc, char** argv) {
    num_of_students_didnt_get_choice = 0;
    srand(time(NULL));
    reset_placements();
    //print_trips();
    assign_students();
    //print_trips();
    write_placements();
    cout << "num_of_students who didn't get their choice: " << num_of_students_didnt_get_choice << endl;
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
        t.requests_vector = {0,0,0,0,0,0};
        t.total_requests = 0;
        t.num_of_females = 0;
        t.num_of_males = 0;
        t.full = false;
        trips.push_back(t);

        num_of_spaces += t.capacity;
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
        if(isdigit(row[4][0])) {
            student_to_add.swimmingAbility = stoi(row[4]);
        }
        if(isdigit(row[5][0])) {
            student_to_add.activityLevel = stoi(row[5]);
        }
        if(isdigit(row[6][0])) {
            student_to_add.activityIntensity = stoi(row[6]);
        }
        student_to_add.pref = {};
        if(row.size() > 7) {
            for(int j = 7; j < 13; j++) {
                int trip_index = trip_name_to_index(row[j]);
                if(trip_index != -1) {
                    student_to_add.pref.push_back(trip_index);
                }
            }
        }
        students.push_back(student_to_add);
    }
    average_percent_filled = (students.size()*1.0)/num_of_spaces;
    // cout << "average_percent_filled1: " << average_percent_filled << endl;
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
    if(name.size() > 1) {
        for(int i = 0; i < trips.size(); i++) {
            if(strings_similar(trips[i].name, name)) {
                return i;
            }
        }
    }
    return -1;
}

bool strings_similar(string s1, string s2) {
    int count = 0;
    for(int i = 0; i < s1.size(); i++) {
        if(toupper(s1[i]) != toupper(s2[i])) {
            count++;
        }
    }
    if(count <= 1) {
        return true;
    }
    return false;
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
    for(int j = 0; j < 6; j++) {
        if(j >= s.pref.size()) {
            studentString += "no pref given,";
            continue;
        }
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
    //assign_students_random_n_iterations(10x);
    assign_least_requested_first();
}

// find the trip with the smallest ratio between capacity and requests
// if requests > capacity, then add the student with the highest priority for that trip (randomly if needed)
// if requests <= capacity, then add a random student
// repeat until no students are left to be placed
void assign_least_requested_first() {
    while(students_placed < students.size()) {
        int trip_to_fill = get_best_trip();
        int student_to_add = -1;
        int i = 0;
        while(trip_to_fill == -1 && i < (students.size()-students_placed)*10) {
            student_to_add = get_random_student();
            trip_to_fill = get_open_trip_for_student(student_to_add);
            i++;
        }
        if(trip_to_fill == -1) {
            trip_to_fill = get_random_open_trip();
            student_to_add = get_random_student();
        }
        if(student_to_add == -1) {
            student_to_add = get_best_student_for_trip(trip_to_fill);
        }


        place_student_on_trip(student_to_add, trip_to_fill);

        if(trips[trip_to_fill].full) {
            remove_from_open_trips(trip_to_fill);
        }
    }
    best_trips = trips;
}

void remove_from_open_trips(int trip_index) {
    for(int i = 0; i < open_trips.size(); i++) {
        if(open_trips[i]->index == trip_index) {
            open_trips.erase(open_trips.begin() + i);
            break;
        }
    }
    return;
}

// returns index of the trip that most needs students
// returns -1 if no trip can be added to without going over the average
int get_best_trip() {
    sort(open_trips.begin(), open_trips.end(), request_ratio_cmp);
    //print_open_trips();
    //printf("\n");
    double threshold = 0.75;
    // usleep(500000);

    int index_in_open_trips = open_trips.size()-1;
    int index_of_best_trip = open_trips[index_in_open_trips]->index;
    double percent_filled_after_add = (trips[index_of_best_trip].participants.size()+1.0)/trips[index_of_best_trip].capacity;
    while(percent_filled_after_add > average_percent_filled) {
        if(index_in_open_trips > 0) {
            index_in_open_trips--;
        }
        else {
            cout << "-1 returned" << endl;
            return -1;
        }
        index_of_best_trip = open_trips[index_in_open_trips]->index;
        percent_filled_after_add = (trips[index_of_best_trip].participants.size()+1.0)/trips[index_of_best_trip].capacity;
    }
    //cout << "index_of_best_trip: " << index_of_best_trip << endl;
    return index_of_best_trip;
}

// gets the best student for a trip and also removes it from unassigned_students
int get_best_student_for_trip(int trip_index) {
    bool max_females = false;
    bool max_males = false;

    if(trips[trip_index].num_of_females >= trips[trip_index].capacity/2) {
        max_females = true;
    }
    if(trips[trip_index].num_of_males >= trips[trip_index].capacity/2) {
        max_males = true;
    }

    int index_of_best_student = -1;
    int best_priority = 6;
    for(int i = 0; i < students.size(); i++) {
        student s = students[i];
        if(max_females && s.gender == 1) {
            continue;
        }
        if(max_males && s.gender == -1) {
            continue;
        }
        if(s.placed == false) {
            for(int j = 0; j < best_priority; j++) {
                if(s.pref.size() > j && s.pref[j] == trip_index) {
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

// calls assign_students_randomly n times and outputs the placement with the best "score"
void assign_students_random_n_iterations(int n) {
    int best_score = 10000000;
    best_trips = trips;
    for(int i = 0; i < n; i++) {
        reset_placements();
        assign_students_randomly();
        int score = get_score();
        if(score < best_score) {
            best_score = score;
            best_trips = trips;
            cout << best_score << endl;

        }
    }
    cout << best_score << endl;
}

// places student on a trip randomly
void assign_students_randomly() {
    for(int i = 0; i < students.size(); i++) {

        int best_random_student = get_random_student();
        int best_request_score = students[best_random_student].request_score;
        for(int i = 0; i < 50; i++) {
            int random_student = get_random_student();
            if(students[random_student].request_score < best_request_score) {
                best_random_student = random_student;
                best_request_score = students[random_student].request_score;
            }
        }

        int trip_index;
        bool all_trips_full = true;
        for(int j = 0; j < students[best_random_student].pref.size(); j++) {
            trip_index = students[best_random_student].pref[j];
            if(trips[trip_index].full == false) {
                students[best_random_student].got_choice = true;
                place_student_on_trip(best_random_student, trip_index);
                all_trips_full = false;
                break;
            }
        }

        if(all_trips_full) {
            students[best_random_student].got_choice = false;
            place_student_on_trip(best_random_student, get_random_open_trip());
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


int get_random_open_trip() {
    int rand_index = rand() % trips.size();
    while(trips[rand_index].full == true) {
        rand_index = (rand_index+1) % trips.size();
    }
    return rand_index;
}

int get_random_student() {
    int rand_index = rand() % students.size();
    while(students[rand_index].placed == true) {
        rand_index = (rand_index+1) % students.size();
    }
    return rand_index;
}

int get_open_trip_for_student(int student_index) {
    int trip_index;
    for(int j = 0; j < students[student_index].pref.size(); j++) {
        trip_index = students[student_index].pref[j];
        if(trips[trip_index].full == false) {
            return trip_index;
        }
    }
    return -1;
}

// counts the number of unplaced students who have requested each trip
void count_trip_requests() {
    for(int i = 0; i < students.size(); i++) {
        for(int j = 0; j < students[i].pref.size(); j++) {

            int trip_index = students[i].pref[j];

            trips[trip_index].total_requests++;

            trips[trip_index].requests_vector[j]++;
            students[i].request_score += trips[trip_index].total_requests/trips[trip_index].capacity;
        }
    }
}

// assigns student to trip
bool place_student_on_trip(int student_index, int trip_index) {
    cout << "placing student " << student_index << " on trip " << trip_index << endl;

    if(trips[trip_index].full) {
        return false;
    }

    assert(students[student_index].placed == false);

    trips[trip_index].participants.push_back(student_index);
    if(students[student_index].gender == 1) {
        trips[trip_index].num_of_females++;
    }
    else if(students[student_index].gender == -1) {
        trips[trip_index].num_of_males++;
    }
    if(trips[trip_index].participants.size() >= trips[trip_index].capacity) {
        // cout << trips[trip_index].participants.size() << endl;
        // cout << trips[trip_index].capacity << endl;
        assert(trips[trip_index].participants.size() == trips[trip_index].capacity);
        cout << "trip " << trip_index << " is full" << endl;
        trips[trip_index].full = true;
    }

    for(int i = 0; i < students[student_index].pref.size(); i++) {
        int trip_index = students[student_index].pref[i];
        trips[trip_index].requests_vector[i]--;
        trips[trip_index].total_requests--;
    }

    students[student_index].placed = true;
    students_placed++;

    return true;
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
    cout << "trip index: " << t1->index << endl;
    assert(t1->capacity-t1->participants.size()!=0);
    double ratio1 = t1->total_requests/((1.0*t1->capacity)-t1->participants.size());
    double ratio2 = t2->total_requests/((1.0*t2->capacity)-t2->participants.size());
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
        assert(trips[i].requests_vector.size() == 6);
        printf("trip index: %d, capacity: %d, num of students placed: %lu, num of females: %d, num of males: %d, total requests: %d, name: %s\n", i, trips[i].capacity, trips[i].participants.size(), trips[i].num_of_females, trips[i].num_of_males, trips[i].total_requests, trips[i].name.c_str());
        printf("c1: %d, c2: %d, c3: %d, c4: %d, c5: %d, c6: %d\n",  trips[i].requests_vector[0], trips[i].requests_vector[1], trips[i].requests_vector[2], trips[i].requests_vector[3], trips[i].requests_vector[4], trips[i].requests_vector[5]);
    }
}

void print_open_trips() {
    for(int i = 0; i < open_trips.size(); i++) {
        double ratio = open_trips[i]->total_requests/((1.0*open_trips[i]->capacity)-open_trips[i]->participants.size());
        printf("trip index: %d, ratio: %f, capacity: %d, num of students placed: %lu, total requests: %d, c1: %d, c2: %d, c3: %d, c4: %d, c5: %d, c6: %d, name: %s\n", open_trips[i]->index, ratio, open_trips[i]->capacity, open_trips[i]->participants.size(), open_trips[i]->total_requests, open_trips[i]->requests_vector[0], open_trips[i]->requests_vector[1], open_trips[i]->requests_vector[2], open_trips[i]->requests_vector[3], open_trips[i]->requests_vector[4], open_trips[i]->requests_vector[5], open_trips[i]->name.c_str());
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
