#pragma once
#include "PluginProcessor.h"

//==============================================================================
// Dark, glowing look: neon arc knobs, soft bloom, pill toggle
class RoyalLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
};

//==============================================================================
class RoyalStripEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit RoyalStripEditor (RoyalStripProcessor&);
    ~RoyalStripEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void drawMeter (juce::Graphics&, juce::Rectangle<float>, float norm, juce::Colour, const juce::String&);

    struct Knob
    {
        std::unique_ptr<juce::Slider> s;
        std::unique_ptr<juce::Label> l;
        std::unique_ptr<juce::ComboBox> combo;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> a;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> comboAttachment; // attachments destroyed first
    };

    struct Panel
    {
        juce::String title;
        juce::Colour colour;
        int row = 0, first = 0, count = 0;
        bool toggle = false;
        bool lastOn = true;
        int btn = -1;
        juce::Rectangle<int> bounds;
        std::unique_ptr<juce::TextButton> presetBtn;
    };

    RoyalStripProcessor& proc;
    RoyalLookAndFeel laf;
    std::vector<std::unique_ptr<Knob>> knobs;
    std::vector<Panel> panels;

    void refreshDim();
    void drawLoudness (juce::Graphics&);
    void applyModulePreset (const juce::String&, int);
    void showPresetMenu (const juce::String&);
    void saveUserPreset();
    void loadUserPreset();
    void drawEqGraph (juce::Graphics&, juce::Rectangle<int>, bool dynamicMode);
    std::vector<std::unique_ptr<juce::ToggleButton>> powerBtns;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> powerAtts; // destroyed before buttons

    juce::ToggleButton extBtn { "EXT SC" };
    juce::ToggleButton bypassBtn { "BYPASS" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAtt;
    juce::TextButton resetBtn { "RESET I" };
    juce::TextButton savePresetBtn { "SAVE PRESET" }, loadPresetBtn { "LOAD PRESET" }, graphBtn { "EQ GRAPH" }, sizeBtn { "EXPAND" };
    std::unique_ptr<juce::FileChooser> fileChooser;
    int graphMode = 0;
    bool expanded = false;
    juce::Rectangle<int> loudRect, stripRect;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> extAtt;

    static constexpr int headerH = 78;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RoyalStripEditor)
};
