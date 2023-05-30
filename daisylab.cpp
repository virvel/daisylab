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
Shaper<512> shaper;
Shaper<512> shaper2;
ReverbSc rev;

Oscillator sine;

namespace pie {
    constexpr float pi = 3.14159265358979323846f; 
    constexpr float twopi = 2.f*pi; 
}

float waver[512];
float x = 0,y = 0;
float ph = 0.f;
float inc = 0.00001f;

uint_fast8_t selected = 0;
#define REV 1

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    UpdateControls(); for (size_t i = 0; i < size; i++)
    {
        out[0][i] = 0.f;
        out[1][i] = 0.f;
        out[2][i] = 0.f;
        out[3][i] = 0.f;
        float s = sine.Process();
        out[0][i] = s;
        out[1][i] = shaper2.process(shaper.process(s))*0.5 + s*0.5;
        float revL, revR; 
        rev.Process(out[1][i], out[1][i], &revL, &revR); 
        out[1][i] += revL;
        float * ws = shaper.getWeights();
        for (uint_fast8_t i = 0; i < 8; ++i) {
             ws[i] = waver[(64*i + static_cast<int>(ph*512))%512]; 
        }
        ph = fmod(ph + inc, 1.f);
    }
}

void UpdateOled()
{
    hw.display.Fill(false);
    
    const float * ws = shaper.getWeights();
    const float * ws2 = shaper2.getWeights();

    std::string str  = std::to_string(int(freq));
    char* cstr = &str[0];
    hw.display.SetCursor(0, 0);
    hw.display.WriteString(cstr, Font_7x10, true);
    uint_fast8_t x = 0;
    uint_fast8_t y = 0;     
    for (uint_fast8_t i = 0; i < 8; ++i) {
       x = 16*i;
       y = 32;
       hw.display.DrawRect(x, y-ws[i]*16, 12+x, y, true, selected==i);
       y = 63; 
       hw.display.DrawRect(x, y-ws2[i]*16, 12+x, y, true, selected==i);
    }

    hw.display.Update();
}

void UpdateControls()
{
    hw.ProcessAllControls();

    //knobs
    float ctrl = hw.GetKnobValue((DaisyPatch::Ctrl)0);
    uint_fast8_t enc = hw.encoder.Increment();
    if (enc)
        selected = (selected + enc)&7;
        
    freq = powf(2.f,6.0f*ctrl) * 40.f; 
    sine.SetFreq(freq);
    float ctrl1 = hw.GetKnobValue((DaisyPatch::Ctrl)1);
    float ctrl2 = hw.GetKnobValue((DaisyPatch::Ctrl)2);
    float z = hw.GetKnobValue((DaisyPatch::Ctrl)3)/10e3;
    inc = z;

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
    rev.Init(48000); 
    rev.SetFeedback(0.7);
    rev.SetLpFreq(3000);
    shaper.init();
    shaper2.init();
    sine.Init(48000);
    sine.SetAmp(1.f);
    sine.Reset();
    constexpr float maxn = 512.f; 
    for (int i = 0; i < 512; ++i) {
        waver[i] = sin(pie::twopi*static_cast<float>(i)/maxn); 
        waver[i] = waver[i] * waver[i];
    }
 
    hw.StartAdc();
    hw.StartAudio(AudioCallback);
    while(1) {
        UpdateOled();
        hw.DelayMs(30);
    }
}
