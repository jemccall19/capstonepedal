#include "daisy_seed.h"
#include "daisysp.h"
#include "Delay.h"

using namespace daisy;
using namespace daisy::seed;
using namespace daisysp;


//init pots
#define POT1 A0
#define POT2 A1

enum AdcChannel {
	KnobOne,
	KnobTwo,
	NUM_ADC_CHANNELS
};

static const float sampleRate = 48000.0f;


//init class objects
DaisySeed hw;
Delay delay(48000.0f, 24000.0f, 0.5f, sampleRate);

//pot variables
float x;
float y;

void InitializeADC();
void ProcessADC();

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	//calculate & update pot/param values
	ProcessADC();
	delay.setDelaySize(x);
	delay.setFeedback(y);

	for (size_t i = 0; i < size; i++)
	{
		//signal path
		float sig = in[1][i];
		sig = delay.ProcessSample(sig);
		out[0][i] = sig;
		out[1][i] = sig;
		
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	InitializeADC();

	hw.StartAudio(AudioCallback);

	while(1) {}
}

void InitializeADC() {
	AdcChannelConfig adcChannelConfig[NUM_ADC_CHANNELS];
	adcChannelConfig[KnobOne].InitSingle(POT1);
	adcChannelConfig[KnobTwo].InitSingle(POT2);

	hw.adc.Init(adcChannelConfig, NUM_ADC_CHANNELS);
	hw.adc.Start();
}

void ProcessADC() {
	
	//pot values SMOOTHED
	float targetX = hw.adc.GetFloat(KnobOne) * 39690.0f + 4410.0f;
	float targetY = hw.adc.GetFloat(KnobTwo) * 0.95f; 

	x += 0.002f * (targetX - x);
	y += 0.002f * (targetY - y);
}
