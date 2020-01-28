/*
 * SampleData.h
 *
 *  Created on: Nov 5, 2014
 *      Author: Victor Zappi
 */

#ifndef SAMPLEDATA_H_
#define SAMPLEDATA_H_

 #include <string>

// User defined structure to pass between main and rendere complex data retrieved from file
struct SampleData {
	std::string filename;
	float *samples;	// Samples in file
	int sampleLen;	// Total nume of samples
};



#endif /* SAMPLEDATA_H_ */
