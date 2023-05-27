#include "daisy_patch.h"
#include "daisysp.h"
#include "dsp-headers/dsp/wavetable.hpp"
#include "shaper.h"

#include <array>

using namespace daisy;
using namespace daisysp;


void UpdateControls();
const int sampleRate = 48000;

DaisyPatch hw;

float x = 0.f;
float y = 0.f;
float freq = 440.f;
int positions[1024][2];
int k = 0;
float prevx = 0.f;
float prevy = 0.f;

Oscillator osc;
Shaper<1024> shaper;
Shaper<1024> shaper2;

dspheaders::WaveTable wt;


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    UpdateControls(); 
    for (size_t i = 0; i < size; i++)
    {
        out[0][i] = 0.f;
        out[1][i] = 0.f;
        out[2][i] = 0.f;
        out[3][i] = 0.f;
        float s = wt.play();
        out[0][i] = s;
        out[1][i] = shaper2.process(shaper.process(s));
    }
}

    float r = 0.f;
void UpdateOled()
{
    hw.display.Fill(false);
    
    const float * ws = shaper.getWeights();
    const float * ws2 = shaper2.getWeights();

    std::string str  = std::to_string(int(freq));
    char*       cstr = &str[0];
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(cstr, Font_7x10, true);
    
    for (int i = 0; i < 8; ++i) {
       hw.display.DrawLine(i*16, 32, i*16, 16-ws[i]*16, true);    
       hw.display.DrawLine(i*16, 64, i*16, 48-ws2[i]*48, true);    
    }

    hw.display.Update();
}

void UpdateControls()
{
    hw.ProcessAnalogControls();

    //knobs
    float ctrl = hw.GetKnobValue((DaisyPatch::Ctrl)0);
    freq = powf(2.f,6.0f*ctrl) * 50.f + 30.f; //Hz
    wt.setFreq(freq);
    x = hw.GetKnobValue((DaisyPatch::Ctrl)1);
    y = hw.GetKnobValue((DaisyPatch::Ctrl)2);
    shaper.setWeight(1, x);
    shaper.setWeight(3, y); 
    k = (k+1)%1024;
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(4); // number of samples handled per callback
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    
    shaper.init();
    shaper2.init();

    wt.setWave(SINE);
 
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    while(1) {
        UpdateOled();
        hw.DelayMs(30);
    }
}
