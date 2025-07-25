juce::AudioProcessorValueTreeState::ParameterLayout PrecisionDriveAudioProcessor::createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> p;

    p.push_back (std::make_unique<AudioParameterFloat>("drive",    "Drive",    NormalisableRange<float>(0.f, 1.f), 0.25f));
    p.push_back (std::make_unique<AudioParameterInt>  ("tonePos",  "Tone",     0, 6, 3));    // SW1 7-way
    p.push_back (std::make_unique<AudioParameterFloat>("bright",   "Bright",   NormalisableRange<float>(0.f, 1.f), 0.5f));
    p.push_back (std::make_unique<AudioParameterFloat>("volume",   "Volume",   NormalisableRange<float>(-24.f, 12.f), 0.f));
    p.push_back (std::make_unique<AudioParameterBool> ("gate",     "Gate",     false));
    p.push_back (std::make_unique<AudioParameterChoice>("oversmpl", "OS",      StringArray{ "1x","2x","4x","8x" }, 2));
    p.push_back (std::make_unique<AudioParameterChoice>("clipModel","Clip",    StringArray{ "tanh","diodePair" }, 1));
    p.push_back (std::make_unique<AudioParameterBool> ("bypass",   "Bypass",   false));

    return { p.begin(), p.end() };
}
