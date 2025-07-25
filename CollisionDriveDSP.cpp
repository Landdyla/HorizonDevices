#include "CollisionDriveDSP.h"
#include <array>

using namespace juce;

namespace
{
    // ----------------------------------------------------------------------------
    // TODO: Replace these with the exact SW1 C7..C13 values from the Collision Drive PDF
    // (values below are placeholders but in the right ballpark for the behaviour)
    // ----------------------------------------------------------------------------
    constexpr std::array<double, 7> toneCaps = {
        220e-12, 330e-12, 470e-12, 680e-12, 1e-9, 1.5e-9, 2.2e-9
    };

    // TODO: replace with the real feedback resistor used with those caps
    constexpr double Rfb = 1000.0; // ohms

    inline int osIndexToFactor (int idx)
    {
        switch (idx)
        {
            case 0: return 1;
            case 1: return 2;
            case 2: return 4;
            case 3: return 8;
        }
        return 4;
    }
}

void CollisionDriveDSP::prepare (double sampleRate, int /*blockSize*/, int numChannels)
{
    sr = sampleRate;
    channels = numChannels;

    osFactor = osIndexToFactor (params.oversmpl);
    oversampler.reset (new dsp::Oversampling<float> (numChannels,
                                                     (unsigned) std::log2 (osFactor),
                                                     dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
                                                     true,  // isMinimisingAlias
                                                     true)); // useIntegerLatency

    toneLP.resize (numChannels);
    brightShelf.resize (numChannels);

    updateFilters();
}

void CollisionDriveDSP::reset()
{
    if (oversampler) oversampler->reset();
    for (auto& f : toneLP)     f.reset();
    for (auto& f : brightShelf) f.reset();
}

void CollisionDriveDSP::setParams (const Params& p)
{
    params = p;

    const int newFactor = osIndexToFactor (params.oversmpl);
    if (newFactor != osFactor)
        prepare (sr, 512, channels); // quick rebuild

    updateFilters();
}

void CollisionDriveDSP::updateFilters()
{
    const int idx = jlimit (0, (int) toneCaps.size() - 1, params.tonePos);
    const double C = toneCaps[(size_t) idx];
    const double fc = 1.0 / (2.0 * MathConstants<double>::pi * Rfb * C);

    for (int ch = 0; ch < channels; ++ch)
    {
        *toneLP[ch].coefficients = *dsp::IIR::Coefficients<float>::makeLowPass (sr * osFactor, (float) fc, 0.7071f);

        const float brightDb = jmap (params.bright, 0.0f, 1.0f, 0.0f, 12.0f);
        *brightShelf[ch].coefficients = *dsp::IIR::Coefficients<float>::makeHighShelf (sr, 2000.0f, 0.7071f,
                                                                                        Decibels::decibelsToGain (brightDb));
    }
}

void CollisionDriveDSP::process (AudioBuffer<float>& buffer)
{
    if (params.bypass)
        return;

    dsp::AudioBlock<float> inBlock (buffer);
    auto osBlock = oversampler->processSamplesUp (inBlock);

    const float preGain = jmap (params.drive, 1.0f, 25.0f);

    for (size_t ch = 0; ch < osBlock.getNumChannels(); ++ch)
    {
        auto* data = osBlock.getChannelPointer (ch);
        const auto n = osBlock.getNumSamples();

        auto& lp    = toneLP[(int) ch];
        auto& shelf = brightShelf[(int) ch];

        for (size_t i = 0; i < n; ++i)
        {
            float x = data[i] * preGain;

            // “feedback” LP approximation
            x = lp.processSample (x);

            // Clippers
            x = (params.clipModel == 0) ? tanhClip (x) : diodeClipApprox (x);

            // simple hard gate (off by default) – PD has one, but not Collision Drive
            if (params.gateOn && std::abs (x) < 0.005f)
                x = 0.0f;

            data[i] = x;
        }

        // We can apply bright shelf here or after downsampling;
        // here is fine (it’s linear)
        shelf.processSamples (data, (int) n);
    }

    oversampler->processSamplesDown (inBlock);

    buffer.applyGain (Decibels::decibelsToGain (params.volumeDb));
}

float CollisionDriveDSP::tanhClip (float vIn) const
{
    const float a = jmap (params.drive, 1.0f, 20.0f);
    return std::tanh (vIn * a);
}

// Symmetric diode-ish soft knee. Cheap but musical.
// Swap with a Shockley/Newton solver if you want exactness.
float CollisionDriveDSP::diodeClipApprox (float vIn) const
{
    constexpr float knee = 0.22f; // ~ Vf of 1N4148-ish
    if (vIn > knee)
        return knee + (vIn - knee) / (1.0f + (vIn - knee) * 10.0f);
    if (vIn < -knee)
        return -knee + (vIn + knee) / (1.0f - (vIn + knee) * 10.0f);
    return vIn;
}
