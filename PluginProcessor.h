#include "dsp/CollisionDriveDSP.h"

class PrecisionDriveAudioProcessor : public juce::AudioProcessor
{
    // ...
private:
    CollisionDriveDSP dsp;
    // ...
};
