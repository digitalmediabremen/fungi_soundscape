// Standalone WASM build of the Fungi Soundscape synth — reuses the real ofxPDSP
// DSP (compiled from of_app/.../addons/ofxPDSP/src), with pdsp::Engine's RtAudio
// output replaced by manual block processing (pdsp::Processor) for the browser.
//
// WasmPolySynth replicates PolySynth::Voice's exact patch graph from the OF app
// (PolySynth.cpp): wavetable osc -> VA filter -> amp, ADSR -> amp/filter/table,
// LFO -> table/filter, DimensionChorus, output gain. The only change is that the
// openFrameworks GUI controls (pdsp::Parameter / ofParameter) are replaced by the
// same constant values PolySynth.cpp set them to — so it's the same synth, same
// DSP library, just driven by constants instead of a GUI.
//
// Include ONLY core/module pdsp headers — never "ofxPDSP.h" or "DSP/header.h"
// (those pull the openFrameworks-coupled ofx/ + ValueControl layer).

#include "DSP/pdspCore.h"
#include "modules/header.h"                       // TableOscillator, VAFilter, LFO, DimensionChorus
#include "DSP/envelopes/ADSR.h"
#include "DSP/oscillators/wavetable/WaveTable.h"
#include "DSP/control/TriggerControl.h"

#include <vector>
#include <emscripten/emscripten.h>

class WasmPolySynth {
public:
    class Voice : public pdsp::Patchable {
    public:
        Voice() {}
        Voice(const Voice& /*other*/) {}  // pdsp units: empty copy ctor for vector

        void setup(WasmPolySynth& m, int v) {
            addModuleInput("trig", voiceTrigger);
            addModuleInput("pitch", oscillator.in_pitch());
            addModuleOutput("signal", voiceAmp);

            oscillator.setTable(m.wavetable);

            // SIGNAL PATH (verbatim from PolySynth::Voice::setup)
            oscillator >> filter >> voiceAmp >> m.chorus.ch(0);
                                    voiceAmp >> m.chorus.ch(1);

            // env/lfo -> wavetable index (was env_table_amt=0.5, table_ctrl=8 in OF;
            // we use a smaller base index since we skip the 3 file-loaded samples)
            envelope >> envToTable >> oscillator.in_table();
            0.5f     >> envToTable.in_mod();
            m.lfo_to_table >> oscillator.in_table();
            3.0f           >> oscillator.in_table();

            // gate -> envelope -> amp
            voiceGate >> voiceTrigger >> envelope >> voiceAmp.in_mod();

            // env/lfo -> filter cutoff (env_filter_amt=10, cutoff=110, reso=0.1, mode=0)
            envelope >> envToFilter >> filter.in_pitch();
            10.0f    >> envToFilter.in_mod();
            m.lfo_to_filter >> filter.in_pitch();
            110.0f          >> filter.in_pitch();
            0.1f            >> filter.in_reso();
            0.0f            >> filter.in_mode();

            // ADSR times (ms) — verbatim from PolySynth.cpp
            590.0f  >> envelope.in_attack();
            1100.0f >> envelope.in_decay();
            1.0f    >> envelope.in_sustain();
            1200.0f >> envelope.in_release();
        }

        void noteOn(float pitch, float velocity) {
            pitch >> oscillator.in_pitch();   // offline/single-threaded: safe to re-set
            voiceGate.trigger(velocity);
        }
        void noteOff() { voiceGate.off(); }

        pdsp::PatchNode        voiceTrigger;
        pdsp::TriggerControl   voiceGate;
        pdsp::TableOscillator  oscillator;
        pdsp::VAFilter         filter;
        pdsp::Amp              voiceAmp;
        pdsp::ADSR             envelope;
        pdsp::Amp              envToTable;
        pdsp::Amp              envToFilter;
    };

    void setup(int numVoices) {
        // WAVETABLE — algorithmic waveforms only (the OF app also addSample()s 3
        // WAV files via ofxAudioFile; we skip those since there's no file system).
        wavetable.setup(600, 256);
        wavetable.addSawWave(64);    // OF used pdsp::highestPartial(60); fixed count here
        wavetable.addSquareWave(48);
        wavetable.addTriangleWave(64);
        wavetable.addSineWave();
        wavetable.addAdditiveWave({1.0f, 1.0f, 1.0f, 1.0f});
        wavetable.addAdditiveWave({1.0f, 0.0f, -1.0f, 0.5f, 0.5f, 1.0f, -1.0f, 0.5f}, true);

        voices.resize(numVoices);
        for (int i = 0; i < numVoices; ++i) voices[i].setup(*this, i);

        chorus.ch(0) >> gainL;  0.25f >> gainL.in_mod();
        chorus.ch(1) >> gainR;  0.25f >> gainR.in_mod();

        // LFO -> switch -> table/filter modulation (lfo_wave=1, speed=0.009)
        lfo_switch.resize(5);
        lfo.out_triangle()        >> lfo_switch.input(0);
        lfo.out_saw()             >> lfo_switch.input(1);
        lfo.out_square()          >> lfo_switch.input(2);
        lfo.out_sine()            >> lfo_switch.input(3);
        lfo.out_sample_and_hold() >> lfo_switch.input(4);
        1.0f   >> lfo_switch.in_select();
        0.009f >> lfo.in_freq();
        lfo_switch >> lfo_to_table;
        lfo_switch >> lfo_to_filter;
        0.5f  >> lfo_to_table.in_mod();
        30.0f >> lfo_to_filter.in_mod();

        // Chorus (speed=0.25, depth=5)
        0.25f >> chorus.in_speed();
        5.0f  >> chorus.in_depth();
    }

    pdsp::Patchable& ch(int index) { return (index % 2 == 0) ? (pdsp::Patchable&)gainL
                                                             : (pdsp::Patchable&)gainR; }

    std::vector<Voice>     voices;
    pdsp::WaveTable        wavetable;
    pdsp::LFO              lfo;
    pdsp::Amp              lfo_to_table;
    pdsp::Amp              lfo_to_filter;
    pdsp::Switch           lfo_switch;
    pdsp::DimensionChorus  chorus;
    pdsp::Amp              gainL;
    pdsp::Amp              gainR;
};

static pdsp::Processor processor;
static WasmPolySynth   synth;
static int             nVoices = 6;
static int             rrVoice = 0; // round-robin voice allocation
static bool            ready = false;

extern "C" {

EMSCRIPTEN_KEEPALIVE
void synth_init(int sampleRate, int bufferSize, int numVoices) {
    if (numVoices > 0) nVoices = numVoices;
    processor.resize(2);

    synth.setup(nVoices);
    synth.ch(0) >> processor.channels[0];
    synth.ch(1) >> processor.channels[1];

    pdsp::prepareAllToPlay(bufferSize, (double)sampleRate);
    ready = true;
}

// Trigger a note. voice < 0 => round-robin allocation. pitch is pdsp semitones
// (60 = C3). Call from JS between process() blocks (offline render is single-threaded).
EMSCRIPTEN_KEEPALIVE
void synth_note_on(int voice, float pitch, float velocity) {
    if (!ready || nVoices <= 0) return;
    int v = (voice < 0) ? (rrVoice++ % nVoices) : (voice % nVoices);
    synth.voices[v].noteOn(pitch, velocity);
}

EMSCRIPTEN_KEEPALIVE
void synth_note_off(int voice) {
    if (!ready || voice < 0 || voice >= nVoices) return;
    synth.voices[voice].noteOff();
}

// Fill an interleaved stereo buffer [L,R,L,R,...] with nFrames frames.
EMSCRIPTEN_KEEPALIVE
void synth_process(float* out, int nFrames) {
    if (!ready) return;
    processor.processAndCopyInterleaved(out, 2, nFrames);
}

} // extern "C"
