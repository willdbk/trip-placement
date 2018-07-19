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
int did_not_get_choice;

int num_of_iterations;



/********************************************************************/
/* forward declarations of functions */
/********************************************************************/

void read_trips();
void read_students();
void write_placements();
void reset_placements();
void count_trip_requests();

int gender_string_to_int(string gender_string);
string gender_int_to_string(int gender_int);
int trip_name_to_index(string name);
bool strings_similar(string s1, string s2);
string get_csv_row_for_student(int index);

void assign_students();
void assign_students_n_iterations(int n);
void assign_least_requested_first();
int get_best_trip();
int get_best_student_for_trip(int trip_index);
bool can_place_student_on_trip(int student_index, int trip_index);
int get_random_student();
int get_open_trip_for_student(int student_index);
int get_random_open_trip();
void erase_open_trip_with_index(int trip_index);
void place_student_on_trip(int student_index, int trip_index);
int preference_squared_heuristic();
int preference_heuristic();

int request_ratio_cmp(trip* t1, trip* t2);

void print_trips();
void print_open_trips();
void print_students();

void assign_students_randomly();


/********************************************************************/
/* explicit declaration of functions */
/********************************************************************/

// parses command line input and calls primary functions
int main(int argc, char** argv) {
    srand(time(NULL));

    //read number of iterations from user
    if (argc!=2) {
      printf("\nusage: ./place_first_years <num_of_iterations> \n");
      printf("<num_of_iterations> flag takes an integer >= 1\n\n");
      exit(1);
    }
    num_of_iterations = atoi(argv[1]);
    printf("\nyou entered num_of_iterations=%d\n", num_of_iterations);


    cout << "\nReading CSV files..." << endl;
    read_trips();
    read_students();
    reset_placements();
    cout << "CSV files read." << endl;

    cout << "\nPlacing students..." << endl;
    assign_students();
    cout << "Students placed." << endl;

    cout << "\nWriting placements..." << endl;
    write_placements();
    cout << "Placements written.\n" << endl;

}


/********************************************************************/
/* filestream functions */

// reads data from "trips.csv" and fills in trips vector
void read_trips() {
    CSVReader reader("trips.csv");
    vector<vector<string>> trip_data = reader.getData();
    trip t;
    // ignore first row which contains column headers
    for(int i = 1; i < trip_data.size(); i++) {
        vector<string> row = trip_data[i];
        t.index = i-1;
        t.name = row[0];
        t.capacity = stoi(row[1]);
        if(isdigit(row[2][0])) {
            t.minSwimmingAbility = stoi(row[2]);
        }
        if(isdigit(row[3][0])) {
            t.minActivityLevel = stoi(row[3]);
        }
        if(isdigit(row[4][0])) {
            t.minActivityIntensity = stoi(row[4]);
        }
        num_of_spaces += t.capacity;
        trips.push_back(t);
    }
}

// reads data from "OrientationChoices.csv" and fills in students vector
void read_students() {
    CSVReader reader("OrientationChoices.csv");
    vector<vector<string>> student_data = reader.getData();
    // ignore first row which contains column headers
    for(int i = 1; i < student_data.size(); i++) {
        student student_to_add;
        vector<string> row = student_data[i];
        // if the first column doesn't contain a number, don't read that row
        if(!isdigit(row[0][0])) {
            continue;
        }
        student_to_add.year = row[0];
        student_to_add.lastName = row[1];
        student_to_add.firstName = row[2];
        student_to_add.gender = gender_string_to_int(row[3]);
        // only adds skill catgeories if given
        if(isdigit(row[4][0])) {
            student_to_add.swimmingAbility = stoi(row[4]);
        }
        if(isdigit(row[5][0])) {
            student_to_add.activityLevel = stoi(row[5]);
        }
        if(isdigit(row[6][0])) {
            student_to_add.activityIntensity = stoi(row[6]);
        }
        student_to_add.preferences = {};
        if(row.size() > 7) {
            for(int j = 7; j < 13; j++) {
                int trip_index = trip_name_to_index(row[j]);
                if(trip_index != -1) {
                    student_to_add.preferences.push_back(trip_index);
                }
            }
        }
        students.push_back(student_to_add);
    }
    average_percent_filled = (students.size()*1.0)/num_of_spaces;
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
            if(students[student_index].best_trip_got_choice == false) {
                output_file << "Student didn't get choice,";
            }
            output_file << "\n";
        }
    }
    output_file.close();
}

// resets the placement of students onto trips
void reset_placements() {
    students_placed = 0;
    num_of_spaces = 0;
    did_not_get_choice = 0;
    open_trips = {};
    // reset trips
    for(int i = 0; i < trips.size(); i++) {
        trips[i].requests_vector = {0,0,0,0,0,0};
        trips[i].total_requests = 0;
        trips[i].num_of_females = 0;
        trips[i].num_of_males = 0;
        trips[i].participants = {};
        trips[i].full = false;
        open_trips.push_back(&trips[i]);
    }
    // reset students
    for(int i = 0; i < students.size(); i++) {
        students[i].placed = false;
        students[i].got_choice = false;
    }
    count_trip_requests();
}

// counts the number of unplaced students who have requested each trip
void count_trip_requests() {
    for(int i = 0; i < students.size(); i++) {
        for(int j = 0; j < students[i].preferences.size(); j++) {
            int trip_index = students[i].preferences[j];
            trips[trip_index].total_requests++;
            trips[trip_index].requests_vector[j]++;
            students[i].request_score += trips[trip_index].total_requests/trips[trip_index].capacity;
        }
    }
}


/********************************************************************/
/* data processing functions */

// parses user input to convert gender to -1 (male), 1 (female), or 0 (non-binary)
int gender_string_to_int(string gender_string) {
    // Reads any string beginning with 'M', 'm', 'B', or 'b' as male
    if(gender_string[0] == 'M' || gender_string[0] == 'm' || gender_string[0] == 'B' || gender_string[0] == 'b') {
        return -1;
    }
    // Reads any string beginning with 'F', 'f', 'G', or 'g' as female
    else if(gender_string[0] == 'F' || gender_string[0] == 'f' || gender_string[0] == 'G' || gender_string[0] == 'g' || gender_string[0] == 'W' || gender_string[0] == 'w') {
        return 1;
    }
    // Reads any other string as non-binary.
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

// return true if two strings differ by less than 2 characters
// function is case-insensitive
// allows for small typos in "OrientationChoices.csv"
bool strings_similar(string s1, string s2) {
    int count = 0;
    for(int i = 0; i < s1.size(); i++) {
        if(i >= s2.size() || toupper(s1[i]) != toupper(s2[i])) {
            count++;
        }
    }
    for(int i = 0; i < s2.size(); i++) {
        if(i >= s1.size() || toupper(s1[i]) != toupper(s2[i])) {
            count++;
        }
    }
    if(count <= 4) {
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
    studentString += to_string(s.swimmingAbility) + ",";
    studentString += to_string(s.activityLevel) + ",";
    studentString += to_string(s.activityIntensity) + ",";
    for(int j = 0; j < 6; j++) {
        if(j >= s.preferences.size()) {
            studentString += "no preferences given,";
            continue;
        }
        int trip_index = s.preferences[j];
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

// places students on trips
void assign_students() {
    assign_students_n_iterations(num_of_iterations);
}

// calls assign_least_requested_first n times and outputs the placement with the best "score"
void assign_students_n_iterations(int n) {
    int best_score = 10000000;
    best_trips = trips;
    for(int i = 0; i < n; i++) {
        reset_placements();
        assign_least_requested_first();
        int score = preference_squared_heuristic();
        if(score < best_score) {
            best_score = score;
            best_trips = trips;
            for(int i = 0; i < students.size(); i++) {
                students[i].best_trip_got_choice = students[i].got_choice;
            }
            double average_pref_given = 1.0 + (preference_heuristic()+0.0)/students.size();
            printf("Current Best Trip's Score: %d, Average Preference Given: %f, Number of Students Didn't Get Choice: %d\n", score, average_pref_given, did_not_get_choice);
        }
    }
}

// places students one by one on trips until no students are unplaced
void assign_least_requested_first() {
    while(students_placed < students.size()) {
        // gets the trip that needs students
        int trip_to_fill = get_best_trip();
        int student_to_add = -1;
        // finds the best student for the trip
        if(trip_to_fill != -1) {
            student_to_add = get_best_student_for_trip(trip_to_fill);
            if(student_to_add == -1) {
                student_to_add = get_random_student();
            }
        }
        // if no trips NEED students
        else {
            // then choose a random student and give them on their highest preference that has space
            int i = 0;
            while(trip_to_fill == -1 && i < (students.size()-students_placed)*50) {
                student_to_add = get_random_student();
                trip_to_fill = get_open_trip_for_student(student_to_add);
                i++;
            }
            // if
            if(trip_to_fill == -1) {
                trip_to_fill = get_random_open_trip();
                student_to_add = get_random_student();
            }
        }

        place_student_on_trip(student_to_add, trip_to_fill);

        if(trips[trip_to_fill].full) {
            erase_open_trip_with_index(trip_to_fill);
        }
    }
}

// returns index of the trip that most needs students
// returns -1 if no trip can be added to without going over the average
int get_best_trip() {
    sort(open_trips.begin(), open_trips.end(), request_ratio_cmp);

    int index_in_open_trips = open_trips.size()-1;
    int index_of_best_trip = open_trips[index_in_open_trips]->index;
    double percent_filled_after_add = (trips[index_of_best_trip].participants.size()+1.0)/trips[index_of_best_trip].capacity;
    while(percent_filled_after_add > average_percent_filled) {
        if(index_in_open_trips > 0) {
            index_in_open_trips--;
        }
        else {
            return -1;
        }
        index_of_best_trip = open_trips[index_in_open_trips]->index;
        percent_filled_after_add = (trips[index_of_best_trip].participants.size()+1.0)/trips[index_of_best_trip].capacity;
    }
    //cout << "index_of_best_trip: " << index_of_best_trip << endl;
    return index_of_best_trip;
}

// gets the best student for a trip
int get_best_student_for_trip(int trip_index) {
    int index_of_best_student = -1;
    int best_priority = 6;
    int rand_index = rand() % students.size();

    for(int i = 0; i < students.size(); i++) {
        int student_index = (i+rand_index)%students.size();
        if(can_place_student_on_trip(student_index, trip_index)) {
            for(int j = 0; j < best_priority; j++) {
                if(students[student_index].preferences.size() > j && students[student_index].preferences[j] == trip_index) {
                    index_of_best_student = student_index;
                    best_priority = j;
                    break;
                }
            }
        }
    }
    if(index_of_best_student != -1) {
        return index_of_best_student;
    }
    return -1;
}

// checks if student and trip are compatible in terms of capacity, gender, and abilities
bool can_place_student_on_trip(int student_index, int trip_index) {
    student* s = &students[student_index];
    trip* t = &trips[trip_index];
    if(s->placed == true) {
        return false;
    }
    if(t->full == true) {
        return false;
    }
    if((s->gender == 1 && t->num_of_females >= t->capacity/2) ||
        (s->gender == -1 && t->num_of_males >= t->capacity/2)) {
        return false;
    }
    if((s->swimmingAbility < t->minSwimmingAbility) ||
        (s->activityLevel < t->minActivityLevel) ||
        (s->activityIntensity < t->minActivityIntensity)) {
        return false;
    }
    return true;
}

// returns a random unplaced student
int get_random_student() {
    int rand_index = rand() % students.size();
    while(students[rand_index].placed == true) {
        rand_index = (rand_index+1) % students.size();
    }
    return rand_index;
}

// returns the student's highest priority open trip
int get_open_trip_for_student(int student_index) {
    int trip_index;
    for(int j = 0; j < students[student_index].preferences.size(); j++) {
        trip_index = students[student_index].preferences[j];
        if(can_place_student_on_trip(student_index, trip_index)) {
            return trip_index;
        }
    }
    return -1;
}

// returns a random open trip
int get_random_open_trip() {
    int rand_index = rand() % trips.size();
    while(trips[rand_index].full == true) {
        rand_index = (rand_index+1) % trips.size();
    }
    return rand_index;
}

// assigns student to trip
void place_student_on_trip(int student_index, int trip_index) {
    //updates trip data
    trips[trip_index].participants.push_back(student_index);
    if(students[student_index].gender == 1) {
        trips[trip_index].num_of_females++;
    }
    else if(students[student_index].gender == -1) {
        trips[trip_index].num_of_males++;
    }
    if(trips[trip_index].participants.size() == trips[trip_index].capacity) {
        trips[trip_index].full = true;
    }
    bool got_choice = false;
    for(int i = 0; i < students[student_index].preferences.size(); i++) {
        int trip_index_of_preference = students[student_index].preferences[i];
        trips[trip_index_of_preference].requests_vector[i]--;
        trips[trip_index_of_preference].total_requests--;
        if(trip_index_of_preference == trip_index) {
            got_choice = true;
        }
    }
    if(got_choice == false) {
        did_not_get_choice++;
    }
    //updates student data
    students[student_index].got_choice = got_choice;
    students[student_index].placed = true;
    students_placed++;
}

// erases an open trip with a given index
void erase_open_trip_with_index(int trip_index) {
    for(int i = 0; i < open_trips.size(); i++) {
        if(open_trips[i]->index == trip_index) {
            open_trips.erase(open_trips.begin() + i);
            break;
        }
    }
    return;
}

// returns a score based on the square of each what preference the trip each student recieved
// for each student, their compenent of the score: is
// k*k if they got their kth preference and
// 100 if they didn't get any of their preferences
int preference_squared_heuristic() {
    int score = 0;
    for(int i = 0; i < trips.size(); i++) {
        for(int j = 0; j < trips[i].participants.size(); j++) {
            int student_index = trips[i].participants[j];
            if(students[student_index].got_choice == false) {
                score += 10*10;
            }
            for(int k = 0; k < students[student_index].preferences.size(); k++) {
                if(students[student_index].preferences[k] == i) {
                    score += k*k;
                    break;
                }
            }
        }
    }
    return score;
}

// returns a score based on the what preference the trip each student recieved
// for each student, their compenent of the score: is
// k*k if they got their kth preference and
// 100 if they didn't get any of their preferences
int preference_heuristic() {
    int score = 0;
    for(int i = 0; i < trips.size(); i++) {
        for(int j = 0; j < trips[i].participants.size(); j++) {
            int student_index = trips[i].participants[j];
            if(students[student_index].got_choice == false) {
                score += 10;
            }
            for(int k = 0; k < students[student_index].preferences.size(); k++) {
                if(students[student_index].preferences[k] == i) {
                    score += k;
                    break;
                }
            }
        }
    }
    return score;
}


/********************************************************************/
/* compare functions */

// compares trips based on the ratio between requests and spaces available
int request_ratio_cmp(trip* t1, trip* t2) {
    assert(t1->capacity-t1->participants.size()!=0);
    double ratio1 = t1->total_requests/((1.0*t1->capacity)-t1->participants.size());
    double ratio2 = t2->total_requests/((1.0*t2->capacity)-t2->participants.size());
    return ratio1 > ratio2;
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
    cout << "Choice 1: " << students[0].preferences[0] << endl;
    cout << "Choice 6: " << students[0].preferences[5] << endl;

    for(int i = 0; i < students.size(); i++) {
        cout << get_csv_row_for_student(i) << endl;
    }
}

/********************************************************************/
/* unused placement functions */

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
        for(int j = 0; j < students[best_random_student].preferences.size(); j++) {
            trip_index = students[best_random_student].preferences[j];
            if(trips[trip_index].full == false) {
                place_student_on_trip(best_random_student, trip_index);
                all_trips_full = false;
                break;
            }
        }
        if(all_trips_full) {
            place_student_on_trip(best_random_student, get_random_open_trip());
        }
    }
}
