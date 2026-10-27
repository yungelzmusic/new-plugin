#include "PluginEditor.h"

namespace
{
const juce::Colour kCyan   (0xff34e0ff);
const juce::Colour kMagenta(0xffff3df2);
const juce::Colour kViolet (0xffa78bff);
const juce::Colour kGold   (0xffffc857);
const juce::Colour kGreen  (0xff4dffb4);

struct KD { const char* id; const char* label; const char* unit; int dec; };
struct SD { const char* title; juce::Colour colour; int row; std::vector<KD> k; bool toggle; };

const char* shortLabel (const juce::String& t)
{
    static const std::pair<const char*, const char*> m[] = {
        { "GATE", "GATE" }, { "PRE-AMP", "PRE" }, { "PITCH CORRECT", "PITCH" }, { "PLOSIVE", "PLOS" }, { "EQ", "EQ" }, { "DYNAMIC EQ", "DYN EQ" },
        { "DE-ESSER", "DE-ESS" }, { "COMPRESSOR", "COMP" }, { "MULTIBAND", "MULTI" }, { "GLUE", "GLUE" }, { "SAT", "SAT" },
        { "AIR", "AIR" }, { "DELAY", "DELAY" }, { "REVERB", "REVERB" }, { "DUCK", "DUCK" }, { "LIMITER", "LIMIT" } };
    for (auto& e : m) if (t == e.first) return e.second;
    return "";
}

const char* onIdFor (const juce::String& t)
{
    static const std::pair<const char*, const char*> m[] = {
        { "GATE", "gateOn" }, { "PRE-AMP", "preOn" }, { "PITCH CORRECT", "pitchOn" }, { "PLOSIVE", "plosiveOn" }, { "EQ", "eqOn" }, { "DYNAMIC EQ", "dynOn" },
        { "DE-ESSER", "deOn" }, { "COMPRESSOR", "compOn" }, { "MULTIBAND", "mbOn" }, { "GLUE", "glueOn" }, { "SAT", "satOn" },
        { "AIR", "airOn" }, { "DELAY", "delayOn" }, { "REVERB", "revOn" }, { "DUCK", "duckOn" }, { "LIMITER", "limOn" } };
    for (auto& e : m) if (t == e.first) return e.second;
    return nullptr;
}

const std::vector<SD>& defs()
{
    static const std::vector<SD> d =
    {
        // row 0
        { "GATE", kGreen, 0, { { "gateThresh", "THRESH", " dB", 1 }, { "gateRange", "RANGE", " dB", 1 }, { "gateAtk", "ATTACK", " ms", 1 },
                               { "gateHold", "HOLD", " ms", 0 }, { "gateRel", "RELEASE", " ms", 0 } }, false },
        { "PRE-AMP", kGold, 0, { { "preDrive", "DRIVE", " dB", 1 }, { "preWarm", "WARMTH", "", 2 }, { "preTone", "TONE", " dB", 1 } }, false },
        { "PITCH CORRECT", kMagenta, 0, { { "pitchKey", "KEY", "", -1 }, { "pitchScale", "SCALE", "", -2 }, { "pitchSpeed", "SPEED", " ms", 0 },
                                          { "pitchAmt", "AMOUNT", "", 2 } }, false },

        { "PLOSIVE", kGold, 0, { { "plosiveFreq", "FREQ", " Hz", 0 }, { "plosiveThresh", "THRESH", " dB", 1 }, { "plosiveAmt", "AMOUNT", " dB", 1 } }, false },

        // row 1
        { "EQ", kCyan, 1, { { "hpf", "HPF", " Hz", 0 }, { "lowGain", "LOW", " dB", 1 }, { "midGain", "MID", " dB", 1 },
                            { "midFreq", "FREQ", " Hz", 0 }, { "midQ", "Q", "", 2 }, { "highGain", "HIGH", " dB", 1 } }, false },
        { "DYNAMIC EQ", kMagenta, 1, { { "dynFreq", "FREQ", " Hz", 0 }, { "dynThresh", "THRESH", " dB", 1 }, { "dynRange", "RANGE", " dB", 1 } }, false },
        { "DE-ESSER", kViolet, 1, { { "deFreq", "FREQ", " Hz", 0 }, { "deThresh", "THRESH", " dB", 1 }, { "deAmt", "AMOUNT", " dB", 1 } }, false },

        // row 2
        { "COMPRESSOR", kCyan, 2, { { "compThresh", "THRESH", " dB", 1 }, { "compRatio", "RATIO", ":1", 1 }, { "compAtk", "ATTACK", " ms", 1 },
                                    { "compRel", "RELEASE", " ms", 0 }, { "compMakeup", "MAKEUP", " dB", 1 } }, false },
        { "MULTIBAND", kGold, 2, { { "mbLowX", "LOW X", " Hz", 0 }, { "mbHighX", "HIGH X", " Hz", 0 }, { "mbLowThr", "LOW", " dB", 1 },
                                   { "mbMidThr", "MID", " dB", 1 }, { "mbHighThr", "HIGH", " dB", 1 }, { "mbRatio", "RATIO", ":1", 1 } }, false },
        { "GLUE", kViolet, 2, { { "glueThresh", "THRESH", " dB", 1 }, { "glueRatio", "RATIO", ":1", 1 }, { "glueRel", "RELEASE", " ms", 0 } }, false },

        // row 3
        { "SAT", kMagenta, 3, { { "satDrive", "DRIVE", " dB", 1 }, { "satMix", "MIX", "", 2 } }, false },
        { "AIR", kCyan, 3, { { "air", "AIR", "", 2 } }, false },
        { "DELAY", kGreen, 3, { { "delayTime", "TIME", " ms", 0 }, { "delayFb", "FDBK", "", 2 }, { "delayMix", "MIX", "", 2 } }, false },
        { "REVERB", kGold, 3, { { "revSize", "SIZE", "", 2 }, { "revDamp", "DAMP", "", 2 }, { "revMix", "MIX", "", 2 } }, false },
        { "DUCK", kGreen, 3, { { "duckAmt", "DUCK", " dB", 1 }, { "duckThr", "THRESH", " dB", 1 }, { "duckRel", "RELEASE", " ms", 0 } }, true },
        { "LIMITER", kViolet, 3, { { "limGain", "GAIN", " dB", 1 }, { "limCeil", "CEIL", " dB", 1 }, { "limRel", "RELEASE", " ms", 0 } }, false },

        // side column
        { "I / O", kCyan, 4, { { "inGain", "INPUT", " dB", 1 }, { "outGain", "OUTPUT", " dB", 1 } }, false },
    };
    return d;
}
}

//==============================================================================
void RoyalLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                        float a0, float a1, juce::Slider& s)
{
    auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (4.0f);
    const float d = std::min (b.getWidth(), b.getHeight());
    auto r = juce::Rectangle<float> (d, d).withCentre (b.getCentre());
    const float rad = d * 0.5f;
    const auto c = r.getCentre();
    const float ang = a0 + pos * (a1 - a0);
    const auto accent = s.findColour (juce::Slider::rotarySliderFillColourId);
    const juce::PathStrokeType thin (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);

    juce::Path track;
    track.addCentredArc (c.x, c.y, rad, rad, 0.0f, a0, a1, true);
    g.setColour (juce::Colour (0xff1a1d28));
    g.strokePath (track, thin);

    juce::Path val;
    val.addCentredArc (c.x, c.y, rad, rad, 0.0f, a0, ang, true);
    for (int i = 4; i >= 1; --i)   // bloom
    {
        g.setColour (accent.withAlpha (0.05f * (float) (5 - i)));
        g.strokePath (val, juce::PathStrokeType (3.0f + (float) i * 2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
    g.setColour (accent);
    g.strokePath (val, thin);

    auto body = r.reduced (rad * 0.28f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2e3242), body.getX(), body.getY(),
                                             juce::Colour (0xff090a0f), body.getRight(), body.getBottom(), false));
    g.fillEllipse (body);
    g.setColour (juce::Colour (0xff3a3f52));
    g.drawEllipse (body, 1.0f);

    juce::Path p;
    p.addRoundedRectangle (-1.5f, -body.getHeight() * 0.5f + 3.0f, 3.0f, body.getHeight() * 0.28f, 1.5f);
    g.setColour (accent.brighter (0.5f));
    g.fillPath (p, juce::AffineTransform::rotation (ang).translated (c.x, c.y));
}

void RoyalLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool, bool)
{
    const bool on = b.getToggleState();

    const auto col = b.findColour (juce::ToggleButton::tickColourId);
    auto r = b.getLocalBounds().toFloat().reduced (2.0f);

    g.setColour (on ? col.withAlpha (0.16f) : juce::Colour (0xff14161e));
    g.fillRoundedRectangle (r, 7.0f);
    if (on)
        for (int i = 3; i >= 1; --i)
        {
            g.setColour (col.withAlpha (0.07f * (float) (4 - i)));
            g.drawRoundedRectangle (r.expanded ((float) i * 1.5f), 7.0f + (float) i, 1.5f);
        }
    g.setColour (on ? col : juce::Colour (0xff3a3f52));
    g.drawRoundedRectangle (r, 7.0f, 1.2f);
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.drawText (b.getButtonText(), r.toNearestInt(), juce::Justification::centred);
}

//==============================================================================
RoyalStripEditor::RoyalStripEditor (RoyalStripProcessor& p) : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&laf);

    for (auto& sd : defs())
    {
        Panel panel;
        panel.title = sd.title;
        panel.colour = sd.colour;
        panel.row = sd.row;
        panel.first = (int) knobs.size();
        panel.count = (int) sd.k.size();
        panel.toggle = sd.toggle;

        for (auto& kd : sd.k)
        {
            auto k = std::make_unique<Knob>();
            k->s = std::make_unique<juce::Slider> (juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow);
            k->s->setColour (juce::Slider::rotarySliderFillColourId, sd.colour);
            k->s->setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffb8c0d8));
            k->s->setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
            k->s->setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);

            k->l = std::make_unique<juce::Label> (juce::String(), kd.label);
            k->l->setJustificationType (juce::Justification::centred);
            k->l->setFont (juce::FontOptions (10.0f, juce::Font::bold));
            k->l->setColour (juce::Label::textColourId, juce::Colour (0xff7b839c));

            const bool isKey = juce::String (kd.id) == "pitchKey";
            const bool isScale = juce::String (kd.id) == "pitchScale";
            if (isKey || isScale)
            {
                k->combo = std::make_unique<juce::ComboBox>();
                if (isKey)
                {
                    const char* keys[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
                    for (int i = 0; i < 12; ++i) k->combo->addItem (keys[i], i + 1);
                }
                else
                {
                    k->combo->addItem ("Chromatic", 1);
                    k->combo->addItem ("Major", 2);
                    k->combo->addItem ("Minor", 3);
                }
                k->combo->setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff10131b));
                k->combo->setColour (juce::ComboBox::textColourId, juce::Colours::white);
                k->combo->setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff41465b));
                k->comboAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, kd.id, *k->combo);
                addAndMakeVisible (*k->combo);
            }
            else
                k->a = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, kd.id, *k->s);

            // set after the attachment so our text format wins
            const int dec = kd.dec;
            const juce::String unit (kd.unit);
            k->s->textFromValueFunction = [dec, unit] (double v)
            {
                static const char* keys[12] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
                static const char* scales[3] = { "CHROM", "MAJOR", "MINOR" };
                if (dec == -1) return juce::String (keys[juce::jlimit (0, 11, (int) std::lround (v))]);
                if (dec == -2) return juce::String (scales[juce::jlimit (0, 2, (int) std::lround (v))]);
                return juce::String (v, dec) + unit;
            };
            k->s->valueFromTextFunction = [] (const juce::String& t) { return t.getDoubleValue(); };
            k->s->updateText();

            if (! k->combo) addAndMakeVisible (*k->s);
            addAndMakeVisible (*k->l);
            knobs.push_back (std::move (k));
        }
        if (auto* onId = onIdFor (panel.title))
        {
            auto pb = std::make_unique<juce::ToggleButton> (shortLabel (panel.title));
            pb->setColour (juce::ToggleButton::tickColourId, sd.colour);
            addAndMakeVisible (*pb);
            powerAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, onId, *pb));
            panel.btn = (int) powerBtns.size();
            powerBtns.push_back (std::move (pb));
        }
        if (panel.title != "I / O")
        {
            panel.presetBtn = std::make_unique<juce::TextButton> ("PRESETS");
            panel.presetBtn->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff171b27));
            panel.presetBtn->setColour (juce::TextButton::textColourOffId, panel.colour);
            const auto title = panel.title;
            panel.presetBtn->onClick = [this, title] { showPresetMenu (title); };
            addAndMakeVisible (*panel.presetBtn);
        }
        panels.push_back (std::move (panel));
    }

    extBtn.setColour (juce::ToggleButton::tickColourId, kGreen);
    addAndMakeVisible (extBtn);

    bypassBtn.setColour (juce::ToggleButton::tickColourId, juce::Colour (0xffff5470));
    addAndMakeVisible (bypassBtn);
    bypassAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "bypass", bypassBtn);

    resetBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff14161e));
    resetBtn.setColour (juce::TextButton::textColourOffId, kGold);
    resetBtn.onClick = [this] { proc.resetLoudness(); };
    addAndMakeVisible (resetBtn);

    auto styleHeaderButton = [] (juce::TextButton& b, juce::Colour c)
    { b.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff151925)); b.setColour (juce::TextButton::textColourOffId, c); };
    styleHeaderButton (savePresetBtn, kGold); styleHeaderButton (loadPresetBtn, kCyan);
    styleHeaderButton (graphBtn, kMagenta); styleHeaderButton (sizeBtn, kGreen);
    savePresetBtn.onClick = [this] { saveUserPreset(); };
    loadPresetBtn.onClick = [this] { loadUserPreset(); };
    graphBtn.onClick = [this] {
        graphMode = (graphMode + 1) % 3;
        graphBtn.setButtonText (graphMode == 0 ? "EQ GRAPH" : (graphMode == 1 ? "EQ CURVE" : "DYN CURVE"));
        for (auto& panel : panels) if (panel.title == "EQ")
            for (int k = 0; k < panel.count; ++k)
            { auto& knob = *knobs[(size_t) (panel.first + k)]; knob.s->setVisible (graphMode == 0); knob.l->setVisible (graphMode == 0); }
        repaint();
    };
    sizeBtn.onClick = [this] { expanded = ! expanded; setSize (expanded ? 1600 : 1284, expanded ? 1000 : 820); sizeBtn.setButtonText (expanded ? "COMPACT" : "EXPAND"); };
    addAndMakeVisible (savePresetBtn); addAndMakeVisible (loadPresetBtn); addAndMakeVisible (graphBtn); addAndMakeVisible (sizeBtn);
    setResizable (true, true);
    setResizeLimits (1000, 700, 1920, 1200);
    extAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, "duckExt", extBtn);

    setSize (1284, 820);
    startTimerHz (30);
}

RoyalStripEditor::~RoyalStripEditor() { setLookAndFeel (nullptr); }

void RoyalStripEditor::refreshDim()
{
    for (auto& p : panels)
    {
        const bool on = p.btn < 0 || powerBtns[(size_t) p.btn]->getToggleState();
        if (on == p.lastOn) continue;
        p.lastOn = on;
        const float a = on ? 1.0f : 0.3f;
        for (int k = 0; k < p.count; ++k)
        {
            knobs[(size_t) (p.first + k)]->s->setAlpha (a);
            knobs[(size_t) (p.first + k)]->l->setAlpha (a);
        }
        repaint (p.bounds);
    }
}

void RoyalStripEditor::timerCallback()
{
    refreshDim();
    repaint (0, 0, getWidth(), headerH);
    repaint (loudRect);
}

//==============================================================================
void RoyalStripEditor::drawMeter (juce::Graphics& g, juce::Rectangle<float> r, float norm, juce::Colour c, const juce::String& label)
{
    norm = juce::jlimit (0.0f, 1.0f, norm);
    g.setColour (juce::Colour (0xff7b839c));
    g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    g.drawText (label, (int) r.getX() - 44, (int) r.getY() - 3, 40, 16, juce::Justification::centredRight);

    g.setColour (juce::Colour (0xff12141c));
    g.fillRoundedRectangle (r, 3.0f);

    auto bar = r.withWidth (r.getWidth() * norm);
    if (bar.getWidth() > 1.0f)
    {
        for (int i = 3; i >= 1; --i)
        {
            g.setColour (c.withAlpha (0.07f));
            g.fillRoundedRectangle (bar.expanded ((float) i * 1.5f), 3.0f + (float) i);
        }
        g.setColour (c);
        g.fillRoundedRectangle (bar, 3.0f);
    }
}

void RoyalStripEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff06070b));
    g.setGradientFill (juce::ColourGradient (kCyan.withAlpha (0.10f), (float) getWidth() * 0.5f, -60.0f,
                                             juce::Colours::transparentBlack, (float) getWidth() * 0.5f, (float) getHeight() * 0.75f, true));
    g.fillAll();

    // title with bloom
    const juce::String title ("T H E   R O Y A L   S T R I P");
    const auto tr = juce::Rectangle<int> (24, 12, 460, 36);
    g.setFont (juce::FontOptions (28.0f, juce::Font::bold));
    for (int i = 3; i >= 1; --i)
    {
        g.setColour (kCyan.withAlpha (0.05f * (float) (4 - i)));
        for (int dx : { -i, i })
            for (int dy : { -i, i })
                g.drawText (title, tr.translated (dx, dy), juce::Justification::centredLeft);
    }
    g.setColour (juce::Colours::white);
    g.drawText (title, tr, juce::Justification::centredLeft);

    g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    g.setColour (kCyan.withAlpha (0.7f));
    g.drawText ("DEVELOPED BY ELZ DA PRYNCE  /  EQ  •  DYNAMICS  •  SPACE", 26, 50, 520, 14, juce::Justification::centredLeft);

    // meters
    auto lvl = [] (float v) { return juce::jmap (juce::jlimit (-60.0f, 0.0f, royal::gainToDb (v)), -60.0f, 0.0f, 0.0f, 1.0f); };
    const int colW = 208, mw = 150;
    const int x0 = getWidth() - 16 - 3 * colW;
    drawMeter (g, { (float) (x0 + 44),            18.0f, (float) mw, 10.0f }, lvl (proc.meterIn.load()),  kCyan,    "IN");
    drawMeter (g, { (float) (x0 + 44),            42.0f, (float) mw, 10.0f }, lvl (proc.meterOut.load()), kGreen,   "OUT");
    drawMeter (g, { (float) (x0 + colW + 44),     18.0f, (float) mw, 10.0f }, proc.grComp.load() / 18.0f, kMagenta, "COMP");
    drawMeter (g, { (float) (x0 + colW + 44),     42.0f, (float) mw, 10.0f }, proc.grGlue.load() / 12.0f, kViolet,  "GLUE");
    drawMeter (g, { (float) (x0 + 2 * colW + 44), 18.0f, (float) mw, 10.0f }, proc.grLim.load()  / 12.0f, kGold,    "LIM");
    drawMeter (g, { (float) (x0 + 2 * colW + 44), 42.0f, (float) mw, 10.0f }, proc.grGate.load() / 40.0f, kGreen,   "GATE");

    // panels
    for (auto& p : panels)
    {
        auto b = p.bounds.toFloat();
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff141722), b.getX(), b.getY(),
                                                 juce::Colour (0xff0a0b10), b.getX(), b.getBottom(), false));
        g.fillRoundedRectangle (b, 10.0f);
        g.setColour (juce::Colour (0xff222639));
        g.drawRoundedRectangle (b.reduced (0.5f), 10.0f, 1.0f);

        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.setColour (p.lastOn ? p.colour : p.colour.withAlpha (0.35f));
        g.drawText (p.title, (int) b.getX() + 14, (int) b.getY() + 8, (int) b.getWidth() - (p.presetBtn ? 100 : 28), 14, juce::Justification::centredLeft);
        g.setColour (p.colour.withAlpha (0.18f));
        g.fillRoundedRectangle (b.getX() + 12.0f, b.getY() + 23.0f, 30.0f, 4.0f, 2.0f);
        g.setColour (p.colour);
        g.fillRoundedRectangle (b.getX() + 14.0f, b.getY() + 24.0f, 26.0f, 1.5f, 1.0f);
    }

    if (graphMode != 0)
        for (auto& p : panels) if (p.title == "EQ") drawEqGraph (g, p.bounds.reduced (10, 28), graphMode == 2);

    drawLoudness (g);
}

void RoyalStripEditor::drawLoudness (juce::Graphics& g)
{
    if (loudRect.isEmpty()) return;
    auto b = loudRect.toFloat();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff141722), b.getX(), b.getY(),
                                             juce::Colour (0xff0a0b10), b.getX(), b.getBottom(), false));
    g.fillRoundedRectangle (b, 10.0f);
    g.setColour (juce::Colour (0xff222639));
    g.drawRoundedRectangle (b.reduced (0.5f), 10.0f, 1.0f);

    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.setColour (kGold);
    g.drawText ("LOUDNESS", (int) b.getX() + 14, (int) b.getY() + 8, (int) b.getWidth() - 28, 14, juce::Justification::centredLeft);
    g.fillRoundedRectangle (b.getX() + 14.0f, b.getY() + 24.0f, 26.0f, 1.5f, 1.0f);

    const float bw = 26.0f, bx1 = b.getX() + 14.0f, bx2 = b.getX() + 56.0f;
    const float by = b.getY() + 40.0f;
    const float bh = b.getHeight() - 40.0f - 22.0f - 8.0f - 4 * 19.0f - 10.0f - 38.0f - 6.0f;

    auto track = [&] (float x) { g.setColour (juce::Colour (0xff12141c)); g.fillRoundedRectangle (x, by, bw, bh, 4.0f); };
    track (bx1); track (bx2);

    // volume (output peak) bar, bottom-up
    const float volDb = royal::gainToDb (proc.meterOut.load());
    const float vol = juce::jlimit (0.0f, 1.0f, (volDb + 60.0f) / 60.0f);
    const float seg[4] = { 0.0f, 0.8f, 0.95f, 1.0f };
    const juce::Colour segCol[3] = { kGreen, kGold, kMagenta };
    for (int i = 0; i < 3; ++i)
    {
        const float top = std::min (seg[i + 1], vol);
        if (top <= seg[i]) continue;
        const float y = by + bh - bh * top, h = bh * (top - seg[i]);
        g.setColour (segCol[i].withAlpha (0.10f));
        g.fillRoundedRectangle (bx2 - 3.0f, y - 2.0f, bw + 6.0f, h + 4.0f, 5.0f);
        g.setColour (segCol[i]);
        g.fillRect (bx2, y, bw, h);
    }

    // gain-reduction bar, top-down (comp + glue + limiter)
    const float gr = juce::jlimit (0.0f, 1.0f, (proc.grComp.load() + proc.grGlue.load() + proc.grLim.load()) / 24.0f);
    if (gr > 0.002f)
    {
        g.setColour (kMagenta.withAlpha (0.10f));
        g.fillRoundedRectangle (bx1 - 3.0f, by - 2.0f, bw + 6.0f, bh * gr + 4.0f, 5.0f);
        g.setColour (kMagenta);
        g.fillRect (bx1, by, bw, bh * gr);
    }

    // scale for the volume bar
    g.setFont (juce::FontOptions (9.0f));
    g.setColour (juce::Colour (0xff7b839c));
    for (int db : { 0, -12, -24, -36, -48 })
    {
        const float y = by + bh - bh * (float) (db + 60) / 60.0f;
        g.fillRect (bx2 + bw, y, 4.0f, 1.0f);
        g.drawText (juce::String (db), (int) (bx2 + bw + 6.0f), (int) y - 5, 30, 10, juce::Justification::centredLeft);
    }

    g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    g.drawText ("GR",  (int) bx1 - 6, (int) (by + bh + 3.0f), (int) bw + 12, 14, juce::Justification::centred);
    g.drawText ("VOL", (int) bx2 - 6, (int) (by + bh + 3.0f), (int) bw + 12, 14, juce::Justification::centred);

    // numeric readouts
    auto fmt = [] (float v) { return v <= -119.0f ? juce::String ("--") : juce::String (v, 1); };
    const struct { const char* name; juce::String val; } rows[4] = {
        { "M  LUFS", fmt (proc.lufsMomentary()) },
        { "S  LUFS", fmt (proc.lufsShort()) },
        { "I  LUFS", fmt (proc.lufsIntegrated()) },
        { "PEAK dB", fmt (proc.peakHoldDb()) } };
    float ry = by + bh + 26.0f;
    for (auto& r : rows)
    {
        g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        g.setColour (juce::Colour (0xff7b839c));
        g.drawText (r.name, (int) b.getX() + 14, (int) ry, 70, 16, juce::Justification::centredLeft);
        g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
        g.setColour (juce::Colours::white);
        g.drawText (r.val, (int) b.getRight() - 64, (int) ry, 50, 16, juce::Justification::centredRight);
        ry += 19.0f;
    }
}

void RoyalStripEditor::showPresetMenu (const juce::String& module)
{
    juce::PopupMenu menu;
    menu.addItem (1, "Reset / Neutral");
    if (module == "GATE") { menu.addItem (2, "Gentle Vocal"); menu.addItem (3, "Tight Booth"); }
    else if (module == "PRE-AMP") { menu.addItem (2, "Warm Tube"); menu.addItem (3, "Clean"); menu.addItem (4, "Driven"); }
    else if (module == "PITCH CORRECT") { menu.addItem (2, "Natural"); menu.addItem (3, "Fast / Modern"); menu.addItem (4, "Hard Tune"); }
    else if (module == "PLOSIVE") { menu.addItem (2, "Light Control"); menu.addItem (3, "Strong Plosives"); }
    else if (module == "EQ") { menu.addItem (2, "Vocal Warmth"); menu.addItem (3, "Presence & Air"); menu.addItem (4, "Telephone"); }
    else if (module == "DYNAMIC EQ") { menu.addItem (2, "Light Taming"); menu.addItem (3, "Harshness Control"); }
    else if (module == "DE-ESSER") { menu.addItem (2, "Soft Sibilance"); menu.addItem (3, "Strong Sibilance"); }
    else if (module == "COMPRESSOR") { menu.addItem (2, "Smooth Vocal"); menu.addItem (3, "Punchy Vocal"); menu.addItem (4, "Heavy Control"); }
    else if (module == "MULTIBAND") { menu.addItem (2, "Gentle Glue"); menu.addItem (3, "Tight Low End"); }
    else if (module == "GLUE") { menu.addItem (2, "Mix Glue"); menu.addItem (3, "Firm Glue"); }
    else if (module == "SAT") { menu.addItem (2, "Subtle Warmth"); menu.addItem (3, "Tape Drive"); }
    else if (module == "AIR") { menu.addItem (2, "Silky Air"); menu.addItem (3, "Bright Air"); }
    else if (module == "DELAY") { menu.addItem (2, "Slapback"); menu.addItem (3, "Quarter Note Feel"); }
    else if (module == "REVERB") { menu.addItem (2, "Small Room"); menu.addItem (3, "Vocal Plate"); menu.addItem (4, "Big Space"); }
    else if (module == "DUCK") { menu.addItem (2, "Subtle Duck"); menu.addItem (3, "Deep Duck"); }
    else if (module == "LIMITER") { menu.addItem (2, "Transparent Peak Catch"); menu.addItem (3, "Louder Output"); }
    menu.showMenuAsync (juce::PopupMenu::Options(), [this, module] (int result) { if (result > 0) applyModulePreset (module, result); });
}

void RoyalStripEditor::applyModulePreset (const juce::String& module, int choice)
{
    auto set = [this] (const char* id, float value)
    {
        if (auto* p = proc.apvts.getParameter (id)) p->setValueNotifyingHost (p->convertTo0to1 (value));
    };
    auto reset = [this, &module, &set]
    {
        const auto& d = defs();
        for (const auto& panel : d) if (module == panel.title)
            for (const auto& kd : panel.k) if (auto* p = proc.apvts.getParameter (kd.id)) p->setValueNotifyingHost (p->getDefaultValue());
    };
    if (choice == 1) { reset(); return; }
    if (module == "GATE") { set ("gateThresh", choice == 2 ? -48 : -34); set ("gateRange", choice == 2 ? 18 : 42); set ("gateAtk", 1.0f); set ("gateRel", choice == 2 ? 220 : 120); }
    else if (module == "PRE-AMP") { set ("preDrive", choice == 2 ? 5 : choice == 3 ? 0 : 14); set ("preWarm", choice == 2 ? 0.55f : choice == 3 ? 0.0f : 0.7f); set ("preTone", choice == 4 ? 1.5f : 0.0f); }
    else if (module == "PITCH CORRECT") { set ("pitchSpeed", choice == 2 ? 55 : choice == 3 ? 12 : 2); set ("pitchAmt", choice == 2 ? 0.45f : choice == 3 ? 0.8f : 1.0f); }
    else if (module == "PLOSIVE") { set ("plosiveFreq", 120); set ("plosiveThresh", choice == 2 ? -20 : -30); set ("plosiveAmt", choice == 2 ? 5 : 14); }
    else if (module == "EQ") { set ("hpf", choice == 2 ? 85 : choice == 3 ? 100 : 250); set ("lowGain", choice == 2 ? 1.5f : choice == 4 ? -4.0f : 0.0f); set ("midGain", choice == 2 ? 1.0f : choice == 3 ? 2.0f : -2.0f); set ("midFreq", choice == 4 ? 1200 : choice == 3 ? 4500 : 1800); set ("midQ", choice == 4 ? 1.5f : 1.0f); set ("highGain", choice == 2 ? 1.0f : choice == 3 ? 3.0f : -1.0f); }
    else if (module == "DYNAMIC EQ") { set ("dynFreq", choice == 2 ? 3500 : 5200); set ("dynThresh", choice == 2 ? -24 : -32); set ("dynRange", choice == 2 ? 3 : 7); }
    else if (module == "DE-ESSER") { set ("deFreq", 7000); set ("deThresh", choice == 2 ? -24 : -34); set ("deAmt", choice == 2 ? 4 : 10); }
    else if (module == "COMPRESSOR") { set ("compThresh", choice == 2 ? -18 : choice == 3 ? -14 : -22); set ("compRatio", choice == 2 ? 2.5f : choice == 3 ? 4.0f : 7.0f); set ("compAtk", choice == 2 ? 18 : choice == 3 ? 7 : 2); set ("compRel", choice == 2 ? 150 : 95); set ("compMakeup", choice == 2 ? 2 : 4); }
    else if (module == "MULTIBAND") { set ("mbRatio", choice == 2 ? 1.8f : 3.0f); set ("mbLowThr", -22); set ("mbMidThr", -24); set ("mbHighThr", -28); }
    else if (module == "GLUE") { set ("glueThresh", choice == 2 ? -12 : -20); set ("glueRatio", choice == 2 ? 2.0f : 3.0f); set ("glueRel", choice == 2 ? 220 : 120); }
    else if (module == "SAT") { set ("satDrive", choice == 2 ? 3 : 10); set ("satMix", choice == 2 ? 0.2f : 0.45f); }
    else if (module == "AIR") set ("air", choice == 2 ? 0.25f : 0.55f);
    else if (module == "DELAY") { set ("delayTime", choice == 2 ? 90 : 500); set ("delayFb", choice == 2 ? 0.12f : 0.32f); set ("delayMix", choice == 2 ? 0.12f : 0.22f); }
    else if (module == "REVERB") { set ("revSize", choice == 2 ? 0.25f : choice == 3 ? 0.55f : 0.85f); set ("revDamp", 0.45f); set ("revMix", choice == 2 ? 0.12f : choice == 3 ? 0.2f : 0.32f); }
    else if (module == "DUCK") { set ("duckAmt", choice == 2 ? 3 : 10); set ("duckThr", choice == 2 ? -24 : -36); set ("duckRel", choice == 2 ? 180 : 100); }
    else if (module == "LIMITER") { set ("limGain", choice == 2 ? 0 : 4); set ("limCeil", -1.0f); set ("limRel", 80); }
}

void RoyalStripEditor::saveUserPreset()
{
    fileChooser = std::make_unique<juce::FileChooser> ("Save Royal Strip preset", juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*.rstrip");
    fileChooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& fc)
        {
            auto file = fc.getResult(); if (file == juce::File()) return;
            auto target = file.hasFileExtension ("rstrip") ? file : file.withFileExtension ("rstrip");
            juce::MemoryBlock state; proc.getStateInformation (state); target.replaceWithData (state.getData(), state.getSize());
        });
}

void RoyalStripEditor::loadUserPreset()
{
    fileChooser = std::make_unique<juce::FileChooser> ("Load Royal Strip preset", juce::File::getSpecialLocation (juce::File::userDocumentsDirectory), "*.rstrip");
    fileChooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& fc)
        {
            auto file = fc.getResult(); if (! file.existsAsFile()) return;
            juce::MemoryBlock state; if (file.loadFileAsData (state)) proc.setStateInformation (state.getData(), (int) state.getSize());
        });
}

void RoyalStripEditor::drawEqGraph (juce::Graphics& g, juce::Rectangle<int> r, bool dynamicMode)
{
    if (r.getWidth() < 80 || r.getHeight() < 35) return;
    g.setColour (juce::Colour (0xf00a0c13)); g.fillRoundedRectangle (r.toFloat(), 6.0f);
    g.setColour (juce::Colour (0xff34394d));
    for (int i = 1; i < 5; ++i) g.drawHorizontalLine (r.getY() + r.getHeight() * i / 5, (float) r.getX(), (float) r.getRight());
    for (int i = 1; i < 6; ++i) g.drawVerticalLine (r.getX() + r.getWidth() * i / 6, (float) r.getY(), (float) r.getBottom());
    juce::Path curve;
    const float midY = (float) r.getCentreY();
    for (int i = 0; i <= 100; ++i)
    {
        const float x = (float) i / 100.0f;
        const float low = proc.apvts.getRawParameterValue ("lowGain")->load() * std::exp (-x * 10.0f);
        const float midFreq = proc.apvts.getRawParameterValue ("midFreq")->load();
        const float midPos = juce::jlimit (0.0f, 1.0f, std::log (midFreq / 20.0f) / std::log (20000.0f / 20.0f));
        const float mid = proc.apvts.getRawParameterValue ("midGain")->load() * std::exp (-std::pow ((x - midPos) * 7.0f, 2.0f));
        const float high = proc.apvts.getRawParameterValue ("highGain")->load() * (1.0f / (1.0f + std::exp (-(x - 0.72f) * 22.0f)));
        const float dyn = dynamicMode ? -proc.apvts.getRawParameterValue ("dynRange")->load() * std::exp (-std::pow ((x - 0.7f) * 8.0f, 2.0f)) : 0.0f;
        const float db = low + mid + high + dyn;
        const float y = juce::jlimit ((float) r.getY(), (float) r.getBottom(), midY - db * (float) r.getHeight() / 30.0f);
        if (i == 0) curve.startNewSubPath ((float) r.getX(), y); else curve.lineTo ((float) r.getX() + x * (float) r.getWidth(), y);
    }
    g.setColour (dynamicMode ? kMagenta : kCyan);
    g.strokePath (curve, juce::PathStrokeType (2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setFont (juce::FontOptions (9.0f, juce::Font::bold)); g.drawText (dynamicMode ? "DYNAMIC RESPONSE (VISUAL GUIDE)" : "EQ RESPONSE (VISUAL GUIDE)", r.reduced (5, 2).removeFromTop (12), juce::Justification::centredLeft);
}

void RoyalStripEditor::resized()
{
    const int margin = 16, gap = 10, sideW = 154;
    const int mainW = getWidth() - 2 * margin - sideW - gap;
    const int top = headerH + 42;
    const int headY = 48, headH = 24, headW = 100, headGap = 7;
    int hx = getWidth() - margin - 4 * headW - 3 * headGap;
    savePresetBtn.setBounds (hx, headY, headW, headH); hx += headW + headGap;
    loadPresetBtn.setBounds (hx, headY, headW, headH); hx += headW + headGap;
    graphBtn.setBounds (hx, headY, headW, headH); hx += headW + headGap;
    sizeBtn.setBounds (hx, headY, headW, headH);
    const int rowH = (getHeight() - top - margin - 3 * gap) / 4;

    // top strip: master bypass + one switch per module
    stripRect = { margin, headerH + 4, mainW, 30 };
    {
        const int bw = 92, sgap = 4, n = (int) powerBtns.size();
        bypassBtn.setBounds (stripRect.getX(), stripRect.getY(), bw, stripRect.getHeight());
        const int w = (mainW - bw - 12 - sgap * (n - 1)) / std::max (1, n);
        int x = stripRect.getX() + bw + 12;
        for (auto& pb : powerBtns) { pb->setBounds (x, stripRect.getY(), w, stripRect.getHeight()); x += w + sgap; }
    }

    auto placeKnobs = [this] (Panel& p, int ps)
    {
        auto inner = p.bounds.reduced (6, 0).withTrimmedTop (30).withTrimmedBottom (6);
        const int cw = inner.getWidth() / ps;
        for (int k = 0; k < p.count; ++k)
        {
            auto c = juce::Rectangle<int> (inner.getX() + k * cw, inner.getY(), cw, inner.getHeight());
            auto& kn = *knobs[(size_t) (p.first + k)];
            kn.l->setBounds (c.removeFromTop (14));
            if (kn.combo)
            {
                auto cb = c.reduced (2, std::max (2, c.getHeight() / 3));
                kn.combo->setBounds (cb);
            }
            else
            {
                kn.s->setTextBoxStyle (juce::Slider::TextBoxBelow, false, cw, 16);
                kn.s->setBounds (c);
            }
        }
        if (p.toggle)
            extBtn.setBounds (inner.getX() + p.count * cw + 4, inner.getCentreY() - 16, cw - 8, 32);
    };

    for (int r = 0; r < 4; ++r)
    {
        int slots = 0, cnt = 0;
        for (auto& p : panels)
            if (p.row == r) { slots += p.count + (p.toggle ? 1 : 0); ++cnt; }
        if (slots == 0) continue;

        const float cell = (float) (mainW - gap * (cnt - 1)) / (float) slots;
        float x = (float) margin;

        for (auto& p : panels)
        {
            if (p.row != r) continue;
            const int ps = p.count + (p.toggle ? 1 : 0);
            const int w = (int) (cell * (float) ps);
            p.bounds = { (int) x, top + r * (rowH + gap), w, rowH };
            if (p.presetBtn) p.presetBtn->setBounds (p.bounds.getRight() - 72, p.bounds.getY() + 5, 62, 18);
            placeKnobs (p, ps);
            x += (float) w + (float) gap;
        }
    }

    // side column: input/output knobs + loudness
    const int sx = getWidth() - margin - sideW;
    for (auto& p : panels)
        if (p.row == 4)
        {
            p.bounds = { sx, top, sideW, rowH };
            placeKnobs (p, p.count);
        }

    const int ly = top + rowH + gap;
    loudRect = { sx, ly, sideW, getHeight() - margin - ly };
    resetBtn.setBounds (loudRect.getX() + 14, loudRect.getBottom() - 38, loudRect.getWidth() - 28, 28);
    repaint();
}
