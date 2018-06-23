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



typedef struct _student {
  int id; //the row that the student is on
  string year, lastName, firstName;
  int gender; //-1 for male, 1 for female, 0 for non-binary
  int swimmingAbility, activityLevel, activityIntensity;
  vector<int> pref;
  bool placed;
} student;

typedef struct _trip {
    int id; //the row that the trip is on
    string name;
    int capacity;
    int num_of_requests;
    bool full;
    vector<int> participants;
} trip;



/*
 * A class to read data from a csv file.
 */
class CSVReader
{
	std::string fileName;
	std::string delimeter;

public:
	CSVReader(std::string filename, std::string delm = ",") :
			fileName(filename), delimeter(delm)
	{ }

	// Function to fetch data from a CSV File
	std::vector<std::vector<std::string> > getData();
};

/*
* Parses through csv file line by line and returns the data
* in vector of vector of strings.
*/
std::vector<std::vector<std::string> > CSVReader::getData()
{
	std::ifstream file(fileName);

	std::vector<std::vector<std::string> > dataList;

	std::string line = "";
	// Iterate through each line and split the content using delimeter
	while (getline(file, line))
	{
		std::vector<std::string> vec;
		boost::algorithm::split(vec, line, boost::is_any_of(delimeter));
		dataList.push_back(vec);
	}
	// Close the File
	file.close();

	return dataList;
}

#endif
