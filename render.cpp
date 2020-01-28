/*****

New Association (Sound Sculpture)
Dillon Bastan 2020

******/

#include <Bela.h>
#include <stdlib.h>
#include <cmath>
#include <string>
#include <SamplePlayer.h>
#include <Sequence.h>


/***************************  GLOBALS    ********************************/

// Constants
const int N_AUDIO_CHANNELS = 2;
const int N_SENSORS = 2;
// Parameters
float micGain = 35;
float fearMicGain = 0.6;
float desireMicGain = 0.6;
float questionGain = 1;
float loThreshSpeech = 0.039;
float hiThreshSpeech = 0.039;
float sensorThresh = 0.5;
// IO
int audioFramesPerAnalogFrame;
float outSums[N_AUDIO_CHANNELS];
// 
Sequence sequence(loThreshSpeech, hiThreshSpeech, sensorThresh, questionGain);
//
int printLock = 1;




/***************************  UTILITY    ********************************/

//
int getMillis(BelaContext *context) {
	return 1000 * (double)(context->audioFramesElapsed)/(double)context->audioSampleRate;
}

//
void printFloatRate(int rate, int millis, float toprint) {
	if (millis%rate == 0 && !printLock) {
		rt_printf("%f\n", toprint);
		printLock = 1;
	} else if (printLock == 1) printLock = 0;
}




/***************************  MAIN    ********************************/

//
bool setup(BelaContext *context, void *userData) {
	audioFramesPerAnalogFrame = context->audioFrames / context->analogFrames;
	sequence.setup(context, "fear.wav", "desire.wav");
	return true;
}


//
void render(BelaContext *context, void *userData) {
	//
	int millis = getMillis(context);
	// iterate through audio frames
	for (unsigned int n = 0; n < context->audioFrames; ++n) {
		// Read Analog Ins (Pressure Sensors)
		if(audioFramesPerAnalogFrame && !(n % audioFramesPerAnalogFrame)) {
			//Add to pressure level for sitting detection
			sequence.pressureLvl = 0;
			for (int i = 0; i < N_SENSORS; ++i) {
				sequence.pressureLvl += analogRead(context, n/audioFramesPerAnalogFrame, i);
			}
			//
			sequence.pressureLvl = sequence.pressureLvl/N_SENSORS;
			//printFloatRate(1000, millis, sequence.pressureLvl);
		}
		// Read Audio ins
		float inL = audioRead(context, n, 0) * fearMicGain;
		float inR = audioRead(context, n, 1) * desireMicGain;
		float inMic = audioRead(context, n, 2) * micGain;
		//printFloatRate(100, millis, std::abs(inMic));
		// Run Resquence
		sequence.run(inL, inR, inMic, outSums, millis);
		// Output to Audio Channels
		for (unsigned int ch = 0; ch < N_AUDIO_CHANNELS; ++ch) {
			audioWrite(context, n, ch, outSums[ch]);
		}
	}
}


//
void cleanup(BelaContext *context, void *userData){
	sequence.cleanup();
}