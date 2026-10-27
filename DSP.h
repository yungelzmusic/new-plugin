#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <algorithm>
#include <cmath>
#include <array>
#include <atomic>
#include <cstdint>

namespace royal
{
inline float dbToGain (float db) { return std::pow (10.0f, db * 0.05f); }
inline float gainToDb (float g)  { return 20.0f * std::log10 (std::max (g, 1.0e-9f)); }

//==============================================================================
// RBJ biquad, stereo state, no allocations -> safe to retune from the audio thread
struct Biquad
{
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0;
    float z1[2] { 0, 0 }, z2[2] { 0, 0 };

    void reset() { z1[0] = z1[1] = z2[0] = z2[1] = 0.0f; }

    float process (int ch, float x) noexcept
    {
        const float y = b0 * x + z1[ch];
        z1[ch] = b1 * x - a1 * y + z2[ch];
        z2[ch] = b2 * x - a2 * y;
        return y;
    }

    void set (float B0, float B1, float B2, float A0, float A1, float A2)
    {
        const float r = 1.0f / A0;
        b0 = B0 * r; b1 = B1 * r; b2 = B2 * r; a1 = A1 * r; a2 = A2 * r;
    }

    static float w0 (double sr, float f)
    {
        f = juce::jlimit (10.0f, (float) sr * 0.45f, f);
        return 6.2831853f * f / (float) sr;
    }

    void lowpass (double sr, float f, float q)
    {
        const float w = w0 (sr, f), c = std::cos (w), al = std::sin (w) / (2 * q);
        set ((1 - c) * 0.5f, 1 - c, (1 - c) * 0.5f, 1 + al, -2 * c, 1 - al);
    }
    void highpass (double sr, float f, float q)
    {
        const float w = w0 (sr, f), c = std::cos (w), al = std::sin (w) / (2 * q);
        set ((1 + c) * 0.5f, -(1 + c), (1 + c) * 0.5f, 1 + al, -2 * c, 1 - al);
    }
    void bandpass (double sr, float f, float q)
    {
        const float w = w0 (sr, f), c = std::cos (w), al = std::sin (w) / (2 * q);
        set (al, 0, -al, 1 + al, -2 * c, 1 - al);
    }
    void peak (double sr, float f, float q, float gDb)
    {
        const float w = w0 (sr, f), c = std::cos (w), al = std::sin (w) / (2 * q);
        const float A = std::pow (10.0f, gDb / 40.0f);
        set (1 + al * A, -2 * c, 1 - al * A, 1 + al / A, -2 * c, 1 - al / A);
    }
    void lowShelf (double sr, float f, float gDb)
    {
        const float w = w0 (sr, f), c = std::cos (w), s = std::sin (w);
        const float A = std::pow (10.0f, gDb / 40.0f);
        const float t = 2.0f * std::sqrt (A) * (s * 0.70710678f);
        set (A * ((A + 1) - (A - 1) * c + t),
             2 * A * ((A - 1) - (A + 1) * c),
             A * ((A + 1) - (A - 1) * c - t),
             (A + 1) + (A - 1) * c + t,
             -2 * ((A - 1) + (A + 1) * c),
             (A + 1) + (A - 1) * c - t);
    }
    void highShelf (double sr, float f, float gDb)
    {
        const float w = w0 (sr, f), c = std::cos (w), s = std::sin (w);
        const float A = std::pow (10.0f, gDb / 40.0f);
        const float t = 2.0f * std::sqrt (A) * (s * 0.70710678f);
        set (A * ((A + 1) + (A - 1) * c + t),
             -2 * A * ((A - 1) + (A + 1) * c),
             A * ((A + 1) + (A - 1) * c - t),
             (A + 1) - (A - 1) * c + t,
             2 * ((A - 1) - (A + 1) * c),
             (A + 1) - (A - 1) * c - t);
    }
};

//==============================================================================
// Linkwitz-Riley 4th order = two cascaded Butterworth biquads
struct LR4
{
    Biquad a, b;
    void setLP (double sr, float f) { a.lowpass  (sr, f, 0.70710678f); b.lowpass  (sr, f, 0.70710678f); }
    void setHP (double sr, float f) { a.highpass (sr, f, 0.70710678f); b.highpass (sr, f, 0.70710678f); }
    float process (int ch, float x) noexcept { return b.process (ch, a.process (ch, x)); }
    void reset() { a.reset(); b.reset(); }
};

//==============================================================================
// Feed-forward compressor with soft knee. Returns a linear gain, tracks grDb (positive)
struct Comp
{
    double sr = 44100.0;
    float thr = 0, ratio = 1, knee = 6, atkC = 0, relC = 0, grDb = 0;

    void prepare (double s) { sr = s; grDb = 0; }

    void set (float t, float r, float atkMs, float relMs, float k)
    {
        thr = t; ratio = std::max (r, 1.0f); knee = k;
        atkC = std::exp (-1.0f / (float) (sr * atkMs * 0.001));
        relC = std::exp (-1.0f / (float) (sr * relMs * 0.001));
    }

    float tick (float det) noexcept
    {
        const float over = gainToDb (det) - thr;
        const float k = std::max (knee, 0.01f);
        float target = 0.0f;
        if (2.0f * over > k)            target = (1.0f - 1.0f / ratio) * over;
        else if (2.0f * over > -k)      { const float x = over + k * 0.5f; target = (1.0f - 1.0f / ratio) * x * x / (2.0f * k); }
        const float c = target > grDb ? atkC : relC;
        grDb = target + c * (grDb - target);
        return dbToGain (-grDb);
    }
};

//==============================================================================
// Dynamic bell: a band-pass detector drives a peaking filter that cuts when the band gets loud.
// Used for both the Dynamic EQ and the De-esser.
struct DynBand
{
    double sr = 44100.0;
    Biquad det, filt;
    float freq = -1, q = 1, thresh = -30, maxCut = 0, factor = 0.5f, atkC = 0, relC = 0, env = 0, cut = 0;
    int cnt = 0;

    void prepare (double s) { sr = s; det.reset(); filt.reset(); env = 0; cut = 0; cnt = 0; freq = -1; }

    void setup (float f, float qq, float th, float mc, float fac, float atkMs, float relMs)
    {
        if (f != freq || qq != q) { det.bandpass (sr, f, qq); freq = f; q = qq; }
        thresh = th; maxCut = mc; factor = fac;
        atkC = std::exp (-1.0f / (float) (sr * atkMs * 0.001));
        relC = std::exp (-1.0f / (float) (sr * relMs * 0.001));
    }

    void process (float& l, float& r) noexcept
    {
        if (maxCut < 0.01f) { cut = 0; env = 0; return; }

        const float d = std::max (std::abs (det.process (0, l)), std::abs (det.process (1, r)));
        env = d > env ? atkC * env + (1 - atkC) * d : relC * env + (1 - relC) * d;

        const float over = gainToDb (env) - thresh;
        const float target = over > 0 ? std::min (maxCut, over * factor) : 0.0f;
        cut += 0.02f * (target - cut);

        if (++cnt >= 16) { cnt = 0; filt.peak (sr, freq, q, -cut); }
        l = filt.process (0, l);
        r = filt.process (1, r);
    }
};
//==============================================================================
// Noise gate: instant-attack envelope, hysteresis + hold, smoothed gain
struct Gate
{
    double sr = 44100.0;
    float thrDb = -80, closeDb = -84, floorGain = 0.01f, atkC = 0, relC = 0, envC = 0, env = 0, g = 1;
    int hold = 0, holdCnt = 0;
    bool open = true;

    void set (double s, float thr, float rangeDb, float atkMs, float holdMs, float relMs)
    {
        sr = s; thrDb = thr; closeDb = thr - 4.0f;
        floorGain = dbToGain (-rangeDb);
        atkC = std::exp (-1.0f / (float) (sr * atkMs * 0.001));
        relC = std::exp (-1.0f / (float) (sr * relMs * 0.001));
        envC = std::exp (-1.0f / (float) (sr * 0.005));
        hold = (int) (sr * holdMs * 0.001);
    }

    float tick (float det) noexcept
    {
        env = det > env ? det : envC * env + (1.0f - envC) * det;
        const float db = gainToDb (env);
        if (db > thrDb)            { open = true; holdCnt = hold; }
        else if (open && db < closeDb) { if (holdCnt > 0) --holdCnt; else open = false; }

        const float target = open ? 1.0f : floorGain;
        const float c = target > g ? atkC : relC;
        g = target + c * (g - target);
        return g;
    }
};

//==============================================================================
// Pre-amp: asymmetric tanh stage (even + odd harmonics), DC blocker, tilt tone control
struct PreAmp
{
    Biquad dc, toneHi, toneLo;
    float drive = 1, bias = 0, biasOut = 0, comp = 1;
    bool active = false;

    void set (double sr, float driveDb, float warmth, float toneDb)
    {
        drive = dbToGain (driveDb);
        bias = warmth * 0.35f;
        biasOut = std::tanh (bias);
        comp = 1.0f / std::sqrt (drive);
        active = driveDb > 0.05f || warmth > 0.001f || std::abs (toneDb) > 0.05f;
        dc.highpass (sr, 15.0f, 0.7071f);
        toneHi.highShelf (sr, 3000.0f, toneDb);
        toneLo.lowShelf (sr, 200.0f, -toneDb);
    }

    float process (int ch, float x) noexcept
    {
        float y = (std::tanh (x * drive + bias) - biasOut) * comp;
        y = dc.process (ch, y);
        y = toneLo.process (ch, y);
        return toneHi.process (ch, y);
    }
};

//==============================================================================
// Lookahead brick-wall limiter (~1.5 ms). Not true-peak: inter-sample peaks can exceed the ceiling slightly.
struct Limiter
{
    int N = 64, w = 0;
    std::vector<float> dl[2], need, rg;
    double sum = 0;
    float relG = 1, relC = 0, ceilLin = 1, lastGain = 1;

    void prepare (double sr)
    {
        N = juce::jlimit (2, 500, (int) (sr * 0.0015));
        dl[0].assign ((size_t) N, 0.0f);
        dl[1].assign ((size_t) N, 0.0f);
        need.assign ((size_t) N, 1.0f);
        rg.assign ((size_t) N, 1.0f);
        sum = (double) N; relG = 1.0f; w = 0; lastGain = 1.0f;
    }
    int latency() const { return N - 1; }

    void set (double sr, float ceilDb, float relMs)
    {
        ceilLin = dbToGain (ceilDb);
        relC = std::exp (-1.0f / (float) (sr * relMs * 0.001));
    }

    void process (float& l, float& r) noexcept
    {
        const float peak = std::max (std::abs (l), std::abs (r));
        const float nd = peak > ceilLin ? ceilLin / peak : 1.0f;
        dl[0][(size_t) w] = l; dl[1][(size_t) w] = r; need[(size_t) w] = nd;

        float m = 1.0f;
        for (int i = 0; i < N; ++i) m = std::min (m, need[(size_t) i]);

        relG = m < relG ? m : relC * relG + (1.0f - relC) * m;
        sum += (double) relG - (double) rg[(size_t) w];
        rg[(size_t) w] = relG;
        lastGain = (float) (sum / (double) N);

        const int ro = (w + 1 >= N) ? 0 : w + 1;
        l = juce::jlimit (-ceilLin, ceilLin, dl[0][(size_t) ro] * lastGain);
        r = juce::jlimit (-ceilLin, ceilLin, dl[1][(size_t) ro] * lastGain);
        w = ro;
    }
};

//==============================================================================
// Basic auto-pitch: YIN detector (decimated) -> scale quantiser -> retune-speed smoothing
// -> dual-tap crossfading delay-line pitch shifter. Good for small corrections, not a
// replacement for Auto-Tune/Melodyne quality. Fixed latency = Lg/2 samples.
struct PitchCorrector
{
    double sr = 44100.0, dsr = 12000.0;
    int Lg = 1322, bufSize = 0, wp = 0;
    std::vector<float> line[2];
    float p1 = 0.0f, ratioCur = 1.0f, ratioTgt = 1.0f, ratioC = 0.0f, corr = 0.0f;

    int dsFactor = 1, lagMax = 100, tauMin = 12, hop = 220, hopCnt = 0, decCnt = 0, decWp = 0, decSize = 0;
    float decAcc = 0.0f;
    std::vector<float> dec, tmp, dif, cm;

    int key = 0, scale = 0;
    float speedMs = 20.0f, amount = 0.0f;

    int latency() const { return Lg / 2; }

    void prepare (double s)
    {
        sr = s;
        Lg = ((int) (sr * 0.03) / 2) * 2;
        bufSize = Lg + 8;
        line[0].assign ((size_t) bufSize, 0.0f);
        line[1].assign ((size_t) bufSize, 0.0f);
        wp = 0; p1 = 0.0f; ratioCur = ratioTgt = 1.0f; corr = 0.0f;
        ratioC = 1.0f - std::exp (-1.0f / (float) (sr * 0.003));

        dsFactor = std::max (1, (int) std::lround (sr / 12000.0));
        dsr = sr / (double) dsFactor;
        lagMax = (int) (dsr / 70.0) + 1;
        tauMin = std::max (2, (int) (dsr / 800.0));
        decSize = 2 * lagMax + 4;
        dec.assign ((size_t) decSize, 0.0f);
        tmp.assign ((size_t) decSize, 0.0f);
        dif.assign ((size_t) lagMax + 2, 0.0f);
        cm.assign ((size_t) lagMax + 2, 0.0f);
        hop = std::max (32, (int) (sr * 0.005));
        hopCnt = decCnt = decWp = 0; decAcc = 0.0f;
    }

    void setup (int k, int sc, float speed, float amt) { key = k; scale = sc; speedMs = speed; amount = amt; }

    static bool allowed (int note, int k, int sc)
    {
        static const bool maj[12] = { 1,0,1,0,1,1,0,1,0,1,0,1 };
        static const bool mnr[12] = { 1,0,1,1,0,1,0,1,1,0,1,0 };
        const int pc = ((note - k) % 12 + 12) % 12;
        return sc == 0 || (sc == 1 ? maj[pc] : mnr[pc]);
    }

    float nearestNote (float midi) const
    {
        const int base = (int) std::lround (midi);
        float best = (float) base, bestD = 1.0e9f;
        for (int off = 0; off <= 6; ++off)
            for (int sgn : { -1, 1 })
            {
                const int cand = base + sgn * off;
                if (! allowed (cand, key, scale)) continue;
                const float d = std::abs ((float) cand - midi);
                if (d < bestD) { bestD = d; best = (float) cand; }
            }
        return best;
    }

    void analyse()
    {
        const float alpha = speedMs < 0.5f ? 1.0f
                          : 1.0f - std::exp (-(float) hop / (float) (sr * speedMs * 0.001));

        for (int i = 0; i < decSize; ++i) tmp[(size_t) i] = dec[(size_t) ((decWp + i) % decSize)];
        const float* x = tmp.data() + (decSize - 2 * lagMax);
        const int Wd = lagMax, tauMax = lagMax - 1;

        double e = 0.0;
        for (int j = 0; j < 2 * lagMax; ++j) e += (double) x[j] * x[j];
        const bool loud = std::sqrt (e / (2.0 * lagMax)) > 0.004;

        float f0 = 0.0f;
        if (loud)
        {
            double run = 0.0;
            cm[0] = 1.0f;
            for (int t = 1; t <= tauMax; ++t)
            {
                double d = 0.0;
                for (int j = 0; j < Wd; ++j) { const float df = x[j] - x[j + t]; d += (double) df * df; }
                dif[(size_t) t] = (float) d;
                run += d;
                cm[(size_t) t] = run > 1.0e-12 ? (float) (d * t / run) : 1.0f;
            }
            int tau = -1;
            for (int t = tauMin; t <= tauMax; ++t)
                if (cm[(size_t) t] < 0.15f)
                {
                    while (t + 1 <= tauMax && cm[(size_t) t + 1] < cm[(size_t) t]) ++t;
                    tau = t; break;
                }
            if (tau > 1 && tau < tauMax)
            {
                const float s0 = cm[(size_t) tau - 1], s1 = cm[(size_t) tau], s2 = cm[(size_t) tau + 1];
                const float den = 2.0f * (2.0f * s1 - s2 - s0);
                const float tf = std::abs (den) > 1.0e-9f ? (float) tau + (s2 - s0) / den : (float) tau;
                f0 = (float) dsr / tf;
            }
            else if (tau > 0) f0 = (float) dsr / (float) tau;
        }

        if (f0 > 70.0f && f0 < 1000.0f)
        {
            const float midi = 69.0f + 12.0f * std::log2 (f0 / 440.0f);
            const float desired = (nearestNote (midi) - midi) * amount;
            corr += alpha * (desired - corr);
        }
        else
            corr *= std::exp (-(float) hop / (float) (sr * 0.08));

        ratioTgt = std::pow (2.0f, corr / 12.0f);
    }

    float readTap (int ch, float d) const
    {
        float pos = (float) wp - d;
        if (pos < 0.0f) pos += (float) bufSize;
        const int i0 = (int) pos;
        const float fr = pos - (float) i0;
        const int i1 = (i0 + 1 >= bufSize) ? 0 : i0 + 1;
        return line[ch][(size_t) i0] * (1.0f - fr) + line[ch][(size_t) i1] * fr;
    }

    void process (float& l, float& r) noexcept
    {
        line[0][(size_t) wp] = l; line[1][(size_t) wp] = r;

        decAcc += 0.5f * (l + r);
        if (++decCnt >= dsFactor)
        {
            dec[(size_t) decWp] = decAcc / (float) dsFactor;
            decAcc = 0.0f; decCnt = 0;
            if (++decWp >= decSize) decWp = 0;
        }
        if (++hopCnt >= hop)
        {
            hopCnt = 0;
            if (amount > 0.001f) analyse();
            else { corr = 0.0f; ratioTgt = 1.0f; }
        }

        ratioCur += ratioC * (ratioTgt - ratioCur);
        p1 += (1.0f - ratioCur) / (float) Lg;
        if (p1 >= 1.0f) p1 -= 1.0f; else if (p1 < 0.0f) p1 += 1.0f;
        float p2 = p1 + 0.5f; if (p2 >= 1.0f) p2 -= 1.0f;

        const float g1 = 0.5f - 0.5f * std::cos (6.2831853f * p1), g2 = 1.0f - g1;
        const float d1 = p1 * (float) Lg, d2 = p2 * (float) Lg, dd = (float) (Lg / 2);
        const float wet = juce::jlimit (0.0f, 1.0f, std::abs (corr) * 10.0f);

        const float sl = g1 * readTap (0, d1) + g2 * readTap (0, d2);
        const float sr_ = g1 * readTap (1, d1) + g2 * readTap (1, d2);
        const float dl = readTap (0, dd), dr = readTap (1, dd);

        l = dl * (1.0f - wet) + sl * wet;
        r = dr * (1.0f - wet) + sr_ * wet;
        if (++wp >= bufSize) wp = 0;
    }
};

//==============================================================================
// ITU-R BS.1770 loudness: K-weighting, 100 ms blocks, momentary (400 ms), short-term (3 s),
// integrated (absolute -70 LUFS + relative -10 LU gating, kept as a 0.1 LU histogram).
struct Loudness
{
    static constexpr int kBins = 800;   // 0.1 LU bins from -70 to +10 LUFS

    struct BQd
    {
        double b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1[2] { 0, 0 }, z2[2] { 0, 0 };
        double process (int ch, double x) noexcept
        {
            const double y = b0 * x + z1[ch];
            z1[ch] = b1 * x - a1 * y + z2[ch];
            z2[ch] = b2 * x - a2 * y;
            return y;
        }
    };

    BQd s1, s2;
    int blockLen = 4800, cnt = 0, nb = 0, wi = 0;
    double sumSq = 0.0, blocks[30] {};
    float blockPeak = 0.0f;
    std::atomic<float> momentary { -120.0f }, shortTerm { -120.0f }, peakHold { -120.0f };
    std::array<std::atomic<uint32_t>, (size_t) kBins> hist;

    Loudness() { for (auto& h : hist) h.store (0); }

    void prepare (double sr)
    {
        const double pi = 3.14159265358979323846;
        blockLen = std::max (1, (int) (sr * 0.1));
        {
            const double f0 = 1681.974450955533, G = 3.999843853973347, Q = 0.7071752369554196;
            const double K = std::tan (pi * f0 / sr), Vh = std::pow (10.0, G / 20.0), Vb = std::pow (Vh, 0.4996667741545416);
            const double a0 = 1.0 + K / Q + K * K;
            s1 = {};
            s1.b0 = (Vh + Vb * K / Q + K * K) / a0;
            s1.b1 = 2.0 * (K * K - Vh) / a0;
            s1.b2 = (Vh - Vb * K / Q + K * K) / a0;
            s1.a1 = 2.0 * (K * K - 1.0) / a0;
            s1.a2 = (1.0 - K / Q + K * K) / a0;
        }
        {
            const double f0 = 38.13547087602444, Q = 0.5003270373238773;
            const double K = std::tan (pi * f0 / sr), d = 1.0 + K / Q + K * K;
            s2 = {};
            s2.b0 = 1.0; s2.b1 = -2.0; s2.b2 = 1.0;
            s2.a1 = 2.0 * (K * K - 1.0) / d;
            s2.a2 = (1.0 - K / Q + K * K) / d;
        }
        reset();
    }

    void reset()
    {
        cnt = nb = wi = 0; sumSq = 0.0; blockPeak = 0.0f;
        for (auto& b : blocks) b = 0.0;
        for (auto& h : hist) h.store (0, std::memory_order_relaxed);
        momentary.store (-120.0f); shortTerm.store (-120.0f); peakHold.store (-120.0f);
    }

    static float toLufs (double e) { return e > 1.0e-12 ? (float) (-0.691 + 10.0 * std::log10 (e)) : -120.0f; }

    double avg (int n) const
    {
        double e = 0.0;
        for (int i = 0; i < n; ++i) e += blocks[((wi - 1 - i) % 30 + 30) % 30];
        return e / (double) std::max (1, n);
    }

    void process (float l, float r) noexcept
    {
        blockPeak = std::max ({ blockPeak, std::abs (l), std::abs (r) });
        const double yl = s2.process (0, s1.process (0, (double) l));
        const double yr = s2.process (1, s1.process (1, (double) r));
        sumSq += yl * yl + yr * yr;

        if (++cnt >= blockLen)
        {
            cnt = 0;
            blocks[wi] = sumSq / (double) blockLen;
            sumSq = 0.0;
            wi = (wi + 1) % 30;
            nb = std::min (nb + 1, 30);

            const float M = toLufs (avg (std::min (nb, 4)));
            momentary.store (M);
            shortTerm.store (toLufs (avg (nb)));
            if (nb >= 4 && M > -70.0f)
                hist[(size_t) juce::jlimit (0, kBins - 1, (int) ((M + 70.0f) * 10.0f))].fetch_add (1, std::memory_order_relaxed);

            const float pk = blockPeak > 0.0f ? 20.0f * std::log10 (blockPeak) : -120.0f;
            if (pk > peakHold.load()) peakHold.store (pk);
            blockPeak = 0.0f;
        }
    }

    // safe to call from the UI thread
    float integrated() const
    {
        double eAll = 0.0; uint64_t nAll = 0;
        for (int i = 0; i < kBins; ++i)
        {
            const uint32_t c = hist[(size_t) i].load (std::memory_order_relaxed);
            if (! c) continue;
            const double L = -70.0 + ((double) i + 0.5) * 0.1;
            eAll += (double) c * std::pow (10.0, (L + 0.691) / 10.0);
            nAll += c;
        }
        if (! nAll) return -120.0f;
        const double gate = 10.0 * std::log10 (eAll / (double) nAll) - 0.691 - 10.0;
        double e2 = 0.0; uint64_t n2 = 0;
        for (int i = 0; i < kBins; ++i)
        {
            const uint32_t c = hist[(size_t) i].load (std::memory_order_relaxed);
            if (! c) continue;
            const double L = -70.0 + ((double) i + 0.5) * 0.1;
            if (L < gate) continue;
            e2 += (double) c * std::pow (10.0, (L + 0.691) / 10.0);
            n2 += c;
        }
        return n2 ? toLufs (e2 / (double) n2) : -120.0f;
    }
};
} // namespace royal
