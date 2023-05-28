#include "daisy_patch.h"
#include "daisysp.h"
#include "dsp-headers/dsp/wavetable.hpp"
#include "shaper.h"

#include <array>

using namespace daisy;
using namespace daisysp;


void UpdateControls();

DaisyPatch hw;

float freq = 440.f;

Oscillator osc;
Shaper<1024> shaper;
Shaper<1024> shaper2;

dspheaders::WaveTable wt;

float x = 0,y = 0;

uint8_t selected = 0;

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
        out[1][i] = shaper2.process(shaper.process(s))*0.5 + s*0.5;
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
    uint_fast8_t x = 0;
    uint_fast8_t y = 0;     
    for (uint_fast8_t i = 0; i < 8; ++i) {
       x = 16*i;
       y = 32;
       hw.display.DrawRect(x, y-ws[i]*16, 5+x, y, true, selected==i);
       y = 63; 
       hw.display.DrawRect(x, y-ws2[i]*16, 5+x, y, true, selected==i);
    }

    hw.display.Update();
}

void UpdateControls()
{
    hw.ProcessAllControls();

    //knobs
    float ctrl = hw.GetKnobValue((DaisyPatch::Ctrl)0);
    int32_t enc = hw.encoder.Increment();
    if (enc)
        selected = (selected + enc)&7;
        
    freq = powf(2.f,6.0f*ctrl) * 40.f; 
    wt.setFreq(freq);
    float ctrl1 = hw.GetKnobValue((DaisyPatch::Ctrl)1);
    float ctrl2 = hw.GetKnobValue((DaisyPatch::Ctrl)2);

    if (abs(x - ctrl1) > 0.01) { 
        x = ctrl1;
        shaper.setWeight(selected, x);
    }
    if (abs(y - ctrl2) > 0.01) {
        y = ctrl2; 
        shaper2.setWeight(selected, y); 
    }
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(48); // number of samples handled per callback
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
