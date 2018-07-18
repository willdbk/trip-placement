#ifndef __header_h
#define __header_h

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <boost/algorithm/string.hpp>

using namespace std;

//
typedef struct _student {
  string year, lastName, firstName;
  int gender; //-1 for male, 1 for female, 0 for non-binary
  int swimmingAbility, activityLevel, activityIntensity;
  vector<int> pref;
  bool placed;
  bool got_choice;
  bool best_trip_got_choice;
  int request_score;
} student;

typedef struct _trip {
    string name;
    int index;
    int capacity;
    int minSwimmingAbility, minActivityLevel, minActivityIntensity;
    vector<int> requests_vector;
    int total_requests;
    int num_of_females;
    int num_of_males;
    bool full;
    vector<int> participants;
} trip;



/*
 * A class to read data from a csv file.
 */
class CSVReader
{
	string fileName;
	string delimeter;

public:
	CSVReader(string filename, string delm = ",") :
			fileName(filename), delimeter(delm)
	{ }

	// Function to fetch data from a CSV File
	vector<vector<string> > getData();
};

/*
* Parses through csv file line by line and returns the data
* in vector of vector of strings.
*/
vector<vector<string> > CSVReader::getData()
{
	ifstream file(fileName);

	vector<vector<string> > dataList;

	string line = "";
	// Iterate through each line and split the content using delimeter
	while (getline(file, line))
	{
		vector<string> vec;
		boost::algorithm::split(vec, line, boost::is_any_of(delimeter));
		dataList.push_back(vec);
	}
	// Close the File
	file.close();

	return dataList;
}

#endif
