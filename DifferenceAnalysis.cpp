// DifferenceAnalysis.cpp : contains driver code to locate differences in wav files
//
// - Loads 2 wavs into memory <-BOTTLENECK
// - Locates the first sample where the two files differ

#include "AudioFile.h"
#include <assert.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <future>

using namespace std;

// FUNCTION PROTOTYPES
////////////////////////////////////////////
int					binarySearchFloatArray(float* input, float target, int startIndex, int endIndex, int counter, int previousCandidate);
float*				buildComparisonArray(AudioFile<float>& f1, AudioFile<float>& f2, int size, int startingIndex, int channel);
pair<bool, bool>	checkNeighbors(float* input, float target, int candidate);
void				setSizeDifference(AudioFile<float>& f1, AudioFile<float>& f2, int& difference, bool& fileOneLarger, int& smallerSize);
int					middleInt(int beginning, int end);
void				testCheckNeighbors();
void				testMiddleInt();

// MAIN
////////////////////////////////////////////

int main() {
	AudioFile<float> audioFile1;
	AudioFile<float> audioFile2;
	string filename;

	// Run tests
	testCheckNeighbors();
	testMiddleInt();

	// LOAD FILES
	// This portion can *easily* be parallelized if loading time takes too long
	// The loading is by far the slowest part of the program right now, so
	// multithreading is probably a good idea

	// Get File #1
	cout << "Enter filename to load " << endl;
	cin >> filename;

	audioFile1.load(filename);

	cout << "Done loading, here are the statistics: " << endl;
	audioFile1.printSummary();

	// Get File #2
	cout << "Enter filename to load " << endl;
	cin >> filename;

	audioFile2.load(filename);

	cout << "Done loading, here are the statistics: " << endl;
	audioFile2.printSummary();

	// Check that the files are the same size. If they aren't it is assumed that the comparison should be run
	// with the first sample in each file being aligned. If this is not the case, an offset can be introduced
	// the shorter file
	int difference;				// # of samples longer one file is than the other
	bool fileOneLarger;			// Whether the first file entered is longer than the second
	int smallerSize;			// The smaller size of the two file lengths

	// This function sets "difference", "fileOneLarger", and "smallerSize"
	setSizeDifference(audioFile1, audioFile2, difference, fileOneLarger, smallerSize);

	// Find the first sample which yields a difference between file one and file two
	float* comparison;				/* Array to hold the element-wise difference between files
									   Zero elements in this array represent samples that are equal
									   between the two files */
	int startingIndex = 0;			// First sample to check for difference
	int endingIndex = smallerSize;	// Last sample to check for difference
	int channel = 0;				/* Channel on which to check differences (generally, this won't need
									   modification unless, for example, a stereo file is modified and only
									   one channel receives modified samples--a highly unusual scenario */
	int counter = 0;				/* A counter to limit the recursion depth of the binary search, generally
									   not needed, but serves as a failsafe in case of a particularly pathalogical
									   edge case */

	// Second bottleneck - THIS CAN BE VASTLY IMPROVED WITH SOME EDITS IN 'AudioFile.cpp' and the original load process
	comparison = buildComparisonArray(audioFile1, audioFile2, smallerSize, startingIndex, channel);

	// Find location of first sample difference
	int location = binarySearchFloatArray(comparison, 0.0, startingIndex, endingIndex, counter, 0);
	cout << "First difference detected between file one and two at sample " << location << endl;
	cout << "This can be found at timecode " << (float)location / audioFile1.getSampleRate() << endl;

	// Some printing to guarantee that the NNN check in binarySearchFloatArray succeeded
	// cout << "Location - 1: " << comparison[location - 1] << endl;
	// cout << "Location:     " << comparison[location] << endl;
	// cout << "Location + 1: " << comparison[location + 1] << endl;

	return location;
}

// WAV HANDLING
////////////////////////////////////////////

void setSizeDifference(AudioFile<float>& f1, AudioFile<float>& f2, int& difference, bool& fileOneLarger, int& smallerSize) {
	// Sets the number of samples by which the lengths of file one and file two differ
	// e.g.: if file one is 300 samples long and file two is 200 samples long, this function would set:
	//		difference		=	100;
	//		fileOneLarger	=	true;
	//		smallerSize		=	200;
	int s1 = f1.getNumSamplesPerChannel();
	int s2 = f2.getNumSamplesPerChannel();
	fileOneLarger = (s1 > s2) ? true : false;
	if (fileOneLarger) {
		smallerSize = s2;
	}
	else {
		smallerSize = s1;
	}
	difference = abs(s1 - s2);
}

float* buildComparisonArray(AudioFile<float>& f1, AudioFile<float>& f2, int size, int startingIndex, int channel) {
	// Takes the two audio files and subtracts them sample-wise
	// Stores the result in the array "comparison"
	// Zero elements in comparison imply that those samples were identical in file one and file two
	float* comparison = NULL;
	comparison = new float[size];
	for (int i = startingIndex; i < size; i++) {
		comparison[i] = f1.samples[channel][i] - f2.samples[channel][i];
	}
	return comparison;
}

// HELPERS
////////////////////////////////////////////

int middleInt(int beginning, int end) {
	// Takes two ints and returns the middle integer
	// If middle is between two ints, the lower is returned
	if ((beginning + end) % 2 == 0) {
		return (beginning + end) / 2;
	}
	else {
		return (beginning + end - 1) / 2;
	}
}

pair<bool, bool> checkNeighbors(float* input, float target, int candidate) {
	// Takes an array. Checks if the elements in front of and behind the element with index "candidate" are equal to the target
	// Returns two bools, one if the left neighbor is the target, one if the right neighbor is the target
	bool tailingMatch = (input[candidate + 1] == target) ? true : false;
	bool leadingMatch = (input[candidate - 1] == target) ? true : false;
	return make_pair(leadingMatch, tailingMatch);
}

int binarySearchFloatArray(float* input, float target, int startIndex, int endIndex, int counter, int previousCandidate) {
	// Takes a binary search approach to finding the first non-target element of input array
	// 
	// NOTE: This isn't the best implementation for finding ALL different samples. It is a quick
	// solution for finding SOME difference between the two WAVs, as long as one doesn't mind which difference that is
	//
	// THOUGHT: For future development, this function can still be of use. We'd need (1) the user to provide a rough window in which
	// to search for a difference, and (2) the user to know with good confidence that the window would look like one of these cases:
	//
	// CASE 1:					CASE 2:					CASE 3:					CASE 4:
	// ----------++++++++++     ++++++++++----------	++++++----------++++	(inversion of CASE 3)
	//
	// where (+) represents a sample that is the same in both files, and (-) represents a sample that is different in both files.
	// (essentially, those cases are when you have no more than 3 contiguous blocks of samples that are either all the same or all
	// different)

	counter++;
	int candidate = middleInt(startIndex, endIndex);
	pair<bool, bool> neighborMatches = make_pair(false, false);
	if (counter < 100) {
		if (candidate == previousCandidate) {
			cout << "Confirmed candidate" << endl;
			return candidate;
		}
		else {
			if (input[candidate] == target) {
				cout << "Found candidate at " << candidate << endl;
				neighborMatches = checkNeighbors(input, target, candidate);
			}
			if (neighborMatches.first) {
				// Check left array
				cout << "Checking right array, left was empty" << endl;
				return binarySearchFloatArray(input, target, candidate, endIndex, counter, candidate);
			}
			return binarySearchFloatArray(input, target, startIndex, candidate, counter, candidate);
		}
	}
	else {
		cout << "Reached max recursion depth" << endl;
		return -1;
	}
}

// TESTS
////////////////////////////////////////////

void testCheckNeighbors() {
	float input[5] = { 3.4, 4.2, 4.2, 4.2, 3.5 };
	float target = 4.2;
	int candidate = 2;
	pair<bool, bool> test = checkNeighbors(input, target, candidate);
	pair<bool, bool> _case = make_pair(true, true);
	assert(test == _case);
}

void testMiddleInt() {
	int test1Beginning = 3;
	int test1End = 8;
	int _case1 = 5;
	int test2Beginning = 8;
	int test2End = 16;
	int _case2 = 12;
	assert(middleInt(test1Beginning, test1End) == _case1);
	assert(middleInt(test2Beginning, test2End) == _case2);
}
