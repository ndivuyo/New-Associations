/*

Dillon Bastan 2020 
(Together with code from Bela->SampleLoader Example)

 */

#ifndef SAMPLEPLAYER_H_
#define SAMPLEPLAYER_H_

#include <SampleData.h>
#include <SampleLoader.h>
#include <string>

// 
struct SamplePlayer {
	//
	SampleData data;
	int loopStart;
	int loopEnd;
	int readPtr;
	bool resetFlag = false;


	//
	void setup(std::string filename, int size) {
		// Setup an empty buffer
		if (size != 0) {
			data.sampleLen = size;
			data.samples = new float[data.sampleLen];
		}
		// Load a sample file into buffer
		else {
			data.filename = filename;
			data.sampleLen = getNumFrames(data.filename);
			data.samples = new float[data.sampleLen];
			getSamples(data.filename, data.samples, 0, 0, data.sampleLen);
		}
		//
		loopStart = 0;
    	loopEnd = data.sampleLen;
	}

	//
	void setPos(int sample) {
		readPtr = sample;
	}

	//
	float play() {
		// Reset Loop
		if ( ++readPtr > loopEnd ) {
			readPtr = loopStart;
			resetFlag = true;
		} else if (resetFlag) resetFlag = false;
		//
		return data.samples[readPtr];
	}

	//
	void record(float x, int dub) {
		float y = x;
		if (dub) y += data.samples[readPtr];
		data.samples[readPtr] = y;
	}

	//
	int audioStarts(float thresh) {
		int sample = 0;
		//
		for (int i = 0; i < data.sampleLen; ++i) {
			if (data.samples[i] > thresh) {
				sample = i;
				break;
			}
		}
		//
		return sample;
	}

	//
	void resize(int newSize) {
		//
		delete[] data.samples;
		//
		data.sampleLen = newSize;
		data.samples = new float[data.sampleLen];
	}

	//
	void normalize() {
		float highest = 0;
		//find highest sample val
		for (int i = 0; i < data.sampleLen; ++i) {
			if (data.samples[i] > highest) {
				highest = data.samples[i];
			}
		}
		//
		float scale = 1 / highest;
		//scale accordingly
		for (int i = 0; i < data.sampleLen; ++i) {
			data.samples[i] = data.samples[i]*scale;
		}
	}

	//
	void cleanup() {
    	delete[] data.samples;
    }
};



#endif /* SAMPLEPLAYER_H_ */
