#include "daisy_patch.h"
#include "daisysp.h"
#include "dsp-headers/dsp/wavetable.hpp"
#include "knot.h"

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

const int tablen = 512;
std::array<dspheaders::WaveTable, 4> wt;

Knot knot;

template<std::size_t Length, typename Generator>
constexpr auto lut(Generator&& f){
    using content_type = decltype(f(std::size_t{0}));
    std::array<content_type, Length> arr {};

    for(std::size_t i = 0; i < Length; i++){
        arr[i] = f(i);
    }

    return arr;
}

template <size_t n>
inline constexpr double cheb(double x) { return 2*x*cheb<n-1>(x) - cheb<n-2>(x); }
template <>
inline constexpr double cheb<0>(double x) { return 1;}
template <>
inline constexpr double cheb<1>(double x) { return x;}

template<std::size_t Length>
inline constexpr auto chebyshev_lut = lut<Length>([](std::size_t n){
    double x = 2.f*(static_cast<double>(n)/static_cast<double>(Length-1)) -1.f;
    return static_cast<float>(cheb<5>(x)); 
});

float theta = 0.f;
float inc = 10 * 2* M_PI / 48000.f; 

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
        out[1][i] = knot.Process1(theta);
        out[2][i] = knot.Process2(theta);
        out[3][i] = knot.Process3(theta);
        theta = fmod(theta + inc, 2*M_PI);
    }
}

    float r = 0.f;
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

    float omega = 0.f;
    for (int i = 0; i < 300; ++i) {
        omega = 2*M_PI*(float)i / float(300);
        float x = knot.Process1(omega);
        float y = knot.Process2(omega);
        float z = knot.Process3(omega);
        
        float rx = x * cos(r) + z * sin(r);
        float ry = y; 
        hw.display.DrawPixel(rx*15+60, ry*15+30, true);    
    }
     r += 0.01;

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
    x = hw.GetKnobValue((DaisyPatch::Ctrl)1)*0.5 + prevx*0.5;
    y = hw.GetKnobValue((DaisyPatch::Ctrl)2)*0.5 + prevx*0.5;
    prevx = x;
    prevy = y;
    positions[k][0] = int(x*50);
    positions[k][1] = int(y*50);
    k = (k+1)%1024;
    knot.setQ(hw.GetKnobValue((DaisyPatch::Ctrl)3)*10);
}

int main(void)
{
    hw.Init();
    hw.SetAudioBlockSize(4); // number of samples handled per callback
    hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    
    knot.Init();

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
