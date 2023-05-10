#include "daisy_patch.h"
#include "daisysp.h"
#include "dsp-headers/dsp/wavetable.hpp"

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

const int tablen = 512;
std::array<dspheaders::WaveTable, 4> wt;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    UpdateControls(); 
    for (size_t i = 0; i < size; i++)
    {
        out[0][i] = 0.f;
        out[1][i] = 0.f;
        out[2][i] = 0.f;
        out[3][i] = 0.f;
        float a = (1.f - x) * wt[0].play() + x * wt[1].play();
        float b = (1.f - x) * wt[2].play() + x * wt[3].play();
        out[0][i] = (1.f - y) * a + y * b;  
    }
}

void UpdateOled()
{
    hw.display.Fill(false);

    std::string str  = std::to_string(int(freq));
    char*       cstr = &str[0];
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(cstr, Font_7x10, true);
    
    for (int i = 0; i < 1024; ++i) {
       hw.display.DrawPixel(positions[i][0], 50-positions[i][1], true);    
    }

    hw.display.Update();
}

void UpdateControls()
{
    hw.ProcessAnalogControls();

    //knobs
    float ctrl = hw.GetKnobValue((DaisyPatch::Ctrl)0);
    freq = powf(2.f,6.0f*ctrl) * 50.f + 30.f; //Hz
    wt[0].setFreq(freq);
    wt[1].setFreq(freq);
    wt[2].setFreq(freq);
    wt[3].setFreq(freq);
    x = hw.GetKnobValue((DaisyPatch::Ctrl)1);
    y = hw.GetKnobValue((DaisyPatch::Ctrl)2);
    positions[k][0] = int(x*50);
    positions[k][1] = int(y*50);
    k = (k+1)%1024;
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(4); // number of samples handled per callback
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    
    wt[0].setWave(TRIANGLE);
    wt[1].setWave(SAW);
    wt[2].setWave(SINE);
    wt[3].setWave(SQUARE);
 
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    while(1) {
        UpdateOled();
        hw.DelayMs(30);
    }
}
