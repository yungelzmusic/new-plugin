#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace royal;

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout RoyalStripProcessor::createLayout()
{
    using Range = juce::NormalisableRange<float>;
    juce::AudioProcessorValueTreeState::ParameterLayout l;

    auto F = [&l] (const char* id, const char* name, float lo, float hi, float def, float skew = 1.0f)
    {
        l.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { id, 1 }, name,
                                                            Range (lo, hi, 0.01f, skew), def));
    };

    auto FS = [&l] (const char* id, const char* name, float lo, float hi, float def)
    {
        l.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { id, 1 }, name, Range (lo, hi, 1.0f), def));
    };

    F ("inGain", "Input", -24, 24, 0);
    F ("outGain", "Output", -24, 24, 0);

    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "bypass", 1 }, "Bypass", false));

    // Module on/off switches (all default ON)
    static const std::pair<const char*, const char*> switches[] = {
        { "gateOn", "Gate On" }, { "pitchOn", "Pitch On" }, { "preOn", "Preamp On" }, { "plosiveOn", "Plosive On" }, { "eqOn", "EQ On" },
        { "dynOn", "Dynamic EQ On" }, { "deOn", "De-esser On" }, { "compOn", "Compressor On" }, { "mbOn", "Multiband On" },
        { "satOn", "Saturation On" }, { "glueOn", "Glue On" }, { "airOn", "Air On" }, { "delayOn", "Delay On" },
        { "revOn", "Reverb On" }, { "duckOn", "Duck On" }, { "limOn", "Limiter On" } };
    for (auto& sw : switches)
        l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { sw.first, 1 }, sw.second, juce::String (sw.first) == "plosiveOn" ? false : true));

    // Gate (threshold at minimum = off)
    F ("gateThresh", "Gate Thresh", -80, 0, -80);
    F ("gateRange", "Gate Range", 0, 80, 40);
    F ("gateAtk", "Gate Attack", 0.1f, 50, 1, 0.4f);
    F ("gateHold", "Gate Hold", 0, 500, 50, 0.5f);
    F ("gateRel", "Gate Release", 5, 1000, 150, 0.5f);

    // Pitch correct (amount 0 = off)
    FS ("pitchKey", "Pitch Key", 0, 11, 0);
    FS ("pitchScale", "Pitch Scale", 0, 2, 0);
    F ("pitchSpeed", "Pitch Speed", 0, 200, 20, 0.5f);
    F ("pitchAmt", "Pitch Amount", 0, 1, 0);

    // Plosive control: transient low-frequency detector with gentle broadband attenuation.
    F ("plosiveFreq", "Plosive Frequency", 70, 220, 120, 0.5f);
    F ("plosiveThresh", "Plosive Threshold", -36, -6, -24);
    F ("plosiveAmt", "Plosive Amount", 0, 18, 6);

    // Pre-amp
    F ("preDrive", "Preamp Drive", 0, 30, 0);
    F ("preWarm", "Preamp Warmth", 0, 1, 0);
    F ("preTone", "Preamp Tone", -6, 6, 0);

    // Limiter
    F ("limGain", "Limiter Gain", 0, 12, 0);
    F ("limCeil", "Limiter Ceiling", -12, 0, -0.3f);
    F ("limRel", "Limiter Release", 10, 500, 80, 0.5f);

    // EQ
    F ("hpf", "HPF", 20, 400, 20, 0.4f);
    F ("lowGain", "Low", -12, 12, 0);
    F ("midGain", "Mid", -12, 12, 0);
    F ("midFreq", "Mid Freq", 200, 8000, 1000, 0.4f);
    F ("midQ", "Mid Q", 0.3f, 6, 1, 0.5f);
    F ("highGain", "High", -12, 12, 0);

    // Dynamic EQ (range 0 = off)
    F ("dynFreq", "Dyn Freq", 100, 10000, 3000, 0.4f);
    F ("dynThresh", "Dyn Thresh", -60, 0, -30);
    F ("dynRange", "Dyn Range", 0, 18, 0);

    // De-esser (amount 0 = off)
    F ("deFreq", "De-Ess Freq", 3000, 12000, 7000);
    F ("deThresh", "De-Ess Thresh", -60, 0, -30);
    F ("deAmt", "De-Ess Amount", 0, 15, 0);

    // Compressor
    F ("compThresh", "Comp Thresh", -40, 0, -12);
    F ("compRatio", "Comp Ratio", 1, 20, 3, 0.5f);
    F ("compAtk", "Comp Attack", 0.1f, 100, 10, 0.4f);
    F ("compRel", "Comp Release", 20, 1000, 120, 0.5f);
    F ("compMakeup", "Comp Makeup", 0, 18, 0);

    // Multiband (ratio 1 = off)
    F ("mbLowX", "MB Low X", 60, 500, 150, 0.5f);
    F ("mbHighX", "MB High X", 1000, 8000, 4000, 0.5f);
    F ("mbLowThr", "MB Low Thr", -40, 0, -20);
    F ("mbMidThr", "MB Mid Thr", -40, 0, -20);
    F ("mbHighThr", "MB High Thr", -40, 0, -20);
    F ("mbRatio", "MB Ratio", 1, 10, 1, 0.5f);

    // Saturation
    F ("satDrive", "Sat Drive", 0, 24, 0);
    F ("satMix", "Sat Mix", 0, 1, 0);

    // Glue
    F ("glueThresh", "Glue Thresh", -40, 0, 0);
    F ("glueRatio", "Glue Ratio", 1, 4, 2);
    F ("glueRel", "Glue Release", 50, 1200, 200, 0.5f);

    // Air
    F ("air", "Air", 0, 1, 0);

    // Delay
    F ("delayTime", "Delay Time", 10, 1000, 250, 0.5f);
    F ("delayFb", "Delay Feedback", 0, 0.9f, 0.3f);
    F ("delayMix", "Delay Mix", 0, 1, 0);

    // Reverb
    F ("revSize", "Reverb Size", 0, 1, 0.5f);
    F ("revDamp", "Reverb Damp", 0, 1, 0.5f);
    F ("revMix", "Reverb Mix", 0, 1, 0);

    // Ducking (amount 0 = off)
    F ("duckAmt", "Duck Amount", 0, 24, 0);
    F ("duckThr", "Duck Thresh", -60, 0, -30);
    F ("duckRel", "Duck Release", 20, 1000, 200, 0.5f);
    l.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "duckExt", 1 }, "Duck Ext Sidechain", false));

    return l;
}

//==============================================================================
RoyalStripProcessor::RoyalStripProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",     juce::AudioChannelSet::stereo(), true)
                          .withInput  ("Sidechain", juce::AudioChannelSet::stereo(), false)
                          .withOutput ("Output",    juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
}

bool RoyalStripProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    if (in != out) return false;
    if (in != juce::AudioChannelSet::mono() && in != juce::AudioChannelSet::stereo()) return false;
    const auto sc = layouts.getChannelSet (true, 1);
    return sc.isDisabled() || sc == juce::AudioChannelSet::mono() || sc == juce::AudioChannelSet::stereo();
}

void RoyalStripProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    monoScratch.assign ((size_t) std::max (1, samplesPerBlock), 0.0f);
    plosiveEnv = plosiveLow = 0.0f;

    inGain.reset (sr, 0.02);  inGain.setCurrentAndTargetValue (1.0f);
    outGain.reset (sr, 0.02); outGain.setCurrentAndTargetValue (1.0f);

    for (auto* b : { &hpf, &lowShelf, &midPk, &hiShelf, &airShelf }) b->reset();
    for (auto* x : { &lp1, &hp1, &lp2, &hp2, &apLo, &apHi }) x->reset();

    gate = {};
    pitch.prepare (sr);
    limiter.prepare (sr);
    setLatencySamples (pitch.latency() + limiter.latency());

    bypassSize = pitch.latency() + limiter.latency() + 1;
    bypassBuf.setSize (2, bypassSize);
    bypassBuf.clear();
    bypassWp = 0;
    bypassMix.reset (sr, 0.01);
    bypassMix.setCurrentAndTargetValue (0.0f);
    loud.prepare (sr);

    dyn.prepare (sr);
    de.prepare (sr);
    comp.prepare (sr);
    glue.prepare (sr);
    for (auto& m : mb) m.prepare (sr);

    delayBuf.setSize (2, (int) (sr * 1.1) + 8);
    delayBuf.clear();
    writePos = 0;
    dampL = dampR = 0.0f;

    reverb.setSampleRate (sr);
    reverb.reset();
    duckEnv = 0.0f;
}

//==============================================================================
void RoyalStripProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int n = buffer.getNumSamples();
    auto main = getBusBuffer (buffer, true, 0);
    if (main.getNumChannels() < 1) return;
    const bool mono = main.getNumChannels() == 1;
    if (mono && (int) monoScratch.size() < n) monoScratch.resize ((size_t) n);
    float* L = main.getWritePointer (0);
    float* R = mono ? monoScratch.data() : main.getWritePointer (1);
    if (mono) std::copy (L, L + n, R);

    auto P = [this] (const char* id) { return apvts.getRawParameterValue (id)->load(); };

    if (resetLoud.exchange (false)) loud.reset();
    bypassMix.setTargetValue (P ("bypass") > 0.5f ? 1.0f : 0.0f);

    auto On = [&P] (const char* id) { return P (id) > 0.5f; };
    const bool plosiveOn = On ("plosiveOn");
    const bool eqOn = On ("eqOn"), mbOn = On ("mbOn"), compOn = On ("compOn"), glueOn = On ("glueOn"), airOn = On ("airOn"),
               satOn = On ("satOn"), revOn = On ("revOn"), limOn = On ("limOn"), preOn = On ("preOn"), delayOn = On ("delayOn");

    // optional external sidechain for the ducker
    const float* scL = nullptr;
    const float* scR = nullptr;
    if (P ("duckExt") > 0.5f && getBusCount (true) > 1 && getBus (true, 1)->isEnabled())
    {
        auto sc = getBusBuffer (buffer, true, 1);
        if (sc.getNumChannels() > 0)
        {
            scL = sc.getReadPointer (0);
            scR = sc.getNumChannels() > 1 ? sc.getReadPointer (1) : scL;
        }
    }

    //--------------------------------------------------------------- parameters
    inGain.setTargetValue  (dbToGain (P ("inGain")));
    outGain.setTargetValue (dbToGain (P ("outGain")));

    const float gateThr = P ("gateThresh");
    const bool gateOn = On ("gateOn") && gateThr > -79.5f;
    gate.set (sr, gateThr, P ("gateRange"), P ("gateAtk"), P ("gateHold"), P ("gateRel"));
    pitch.setup ((int) P ("pitchKey"), (int) P ("pitchScale"), P ("pitchSpeed"), On ("pitchOn") ? P ("pitchAmt") : 0.0f);
    pre.set (sr, P ("preDrive"), P ("preWarm"), P ("preTone"));
    const float limGain = limOn ? dbToGain (P ("limGain")) : 1.0f;
    limiter.set (sr, limOn ? P ("limCeil") : 60.0f, P ("limRel"));

    hpf.highpass   (sr, P ("hpf"), 0.7071f);
    lowShelf.lowShelf (sr, 100.0f, P ("lowGain"));
    midPk.peak     (sr, P ("midFreq"), P ("midQ"), P ("midGain"));
    hiShelf.highShelf (sr, 8000.0f, P ("highGain"));

    dyn.setup (P ("dynFreq"), 1.5f, P ("dynThresh"), On ("dynOn") ? P ("dynRange") : 0.0f, 0.5f, 8.0f, 120.0f);
    de.setup  (P ("deFreq"),  2.5f, P ("deThresh"),  On ("deOn") ? P ("deAmt") : 0.0f,    0.9f, 0.5f, 50.0f);

    comp.set (P ("compThresh"), P ("compRatio"), P ("compAtk"), P ("compRel"), 6.0f);
    compMakeup = dbToGain (P ("compMakeup"));

    const float x1 = P ("mbLowX");
    const float x2 = std::max (P ("mbHighX"), x1 * 1.5f);
    lp1.setLP (sr, x1);  hp1.setHP (sr, x1);
    lp2.setLP (sr, x2);  hp2.setHP (sr, x2);
    apLo.setLP (sr, x2); apHi.setHP (sr, x2);   // allpass-align the low band
    const float mbR = P ("mbRatio");
    mb[0].set (P ("mbLowThr"),  mbR, 20.0f, 150.0f, 6.0f);
    mb[1].set (P ("mbMidThr"),  mbR, 10.0f, 100.0f, 6.0f);
    mb[2].set (P ("mbHighThr"), mbR,  3.0f,  60.0f, 6.0f);

    const float drive   = dbToGain (P ("satDrive"));
    const float satMix  = P ("satMix");
    const float satComp = 1.0f / std::sqrt (drive);

    const float glueThr = P ("glueThresh"), glueRat = P ("glueRatio");
    glue.set (glueThr, glueRat, 30.0f, P ("glueRel"), 8.0f);
    glueMakeup = dbToGain (-glueThr * (1.0f - 1.0f / glueRat) * 0.3f);

    airShelf.highShelf (sr, 11000.0f, P ("air") * 9.0f);

    const int dBufSize = delayBuf.getNumSamples();
    const int delaySamples = juce::jlimit (1, dBufSize - 1, (int) (P ("delayTime") * 0.001 * sr));
    const float delayFb = delayOn ? P ("delayFb") : 0.0f, delayMix = delayOn ? P ("delayMix") : 0.0f;

    juce::Reverb::Parameters rp;
    rp.roomSize = P ("revSize");
    rp.damping  = P ("revDamp");
    rp.wetLevel = 0.33f;
    rp.dryLevel = 0.0f;
    rp.width    = 1.0f;
    rp.freezeMode = 0.0f;
    reverb.setParameters (rp);
    const float revMix = revOn ? P ("revMix") : 0.0f;

    const float duckAmt = On ("duckOn") ? P ("duckAmt") : 0.0f, duckThr = P ("duckThr");
    const float duckAtk = std::exp (-1.0f / (float) (sr * 0.005));
    const float duckRel = std::exp (-1.0f / (float) (sr * P ("duckRel") * 0.001));

    //--------------------------------------------------------------- audio
    float* dL = delayBuf.getWritePointer (0);
    float* dR = delayBuf.getWritePointer (1);

    float peakIn = 0, peakOut = 0, maxGrC = 0, maxGrG = 0, maxGrL = 0, minGate = 1.0f;

    for (int i = 0; i < n; ++i)
    {
        const float dryL = L[i], dryR = R[i];
        const float g = inGain.getNextValue();
        float xl = L[i] * g, xr = R[i] * g;
        peakIn = std::max ({ peakIn, std::abs (xl), std::abs (xr) });

        // ---- ducking detector (internal = input signal, or external sidechain)
        const float det = scL != nullptr ? std::max (std::abs (scL[i]), std::abs (scR[i]))
                                         : std::max (std::abs (xl), std::abs (xr));
        duckEnv = det > duckEnv ? duckAtk * duckEnv + (1 - duckAtk) * det
                                : duckRel * duckEnv + (1 - duckRel) * det;
        const float duckRed  = duckAmt * juce::jlimit (0.0f, 1.0f, (gainToDb (duckEnv) - duckThr) / 12.0f);
        const float duckGain = duckRed > 0.001f ? dbToGain (-duckRed) : 1.0f;

        // ---- Plosive control: detect a burst of low-frequency energy and softly duck it.
        if (plosiveOn)
        {
            const float cutoff = juce::jlimit (70.0f, 220.0f, P ("plosiveFreq"));
            const float lpCoef = std::exp (-2.0f * juce::MathConstants<float>::pi * cutoff / (float) sr);
            plosiveLow = lpCoef * plosiveLow + (1.0f - lpCoef) * ((xl + xr) * 0.5f);
            const float absLow = std::abs (plosiveLow);
            const float atk = std::exp (-1.0f / (float) (sr * 0.002));
            const float rel = std::exp (-1.0f / (float) (sr * 0.085));
            plosiveEnv = absLow > plosiveEnv ? atk * plosiveEnv + (1.0f - atk) * absLow
                                               : rel * plosiveEnv + (1.0f - rel) * absLow;
            const float envDb = gainToDb (plosiveEnv);
            const float over = juce::jlimit (0.0f, 1.0f, (envDb - P ("plosiveThresh")) / 12.0f);
            const float reduction = P ("plosiveAmt") * over;
            const float pg = dbToGain (-reduction);
            xl *= pg; xr *= pg;
        }

        // ---- Gate -> Pitch correct -> Pre-amp
        if (gateOn)
        {
            const float gg = gate.tick (std::max (std::abs (xl), std::abs (xr)));
            xl *= gg; xr *= gg;
            minGate = std::min (minGate, gg);
        }
        pitch.process (xl, xr);
        if (preOn && pre.active)
        {
            xl = pre.process (0, xl);
            xr = pre.process (1, xr);
        }

        auto stage = [&] (Biquad& b) { xl = b.process (0, xl); xr = b.process (1, xr); };

        // ---- EQ -> Dynamic EQ -> De-esser
        if (eqOn) { stage (hpf); stage (lowShelf); stage (midPk); stage (hiShelf); }
        dyn.process (xl, xr);
        de.process (xl, xr);

        // ---- Compressor
        if (compOn)
        {
            const float gc = comp.tick (std::max (std::abs (xl), std::abs (xr))) * compMakeup;
            xl *= gc; xr *= gc;
            maxGrC = std::max (maxGrC, comp.grDb);
        }
        else comp.grDb = 0.0f;

        // ---- Multiband compressor (LR4, 3 bands)
        if (mbOn)
        {
            const float in[2] = { xl, xr };
            float lo[2], mid[2], hi[2];
            for (int ch = 0; ch < 2; ++ch)
            {
                const float low  = lp1.process (ch, in[ch]);
                const float rest = hp1.process (ch, in[ch]);
                lo[ch]  = apLo.process (ch, low) + apHi.process (ch, low);
                mid[ch] = lp2.process (ch, rest);
                hi[ch]  = hp2.process (ch, rest);
            }
            const float gl = mb[0].tick (std::max (std::abs (lo[0]),  std::abs (lo[1])));
            const float gm = mb[1].tick (std::max (std::abs (mid[0]), std::abs (mid[1])));
            const float gh = mb[2].tick (std::max (std::abs (hi[0]),  std::abs (hi[1])));
            xl = lo[0] * gl + mid[0] * gm + hi[0] * gh;
            xr = lo[1] * gl + mid[1] * gm + hi[1] * gh;
        }

        // ---- Saturation (parallel tanh)
        if (satOn && satMix > 0.001f)
        {
            xl += (std::tanh (xl * drive) * satComp - xl) * satMix;
            xr += (std::tanh (xr * drive) * satComp - xr) * satMix;
        }

        // ---- Glue compressor
        if (glueOn)
        {
            const float gc = glue.tick (std::max (std::abs (xl), std::abs (xr))) * glueMakeup;
            xl *= gc; xr *= gc;
            maxGrG = std::max (maxGrG, glue.grDb);
        }
        else glue.grDb = 0.0f;

        // ---- Air
        if (airOn) stage (airShelf);

        // ---- Delay (cross-fed ping-pong, darkened feedback)
        int rp_ = writePos - delaySamples;
        if (rp_ < 0) rp_ += dBufSize;
        const float dlyL = dL[rp_], dlyR = dR[rp_];
        dampL += 0.35f * (dlyR - dampL);
        dampR += 0.35f * (dlyL - dampR);
        dL[writePos] = xl + dampL * delayFb;
        dR[writePos] = xr + dampR * delayFb;
        if (++writePos >= dBufSize) writePos = 0;

        // ---- Reverb
        float rl = 0.0f, rr = 0.0f;
        if (revMix > 0.001f)
        {
            rl = xl; rr = xr;
            reverb.processStereo (&rl, &rr, 1);
        }

        // ---- Mix FX returns (ducked) + output
        const float wetL = (dlyL * delayMix + rl * revMix) * duckGain;
        const float wetR = (dlyR * delayMix + rr * revMix) * duckGain;

        const float og = outGain.getNextValue();
        float ol = (xl + wetL) * og * limGain, orr = (xr + wetR) * og * limGain;
        limiter.process (ol, orr);
        maxGrL = std::max (maxGrL, -gainToDb (limiter.lastGain));
        // ---- Master bypass: latency-compensated dry path, crossfaded
        {
            bypassBuf.setSample (0, bypassWp, dryL);
            bypassBuf.setSample (1, bypassWp, dryR);
            const int rd = (bypassWp + 1 >= bypassSize) ? 0 : bypassWp + 1;
            const float bm = bypassMix.getNextValue();
            if (bm > 0.0001f)
            {
                ol  = ol  * (1.0f - bm) + bypassBuf.getSample (0, rd) * bm;
                orr = orr * (1.0f - bm) + bypassBuf.getSample (1, rd) * bm;
            }
            bypassWp = rd;
        }

        loud.process (ol, mono ? 0.0f : orr);
        L[i] = ol; if (! mono) R[i] = orr;
        peakOut = std::max ({ peakOut, std::abs (ol), std::abs (orr) });
    }

    meterIn.store  (std::max (peakIn,  meterIn.load()  * 0.85f));
    meterOut.store (std::max (peakOut, meterOut.load() * 0.85f));
    grComp.store   (std::max (maxGrC,  grComp.load()   * 0.85f));
    grGlue.store   (std::max (maxGrG,  grGlue.load()   * 0.85f));
    grLim.store    (std::max (maxGrL,  grLim.load()    * 0.85f));
    grGate.store   (std::max (-gainToDb (minGate), grGate.load() * 0.85f));
    grPlosive.store (std::max (P ("plosiveAmt") * juce::jlimit (0.0f, 1.0f, (gainToDb (plosiveEnv) - P ("plosiveThresh")) / 12.0f), grPlosive.load() * 0.85f));
}

//==============================================================================
juce::AudioProcessorEditor* RoyalStripProcessor::createEditor() { return new RoyalStripEditor (*this); }

void RoyalStripProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, dest);
}

void RoyalStripProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new RoyalStripProcessor(); }
