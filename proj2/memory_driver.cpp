#include "memory_controller.h"
#include <fstream>
#include <iostream>
#include <cstring>
#include <sstream>
#include <iostream>
#include <vector>

using namespace std;

struct trace
{
	bool MemR;
	bool MemW;
	int adr;
	int data;
};

/*
Either implement your memory_controller here or use a separate .cpp/.c file for memory_controller and all the other functions inside it (e.g., LW, SW, Search, Evict, etc.)
*/

int main(int argc, char *argv[]) // the program runs like this: ./program <filename> <mode>
{
	// input file (i.e., test.txt)
	string filename = argv[1];

	// mode for replacement policy
	int type;

	ifstream fin;

	// opening file
	fin.open(filename.c_str());
	if (!fin)
	{ // making sure the file is correctly opened
		cout << "Error opening " << filename << endl;
		exit(1);
	}

	if (argc > 2)
		type = stoi(argv[2]); // DM: 0, FA: 1, SA: 2
	else
		type = 0; // the default is DM.

	// reading the text file
	string line;
	vector<trace> myTrace;
	int TraceSize = 0;
	string s1, s2, s3, s4;
	while (getline(fin, line))
	{
		stringstream ss(line);
		getline(ss, s1, ',');
		getline(ss, s2, ',');
		getline(ss, s3, ',');
		getline(ss, s4, ',');
		myTrace.push_back(trace());
		myTrace[TraceSize].MemR = stoi(s1);
		myTrace[TraceSize].MemW = stoi(s2);
		myTrace[TraceSize].adr = stoi(s3);
		myTrace[TraceSize].data = stoi(s4);
		//cout<<myTrace[TraceSize].MemW << endl;
		TraceSize += 1;
	}

	// defining a fully associative or direct-mapped cache
	cache_line myCache[CACHE_LINES]; // 1 set per line. 1B per Block
	int myMem[MEM_SIZE];

	// initializing
	for (int i = 0; i < CACHE_LINES; i++)
	{
		myCache[i].tag = -1;		 // -1 indicates that the tag value is invalid. We don't use a separate VALID bit.
		myCache[i].lru_position = 0; // 0 means lowest position
		myCache[i].data = 0;
	}

	// counters for miss rate
	int accessL = 0; //////
	int accessS = 0;
	int miss = 0; // this has to be updated inside your memory_controller
	int status = 1;
	int clock = 0;
	int traceCounter = 0;
	bool cur_MemR;
	bool cur_MemW;
	int cur_adr;
	int cur_data;
	bool hit;
	// this is the main loop of the code
	while (traceCounter < TraceSize || status < 1)
	{
		if (status == 1)
		{
			cur_MemR = myTrace[traceCounter].MemR;
			cur_MemW = myTrace[traceCounter].MemW;
			cur_data = myTrace[traceCounter].data;
			cur_adr = myTrace[traceCounter].adr;
			hit = false;
			traceCounter += 1;
			if (cur_MemR == 1)
				accessL += 1;
			else if (cur_MemW == 1)
				accessS += 1;
		}

		status = memoryController(cur_MemR, cur_MemW, &cur_data, cur_adr, status, &miss, type, myCache, myMem); // in your memory controller you need to implement your FSM, LW, SW, and MM.

		clock += 1;
	}
	float miss_rate = miss / (float)accessL;

	// printing the final result
	cout << "(" << clock << "," << miss_rate << ")" << endl;

	// closing the file
	fin.close();

	return 0;
}
