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
float p1;
float p2;

void InitializeADC();
void ProcessADC();

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	//calculate & update pot/param values
	ProcessADC();
	delay.setDelaySize(p1);
	delay.setFeedback(p2);

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
	float targetp1 = hw.adc.GetFloat(KnobOne) * 39690.0f + 4410.0f;
	float targetp2 = hw.adc.GetFloat(KnobTwo) * 0.95f; 

	p1 += 0.002f * (targetp1 - p1);
	p2 += 0.002f * (targetp2 - p2);
}
