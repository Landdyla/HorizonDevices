void PrecisionDriveAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ignoreUnused (midi);
    juce::ScopedNoDenormals noDenormals;

    CollisionDriveDSP::Params p;
    p.drive     = apvts.getRawParameterValue ("drive")->load();
    p.tonePos   = (int) apvts.getRawParameterValue ("tonePos")->load();
    p.bright    = apvts.getRawParameterValue ("bright")->load();
    p.volumeDb  = apvts.getRawParameterValue ("volume")->load();
    p.gateOn    = apvts.getRawParameterValue ("gate")->load() > 0.5f;
    p.oversmpl  = (int) apvts.getRawParameterValue ("oversmpl")->load();
    p.clipModel = (int) apvts.getRawParameterValue ("clipModel")->load();
    p.bypass    = apvts.getRawParameterValue ("bypass")->load() > 0.5f;

    dsp.setParams (p);
    dsp.process (buffer);
}
