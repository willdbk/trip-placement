## Project: Trip Placement
## Author: Will deBruynKops
## All code in C/C++

#### USAGE:
- Copy the folder named "trip-placement" (where this document is located) onto your Desktop
- Two files must be updated for this years trips, "trips.csv" and "OrientationChoices.csv"
- "trips.csv" should be updated to include the capacity and minimum skill levels for the 3 skill categories
- Trip requests should be exported from https://gooseeye.bowdoin.edu/outingclub/Home/AdminHome, moved into the folder "trip-placement" and renamed "OrientationChoices.csv"
- Note: both files must be saved as CSX files, not XLXS
- At this point, you must the program "place_first_years". This code will output a new spreadsheet with each student placed on a trip.
- To execute this program, open up the app "Terminal" if on a Mac and "CMD" if on a PC.
- Once you have a command line prompt open, type the command:
    `cd Desktop/trip-placement`
- Then type the command:
    `./place_first_years`
- At this point, there should be an output file titled "OrientationPlacement.csv" in which each student has been assigned a trip!

#### HOW IT WORKS:
CSV files are read and their data is stored using the class CSVReader. The output file is written to using the ofstream variable type.

This algorithm is a greedy best-first algorithm that assigns students using a best-first search. Each iteration it sorts the open_trips based on which trip needs students the most (i.e. which trip has the lowest ratio between number of requests and number of remaining spots). It then chooses the best student for this trip. It repeats this process until all trips are full enough that they remain below the average trip fullness. Once all trips have been sufficiently filled, the remaining students are randomly chosen and placed on their trip of highest preference that still has space. During the entire process, students are never placed on trips if it would lead to a gender imbalance or if they do not have sufficient skills for the trip.

Due to the random nature of this algorithm, 1000 unique trip placements are generated and the one with the highest score is outputted. The score is computed based on how many students got as high a preference as possible.
