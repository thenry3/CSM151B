#include "myCPU.h"
#include "instruction.h"
#include <chrono>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <bitset>

using namespace std;

int main(int argc, char *argv[]) // your project should be executed like this: ./cpusim filename.txt and should print (a0,a1)
{
	// Ensure we have a given file.
	if (argc < 2)
		return 1;

	// Open trace file
	ifstream instructionFile;
	instructionFile.open(argv[1]);

	// Instantiate CPU
	CPU myCPU(&instructionFile);

	// Clock and PC
	uint64_t myClock = 0;
	uint64_t myPC = 0;

	// start timer
	chrono::nanoseconds start = chrono::duration_cast<chrono::nanoseconds>(chrono::system_clock::now().time_since_epoch());

	while (1) // processor's main loop. Each iteration is equal to one clock cycle.
	{
		//fetch
		myCPU.fetch(myPC); // fetching the instruction

		// decode
		myCPU.decode(); // decoding

		// execute
		myCPU.execute();

		// memory
		myCPU.memory();

		// writeback
		myCPU.writeback();

		// Increment clock and PC
		myClock += 1;
		myPC += 4; // for now we can assume that next PC is always PC + 4

		// Break loop if all instructions have OPCODE zero.
		if (myCPU.shouldEnd())
			break;

		// Cycle pipeline registers
		myCPU.cycle();
	}
	chrono::nanoseconds end = chrono::duration_cast<chrono::nanoseconds>(chrono::system_clock::now().time_since_epoch());

	// print the stats
	// cout << "Clock: " << myClock << endl;
	// cout << "PC: " << myPC << endl;
	// cout << "Duration: " << (end - start).count() << endl;

	// Print a0 and a1: (x10, x11)
	cout << "(" << myCPU.getVal(10) << "," << myCPU.getVal(11) << ")" << endl;

	return 0;
}
