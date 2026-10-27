#pragma once
#include "DSP.h"
#include <vector>

class RoyalStripProcessor : public juce::AudioProcessor
{
public:
    RoyalStripProcessor();
    ~RoyalStripProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "The Royal Strip"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorParameter* getBypassParameter() const override { return apvts.getParameter ("bypass"); }

    float lufsMomentary()  const { return loud.momentary.load(); }
    float lufsShort()      const { return loud.shortTerm.load(); }
    float lufsIntegrated() const { return loud.integrated(); }
    float peakHoldDb()     const { return loud.peakHold.load(); }
    void resetLoudness() { resetLoud.store (true); }

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    juce::AudioProcessorValueTreeState apvts;

    // meters read by the editor
    std::atomic<float> meterIn { 0.0f }, meterOut { 0.0f }, grComp { 0.0f }, grGlue { 0.0f }, grLim { 0.0f }, grGate { 0.0f };
    std::atomic<float> grPlosive { 0.0f };

private:
    double sr = 44100.0;

    juce::SmoothedValue<float> inGain, outGain;
    std::vector<float> monoScratch;
    float plosiveEnv = 0.0f, plosiveLow = 0.0f;

    royal::Gate gate;
    royal::PreAmp pre;
    royal::PitchCorrector pitch;
    royal::Limiter limiter;
    royal::Biquad hpf, lowShelf, midPk, hiShelf, airShelf;
    royal::DynBand dyn, de;
    royal::Comp comp, glue, mb[3];
    float compMakeup = 1.0f, glueMakeup = 1.0f;

    royal::LR4 lp1, hp1, lp2, hp2, apLo, apHi;

    juce::AudioBuffer<float> delayBuf;
    int writePos = 0;
    float dampL = 0.0f, dampR = 0.0f;

    juce::Reverb reverb;
    float duckEnv = 0.0f;

    royal::Loudness loud;
    std::atomic<bool> resetLoud { false };

    juce::AudioBuffer<float> bypassBuf;
    int bypassWp = 0, bypassSize = 1;
    juce::SmoothedValue<float> bypassMix;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoyalStripProcessor)
};
