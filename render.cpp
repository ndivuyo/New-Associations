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
float fearMicGain = 3;
float desireMicGain = 3;
float questionGain = 0.85;
float loThreshSpeech = 0.039;
float hiThreshSpeech = 0.039;
float sensorThresh = 0.5;
// IO
int audioFramesPerAnalogFrame;
float outSums[N_AUDIO_CHANNELS];
// 
Sequence sequence(loThreshSpeech, hiThreshSpeech, sensorThresh, questionGain);




/***************************  UTILITY    ********************************/

//
int getMillis(BelaContext *context) {
	return 1000 * (double)(context->audioFramesElapsed)/(double)context->audioSampleRate;
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
	int millis = getMillis(context);
	// iterate through audio frames
	for (unsigned int n = 0; n < context->audioFrames; ++n) {
		// Read Analog Ins (Pressure Sensors)
		if(audioFramesPerAnalogFrame && !(n % audioFramesPerAnalogFrame)) {
			int analogFrame = n/audioFramesPerAnalogFrame;
			//Add each sensor in to pressure level for human detection
			sequence.pressureLvl = 0;
			for (int i = 0; i < N_SENSORS; ++i) {
				sequence.pressureLvl += analogRead(context, analogFrame, i);
			}
			// Scale pressure with # of sensors
			sequence.pressureLvl = sequence.pressureLvl/N_SENSORS;
			// Control brightness of lights with sclaled amplitudes of speakers
			for (int i = 0; i < 2; ++i) {
				float lv = pow(std::abs(outSums[i]), 1.63)*0.258 + 0.226;
				analogWrite(context, analogFrame, i, lv);
			}
		}
		// Read Audio ins
		float inL = audioRead(context, n, 0) * fearMicGain;
		float inR = audioRead(context, n, 1) * desireMicGain;
		float inMic = audioRead(context, n, 2) * micGain;
		// Run sequence
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