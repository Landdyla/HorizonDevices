#pragma once
#include <JuceHeader.h>

class CollisionDriveDSP
{
public:
    struct Params
    {
        float drive     = 0.25f;  // 0..1
        int   tonePos   = 3;      // 0..6 -> SW1
        float bright    = 0.5f;   // 0..1
        float volumeDb  = 0.0f;   // -24..+12
        bool  gateOn    = false;
        int   oversmpl  = 2;      // 0=1x,1=2x,2=4x,3=8x
        int   clipModel = 1;      // 0=tanh, 1=diode
        bool  bypass    = false;
    };

    void prepare (double sampleRate, int blockSize, int numChannels);
    void reset();
    void setParams (const Params& p);
    void process (juce::AudioBuffer<float>& buffer);

private:
    void updateFilters();
    float tanhClip (float vIn) const;
    float diodeClipApprox (float vIn) const;

    double sr = 44100.0;
    int channels = 2;

    Params params;

    // Oversampling
    int osFactor = 4;
    std::unique_ptr<juce::dsp::Oversampling<float>> oversampler;

    // “Feedback” LP approximation (implemented as post-preamp LP)
    std::vector<juce::dsp::IIR::Filter<float>> toneLP;

    // Bright high shelf
    std::vector<juce::dsp::IIR::Filter<float>> brightShelf;
};
