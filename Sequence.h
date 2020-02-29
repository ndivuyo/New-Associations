/*

Dillon Bastan 2020
Structure for controlling the sequence of events

 */

#ifndef SEQUENCE_H_
#define SEQUENCE_H_

#include <string>
#include <SamplePlayer.h>


//
struct Sequence {
	const int CHANNEL_FEAR = 0;
	const int CHANNEL_DESIRE = 1;
	//
	SamplePlayer playerFear;
	SamplePlayer playerDesire;
	SamplePlayer playerFearQuestion;
	SamplePlayer playerDesireQuestion;
	int answerSize = 20;	//in seconds
	float speechLoThresh;
	float speechHiThresh;
	float questionGain;
	//
	bool answerBegan = false;
	bool finishDetected = false;
	int timeFinish;
	int finishThresh = 2000;
	//
	float pressureLvl = 0;
	float pressureThresh;
	//
	bool sitting = false;
	bool sitDownDetected = false;
	int timeSatDown;
	int sitdownThresh = 3000;
	bool releaseDetected = false;
	int timeReleased;
	int releaseThresh = 2000;
	//
	int frame;


	//
	Sequence(float loThresh, float hiThresh, float pressureLvlThresh, float questionLvl) {
		speechLoThresh = loThresh;
		speechHiThresh = hiThresh;
		pressureThresh = pressureLvlThresh;
		questionGain = questionLvl;
	}


	//
	void setup(BelaContext *context, std::string filenameFear, std::string filenameDesire) {
		answerSize *= context->audioSampleRate;
		//
		playerFear.setup("", answerSize);
		playerDesire.setup("", answerSize);
		playerFearQuestion.setup(filenameFear, 0);
		playerDesireQuestion.setup(filenameDesire, 0);
		playerFearQuestion.normalize();
		playerDesireQuestion.normalize();
		//
		frame = -1;
	}

	//
	void run(float inFear, float inDesire, float inMic, float *outSums, int millis) {
		// Detect a new sequence if a new person sits down
		if ( detectNewSequence(millis) ) reset(outSums);
		// Play sequence frame
		switch(frame) {
			// Ask Fear Question
			case 0:
				outSums[CHANNEL_FEAR] = playerFearQuestion.play() * questionGain;
				if (playerFearQuestion.resetFlag) frame++;
				break;
			// Record Fear Answer
			case 1:
				playerFear.record(inMic, 0);
				playerFear.play();
				// Detect beginning of the answer
				if (!answerBegan) { if (inMic > speechHiThresh) answerBegan = true; }
				// Detect answer ending
				else if ( answerFinished(inMic, millis) ) {
					playerFear.loopEnd = playerFear.readPtr;
					frame++;
					answerBegan = false;
				}
				break;
			// Ask Desire Question
			case 2:
				outSums[CHANNEL_DESIRE] = playerDesireQuestion.play() * questionGain;
				if (playerDesireQuestion.resetFlag) frame++;
				break;
			// Record Desire Answer
			case 3:
				playerDesire.record(inMic, 0);
				playerDesire.play();
				// Detect beginning of the answer
				if (!answerBegan) { if (inMic > speechHiThresh) answerBegan = true; } 
				// Detect answer ending
				else if ( answerFinished(inMic, millis) ) {
					playerDesire.loopEnd = playerDesire.readPtr;
					processAnswers();
					frame++;
					answerBegan = false;
				}
				break;
			// Overdub Playback of the answers
			case 4:
				playerFear.record(inFear, 1);
				outSums[CHANNEL_FEAR] = playerFear.play();
				playerDesire.record(inDesire, 1);
				outSums[CHANNEL_DESIRE] = playerDesire.play();
				break;
			
		}
	}


	// Detect if person is done speaking
	bool answerFinished(float input, int millis) {
		bool finished = false;
		float absIn = std::abs(input);
		// If silence has been detected
		if (finishDetected) {
			// If speech continues
			if (absIn > speechLoThresh) {
				finishDetected = false;
			}
			// Silent long enough to be finished
			else if (millis-timeFinish > finishThresh) {
				finishDetected = false;
				finished = true;
			}
		}
		// Detect silence
		else if (absIn < speechLoThresh) {
			finishDetected = true;
			timeFinish = millis;
		}
		//
		return finished;
	}


	//
	void processAnswers() {
		int loopLen = 0;
		// Get longest loop length
		playerFear.loopStart = playerFear.audioStarts(speechHiThresh);
		int fearLen = playerFear.loopEnd - playerFear.loopStart;
		loopLen = fearLen;
		playerDesire.loopStart = playerDesire.audioStarts(speechHiThresh);
		int desireLen = playerDesire.loopEnd - playerDesire.loopStart;
		if (desireLen > loopLen) loopLen = desireLen;
		// Reposition loops to longest length
		loopLen += loopLen * 0.8;
		playerFear.loopEnd = playerFear.loopStart + loopLen;
		playerDesire.loopEnd = playerDesire.loopStart + loopLen;
		// Start fear at beg and Desire at halfway for ping pong
		playerFear.setPos(playerFear.loopStart);
		playerDesire.setPos(playerDesire.loopStart + loopLen/2);
		//Normalize volumes
		playerFear.normalize();
		playerDesire.normalize();
	}


	//
	bool detectNewSequence(int millis) {
		bool triggerNewSequence = false;
		// If new person has sat (after seat has been empty)
		if (sitDownDetected) {
			// Stood up or moved in seat
			if (pressureLvl < pressureThresh) {
				sitDownDetected = false;
			}
			// If new persons sat down long enough
			else if (millis-timeSatDown > sitdownThresh) {
				sitDownDetected = false;
				sitting = true;
				triggerNewSequence = true;
			}
		} 
		// If someone sitting has stood up (or moved)
		else if (releaseDetected) {
			// Sat back down (so was a movement and not someone leaving)
			if (pressureLvl > pressureThresh) {
				releaseDetected = false;
			}
			// Stoop up long enough
			else if (millis-timeReleased > releaseThresh) {
				releaseDetected = false;
				sitting = false;
			}
		}
		// Detect new person sitting down
		else if (!sitting && pressureLvl > pressureThresh) {
			sitDownDetected = true;
			timeSatDown = millis;
		}
		// Detect sitting person standing
		else if (sitting && pressureLvl < pressureThresh) {
			releaseDetected = true;
			timeReleased = millis;
		}
		//
		return triggerNewSequence;
	}


	//
	void reset(float *outSums) {
		outSums[0] = 0;
		outSums[1] = 0;
		playerFear.resize(answerSize);
		playerFear.setPos(0);
		playerDesire.resize(answerSize);
		playerDesire.setPos(0);
		playerFearQuestion.setPos(0);
		playerDesireQuestion.setPos(0);
		frame = 0;
		finishDetected = false;
		answerBegan = false;
	}


	//
	void cleanup() {
		playerFear.cleanup();
		playerDesire.cleanup();
		playerFearQuestion.cleanup();
		playerDesireQuestion.cleanup();
	}
};



#endif /* SEQUENCE_H_ */
