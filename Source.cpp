// Jeremiah Spears
// This program will ask the user for an input file and will
// then perform all required operations from the input file
// and output the results to the user

#include <iostream>
#include <fstream>
#include <iostream>
#include <string>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <iterator>

using namespace std;

// struct to hold the agent information
struct agent
{
	int infectiousTime = 0;	// infectious period for each agent
	char state = 's';		// state of each agent:
							// 's' for susceptible,
							// 'i' for infected,
							// 'r' for recovered,
							// 'v' for vaccinated,
							// ('t' for temporary)
};

// struct to store the amount of agents for each day
// this is used to find the peak outbreak day
struct peak
{	// amount of agents per day and corresponding day
	int peakDay, peakAgents;
};

// declare functions
void getAgents(bool& fileFound, ifstream& fileName);
void makeGrid();
void printGrid(vector < vector<agent>> vecP);
void getNeighbors(vector < vector<agent>>& vec2, bool& noInfectious);
void checkNeighbors(vector < vector<agent>>& vec2, int row, int col);
void returntoS(vector < vector<agent>>& vec2);
void fixInfectious(vector < vector<agent>>& vec2);
void recovery(vector < vector<agent>>& vec2);
void countInfect(bool& noInfectious, int day);
void countAll(vector < vector<agent>>& vec2);

// global variables
int countS = 0, countI = 0, countR = 0, countV = 0;		// variables to store state counts
vector < vector<agent>> vec;							// 2D vector to hold agents
vector<agent> vecA;										// vector to hold agents
int infect = 0;											// the infectious period
int threshold = 0;										// the required threshold of infected agents
int display = 1;										// the display period 								
int length = 1;											// the dimension of the grid
agent nAgent;											// create a new instance of struct agent
peak peakpeak;											// create a new instance of struct peak
vector<peak> peakOut;									// vector to store peak struct

// main function
int main()
{
	// variables
	ifstream fileName;									// fileName to open
	string fName;										// Input from user to get fileName
	bool fileFound = false;								// let the program know if the file was found
	bool noInfectious = false;							// status of existence of infectious agents
	vector < vector<agent>> vec2;						// second 2d vector for copying
	int day = 0;										// the current day for the program

	// Ask the user which simulation they want (the user can input any file name)
	cout << "Input the file name (and extension) of the simulation you would like to run.\n";
	cin >> fName;

	fileName.open(fName);											// open the file the user asked for
	
	getAgents(fileFound, fileName);									// call getAgents function

	// initial setup for the program
	if (fileFound == true)	
	{
		makeGrid();													// call makeGrid function
		vec2 = vec;													// copy vec to vec2
		fixInfectious(vec2);										// call fixInfectious

		cout << "Day: " << day << endl;
		printGrid(vec);												// call printGrid function
	}

	// run the program until there are no more
	// infected agents
	while (noInfectious == false && fileFound == true)
	{
		day++;														// increase the day count
		vec2 = vec;													// copy vec to vec2
		getNeighbors(vec2, noInfectious);							// call getNeighbors function
		returntoS(vec2);											// call returntoS function
		recovery(vec2);												// call recovery function
		vec = vec2;													// copy vec2 to vec
		countInfect(noInfectious, day);								// call countInfect function
		// only show output for the display days
		if (day % display == 0)
		{
			cout << "\nDay: " << day;									// output the day to the user
			cout << endl;												// skip a line
			printGrid(vec);												// call printGrid function
		}
	} 
	
	// final output
	if (noInfectious == true)
	{
		// find the maximum outbreak day
		auto max = std::minmax_element(peakOut.begin(), peakOut.end(),
			[](peak const& lhs, peak const& rhs) {return lhs.peakAgents < rhs.peakAgents; });

		// call countAll function
		countAll(vec2);
		
		//output
		cout << "Outbreak Ends: Day " << day << endl;				// output final day 
		cout << "Peak Day: Day " << max.second->peakDay << endl;	// output peak day
		cout << "Number of Susceptible Agents: " << countS << endl;	// output number of susceptible agents
		cout << "Number of Infected Agents: " << countI << endl;	// output number of Infected agents
		cout << "Number of Recovered Agents: " << countR << endl;	// output number of recovered agents
		cout << "Number of Vaccinated Agents: " << countV << endl;	// output number of vaccinated agents
	}

	cout << endl;
	system("PAUSE");
	return 0;
}

// this function will output the file and count the number of each type of agent
void getAgents(bool& fileFound, ifstream& fileName)
{
	char stream;								// file parser to get the agents
	int stream2;								// file parser to get the threshold
	int stream3;								// file parser to get the infectious period
	int stream4;								// file parser to get the display period
	if (fileName.is_open())						// check if file is open
	{
		fileFound = true;						// Let program know the file has been found

		fileName.ignore(10);					// skip to the threshold number
		fileName >> noskipws >> stream2;		// output the next number to stream
		threshold = stream2;					// set the number to threshold

		fileName.clear();						// clear eof			
		fileName.seekg(0, ios::beg);			// return to beginning of file

		fileName.ignore(30);					// skip to the infectious period number
		fileName >> noskipws >> stream3;		// output the next number to stream
		infect = stream3;						// set the number to infect

		fileName.clear();						// clear eof			
		fileName.seekg(0, ios::beg);			// return to beginning of file
		
		fileName.ignore(40);					// skip to the display period number
		fileName >> noskipws >> stream4;		// output the next number to the stream
		display = stream4;						// set the number to display

		fileName.clear();						// clear eof			
		fileName.seekg(0, ios::beg);			// return to beginning of file

		fileName.ignore(42);					// skip to the agents
		while (fileName >> noskipws >> stream)	// move through each character in the field
		{
			if (stream == '\n')					// search file for newlines
			{
				length++;						// count how many rows and columns there are
			}
		}
		length--;								// don't count the last line in the file

		fileName.clear();						// clear eof			
		fileName.seekg(0, ios::beg);			// return to beginning of file

		fileName.ignore(42);					// skip ahead in the file to the agents
		while (fileName >> noskipws >> stream)	// move through each character in the field
		{
			if (stream == 'i')					// search file for infected agents
			{
				nAgent.state = stream;			// set state for nAgent struct
				nAgent.infectiousTime = infect;	// set infectiousTime for nAgent struct
				vecA.push_back(nAgent);			// push back nAgent into vecA
			}
			if (stream == 's')					// search file for susceptible agents
			{
				nAgent.state = stream;			// set state for nAgent struct
				nAgent.infectiousTime = infect;	// set infectiousTime for nAgent struct
				vecA.push_back(nAgent);			// push back nAgent into vecA
			}
			if (stream == 'r')					// search file for recovered agents
			{
				nAgent.state = stream;			// set state for nAgent struct
				nAgent.infectiousTime = infect;	// set infectiousTime for nAgent struct
				vecA.push_back(nAgent);			// push back nAgent into vecA
			}
			if (stream == 'v')					// search file for vaccinated agents
			{
				nAgent.state = stream;			// set state for nAgent struct
				nAgent.infectiousTime = infect;	// set infectiousTime for nAgent struct
				vecA.push_back(nAgent);			// push back nAgent into vecA
			}
		}
		fileName.close();						// close the file
	}
	else cout << "File could not be found.\n";	// let the user know the file could not be found
}

// fix the infectious period for the initial output
void fixInfectious(vector < vector<agent>>& vec2)
{
	for (int col = 0; col < vec.size(); col++)
	{
		for (int row = 0; row < vec[col].size(); row++)
		{
			if (vec[row][col].state == 'i')
			{
				vec2[row][col].infectiousTime--; // decrease the infectiousTime
			}
		}
	}
}

// this function will take the states and put them in a 2D vector
// with the same format as the input file
void makeGrid()
{
	// setup the 2d Vector
	for (int col = 0; col < length; col++)				// set number of columns
	{
		vector<agent>temp;								// create temporary vector to hold columns
		for (int row = 0; row < length; row++)			// set number of rows
		{
			temp.push_back(nAgent);						// push back nAgent onto temp vector
		}
		vec.push_back(temp);							// push back the temp vector onto the 2dVec
	}

	// Set the values of the 2d Vector
	for (int col = 0; col < vec.size(); col++)
	{
		for (int row = 0; row < vec[col].size(); row++)
		{
			vec[col][row] = vecA[col*length + row];		// fill each row and column with vecA
		}
	}
}


// the function will print the grid for any given day
void printGrid(vector < vector<agent>> vecP)
{
	for (int col = 0; col < vecP.size(); col++)
	{
		for (int row = 0; row < vecP[col].size(); row++)
		{
			cout << vecP[col][row].state << " ";		// output the grid
		}
		cout << endl;									// skip a line
	}
}

// this function will set the agents state to r if it reaches
// 0 infectious time
void recovery(vector < vector<agent>>& vec2)
{
	for (int col = 0; col < vec.size(); col++)
	{
		for (int row = 0; row < vec[col].size(); row++)
		{
			if (vec2[row][col].infectiousTime == 0)
			{
				vec2[row][col].state = 'r';
			}
		}
	}
}

// this function will count the i agents
void countInfect(bool& noInfectious, int day)
{
	int countInfect = 0;
	for (int col = 0; col < vec.size(); col++)
	{
		for (int row = 0; row < vec[col].size(); row++)
		{
			if (vec[row][col].state == 'i')
			{
				countInfect++;
			}
		}
	}
	
	// store the number of infected agents for each 
	// day as a struct in the peakOut vector
	peakpeak.peakAgents = countInfect;	
	peakpeak.peakDay = day;
	peakOut.push_back(peakpeak);

	// if there are no more infected agents 
	// set noInfectious to true
	if (countInfect == 0)
	{
		noInfectious = true;
	}
}

// count the states in the final output
void countAll(vector < vector<agent>>& vec2)
{
	for (int col = 0; col < vec.size(); col++)
	{
		for (int row = 0; row < vec[col].size(); row++)
		{
			if (vec2[row][col].state == 'i') // i agents
			{
				countI++;
			}
			if (vec2[row][col].state == 's') // s agents
			{
				countS++;
			}
			if (vec2[row][col].state == 'v') // v agents
			{
				countV++;
			}
			if (vec2[row][col].state == 'r') // r agents
			{
				countR++;
			}
		}
	}
}

// This function will get all the neighbors for every 'i' in vec
// it will then convert those neighbors to the proper state if needed and store
// the updated grid in vec2
void getNeighbors(vector < vector<agent>>& vec2, bool& noInfectious)
{
	// set max to length - 1 because the grid starts at [0],[0] not [1],[1]
	int max = length - 1;	
	int countInfect = 0;

	// this double for loop will iterate through all states in the grid
	for (int col = 0; col < vec.size(); col++)
	{
		for (int row = 0; row < vec[col].size(); row++)
		{
			// call checkNeighbors function
			checkNeighbors(vec2, row, col);
			
			// if the state is i, lower the states infectiousTime
			if (vec[row][col].state == 'i')
			{
				vec2[row][col].infectiousTime--;
			}

			// if the state is i and the infectiousTime is greater than 0
			// find all neighbors
			if (vec[row][col].state == 'i' && vec[row][col].infectiousTime > 0)
			{
				// row 0
				if (row == 0)
				{
					// row 0 and col 0
					if (col == 0)
					{
						if (vec2[row][col + 1].state == 's')
							vec2[row][col + 1].state = 'i';
						if (vec2[row + 1][col].state == 's')
							vec2[row + 1][col].state = 'i';
						if (vec2[row + 1][col + 1].state == 's')
							vec2[row + 1][col + 1].state = 'i';
						if (vec2[row][col + max].state == 's')
							vec2[row][col + max].state = 'i';
						if (vec2[row + 1][col + max].state == 's')
							vec2[row + 1][col + max].state = 'i';
					}
					// row 0 and col max
					if (col == max)
					{
						if (vec2[row][col - 1].state == 's')
							vec2[row][col - 1].state = 'i';
						if (vec2[row + 1][col - 1].state == 's')
							vec2[row + 1][col - 1].state = 'i';
						if (vec2[row + 1][col].state == 's')
							vec2[row + 1][col].state = 'i';
						if (vec2[row][col - max].state == 's')
							vec2[row][col - max].state = 'i';
						if (vec2[row + 1][col - max].state == 's')
							vec2[row + 1][col - max].state = 'i';
					}
					// row 0 and non edge col
					if (col != 0 && col != max)
					{
						if (vec2[row][col - 1].state == 's')
							vec2[row][col - 1].state = 'i';
						if (vec2[row][col + 1].state == 's')
							vec2[row][col + 1].state = 'i';
						if (vec2[row + 1][col - 1].state == 's')
							vec2[row + 1][col - 1].state = 'i';
						if (vec2[row + 1][col].state == 's')
							vec2[row + 1][col].state = 'i';
						if (vec2[row + 1][col + 1].state == 's')
							vec2[row + 1][col + 1].state = 'i';
					}
				}
				// max row
				if (row == max)
				{
					// max row and 0 col
					if (col == 0)
					{
						if (vec2[row - 1][col].state == 's')
							vec2[row - 1][col].state = 'i';
						if (vec2[row - 1][col + 1].state == 's')
							vec2[row - 1][col + 1].state = 'i';
						if (vec2[row][col + 1].state == 's')
							vec2[row][col + 1].state = 'i';
						if (vec2[row][col + max].state == 's')
							vec2[row][col + max].state = 'i';
						if (vec2[row - 1][col + max].state == 's')
							vec2[row - 1][col + max].state = 'i';
					}
					// max row and max col
					if (col == max)
					{
						if (vec2[row - 1][col - 1].state == 's')
							vec2[row - 1][col - 1].state = 'i';
						if (vec2[row - 1][col].state == 's')
							vec2[row - 1][col].state = 'i';
						if (vec2[row][col - 1].state == 's')
							vec2[row][col - 1].state = 'i';
						if (vec2[row][col - max].state == 's')
							vec2[row][col - max].state = 'i';
						if (vec2[row - 1][col - max].state == 's')
							vec2[row - 1][col - max].state = 'i';
					}
					// max row and non edge col
					if (col != 0 && col != max)
					{
						if (vec2[row - 1][col - 1].state == 's')
							vec2[row - 1][col - 1].state = 'i';
						if (vec2[row - 1][col].state == 's')
							vec2[row - 1][col].state = 'i';
						if (vec2[row - 1][col + 1].state == 's')
							vec2[row - 1][col + 1].state = 'i';
						if (vec2[row][col - 1].state == 's')
							vec2[row][col - 1].state = 'i';
						if (vec2[row][col + 1].state == 's')
							vec2[row][col + 1].state = 'i';
					}
				}
				// 0 col non edge row
				if (col == 0 && row != max && row != 0)
				{
					if (vec2[row - 1][col].state == 's')
						vec2[row - 1][col].state = 'i';
					if (vec2[row - 1][col + 1].state == 's')
						vec2[row - 1][col + 1].state = 'i';
					if (vec2[row][col + 1].state == 's')
						vec2[row][col + 1].state = 'i';
					if (vec2[row + 1][col].state == 's')
						vec2[row + 1][col].state = 'i';
					if (vec2[row + 1][col + 1].state == 's')
						vec2[row + 1][col + 1].state = 'i';
					if (vec2[row][col + max].state == 's')
						vec2[row][col + max].state = 'i';
					if (vec2[row + 1][col + max].state == 's')
						vec2[row + 1][col + max].state = 'i';
					if (vec2[row - 1][col + max].state == 's')
						vec2[row - 1][col + max].state = 'i';
				}
				// max col non edge row
				if (col == max && row != max && row != 0)
				{
					if (vec2[row - 1][col - 1].state == 's')
						vec2[row - 1][col - 1].state = 'i';
					if (vec2[row - 1][col].state == 's')
						vec2[row - 1][col].state = 'i';
					if (vec2[row][col - 1].state = 's')
						vec2[row][col - 1].state = 'i';
					if (vec2[row + 1][col - 1].state == 's')
						vec2[row + 1][col - 1].state = 'i';
					if (vec2[row + 1][col].state == 's')
						vec2[row + 1][col].state = 'i';
					if (vec2[row][col - max].state == 's')
						vec2[row][col - max].state = 'i';
					if (vec2[row + 1][col - max].state == 's')
						vec2[row + 1][col - max].state = 'i';
					if (vec2[row - 1][col - max].state == 's')
						vec2[row - 1][col - max].state = 'i';
				}
				// all non edge col and rows
				if (col != 0 && col != max && row != 0 && row != max)
				{
					if (vec2[row - 1][col - 1].state == 's')
						vec2[row - 1][col - 1].state = 'i';
					if (vec2[row - 1][col].state == 's')
						vec2[row - 1][col].state = 'i';
					if (vec2[row - 1][col + 1].state == 's')
						vec2[row - 1][col + 1].state = 'i';
					if (vec2[row][col - 1].state == 's')
						vec2[row][col - 1].state = 'i';
					if (vec2[row][col + 1].state == 's')
						vec2[row][col + 1].state = 'i';
					if (vec2[row + 1][col - 1].state == 's')
						vec2[row + 1][col - 1].state = 'i';
					if (vec2[row + 1][col].state == 's')
						vec2[row + 1][col].state = 'i';
					if (vec2[row + 1][col + 1].state == 's')
						vec2[row + 1][col + 1].state = 'i'; 
				}
			}
		}
	}
}		 
	
// this function will get all the neighbors for s states
// if there are not at least threshold number of i states
// in the s state's neighborhood the function will
// temporarily set the state to t
// so it will not be updated in the getNeighbor function
void checkNeighbors(vector < vector<agent>>& vec2, int row, int col)
{
	// set max to length - 1 because the grid starts at [0],[0] not [1],[1]
	int max = length - 1;
	int numofI = 0;		// number of i in neighborhood
	
	if (vec[row][col].state == 's')
	{
		// row 0
		if (row == 0)
		{
			// row 0 and col 0
			if (col == 0)
			{
				if (vec[row][col + 1].state == 'i')
					numofI++;
				if (vec[row + 1][col].state == 'i')
					numofI++;
				if (vec[row + 1][col + 1].state == 'i')
					numofI++;
			}
			// row 0 and col max
			if (col == max)
			{
				if (vec[row][col - 1].state == 'i')
					numofI++;
				if (vec[row + 1][col - 1].state == 'i')
					numofI++;
				if (vec[row + 1][col].state == 'i')
					numofI++;
			}
			// row 0 and non edge col
			if (col != 0 && col != max)
			{
				if (vec[row][col - 1].state == 'i')
					numofI++;
				if (vec[row][col + 1].state == 'i')
					numofI++;
				if (vec[row + 1][col - 1].state == 'i')
					numofI++;
				if (vec[row + 1][col].state == 'i')
					numofI++;
				if (vec[row + 1][col + 1].state == 'i')
					numofI++;
			}
		}
		// max row
		if (row == max)
		{
			// max row and 0 col
			if (col == 0)
			{
				if (vec[row - 1][col].state == 'i')
					numofI++;
				if (vec[row - 1][col + 1].state == 'i')
					numofI++;
				if (vec[row][col + 1].state == 'i')
					numofI++;
			}
			// max row and max col
			if (col == max)
			{
				if (vec[row - 1][col - 1].state == 'i')
					numofI++;
				if (vec[row - 1][col].state == 'i')
					numofI++;
				if (vec[row][col - 1].state == 'i')
					numofI++;
			}
			// max row and non edge col
			if (col != 0 && col != max)
			{
				if (vec[row - 1][col - 1].state == 'i')
					numofI++;
				if (vec[row - 1][col].state == 'i')
					numofI++;
				if (vec[row - 1][col + 1].state == 'i')
					numofI++;
				if (vec[row][col - 1].state == 'i')
					numofI++;
				if (vec[row][col + 1].state == 'i')
					numofI++;
			}
		}
		// 0 col non edge row
		if (col == 0 && row != max && row != 0)
		{
			if (vec[row - 1][col].state == 'i')
				numofI++;
			if (vec[row - 1][col + 1].state == 'i')
				numofI++;
			if (vec[row][col + 1].state == 'i')
				numofI++;
			if (vec[row + 1][col].state == 'i')
				numofI++;
			if (vec[row + 1][col + 1].state == 'i')
				numofI++;
		}
		// max col non edge row
		if (col == max && row != max && row != 0)
		{
			if (vec[row - 1][col - 1].state == 'i')
				numofI++;
			if (vec[row - 1][col].state == 'i')
				numofI++;
			if (vec[row][col - 1].state = 'i')
				numofI++;
			if (vec[row + 1][col - 1].state == 'i')
				numofI++;
			if (vec[row + 1][col].state == 'i')
				numofI++;
		}
		// all non edge col and rows
		if (col != 0 && col != max && row != 0 && row != max)
		{
			if (vec[row - 1][col - 1].state == 'i')
				numofI++;
			if (vec[row - 1][col].state == 'i')
				numofI++;
			if (vec[row - 1][col + 1].state == 'i')
				numofI++;
			if (vec[row][col - 1].state == 'i')
				numofI++;
			if (vec[row][col + 1].state == 'i')
				numofI++;
			if (vec[row + 1][col - 1].state == 'i')
				numofI++;
			if (vec[row + 1][col].state == 'i')
				numofI++;
			if (vec[row + 1][col + 1].state == 'i')
				numofI++;
		}
	}

	// set temp state
	if (vec[row][col].state == 's' && numofI < threshold)
	{
		vec2[row][col].state = 't';
	}	
}

// this function will return the temporary t agents back to their original state
void returntoS(vector < vector<agent>>& vec2)
{
	for (int col = 0; col < vec.size(); col++)
	{
		for (int row = 0; row < vec[col].size(); row++)
		{
			if (vec2[row][col].state == 't') // if state is t
			{
				vec2[row][col].state = 's';	 // set state to s
			}
		}
	}
}