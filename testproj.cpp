#include "daisy_seed.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisy::seed;
using namespace daisysp;

#define POT1 A0
#define POT2 A1

enum AdcChannel {
	KnobOne,
	KnobTwo,
	NUM_ADC_CHANNELS
};

DaisySeed hw;
Chorus chorus;
Tremolo tremolo;
float x;
float y;

void InitializeADC();
void ProcessADC();

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
	float sig;
	ProcessADC();
	tremolo.SetDepth(1);
	tremolo.SetFreq(y);
	chorus.SetLfoDepth(x);

	for (size_t i = 0; i < size; i++)
	{
		
		sig = in[1][i] * 10;
		sig = tremolo.Process(sig);
		sig = chorus.Process(sig);
		out[0][i] = sig;
		out[1][i] = sig;
		
	}
}

int main(void)
{
	hw.Init();
	hw.SetAudioBlockSize(4); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
	hw.StartAudio(AudioCallback);

	InitializeADC();

	float sampleRate = hw.AudioSampleRate();

	chorus.Init(sampleRate);
	tremolo.Init(sampleRate);
	tremolo.SetFreq(5);

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
	x = hw.adc.GetFloat(KnobOne);
	y = hw.adc.GetFloat(KnobTwo) * 10.0f; 
}
