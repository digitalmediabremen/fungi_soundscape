
// AllPassDelay.h
// ofxPDSP
// Nicola Pisanti, MIT License, 2016


#ifndef PDSP_DELAYS_ALLPASSDELAY_H_INCLUDED
#define PDSP_DELAYS_ALLPASSDELAY_H_INCLUDED

#include "../pdspCore.h"

namespace pdsp{

    /*!
    @brief Digital Delay with an all pass filter in the feedback path, useful for building reverbs.
    
    AllPassDelay is a delay with an all pass filter inside the feedback path. The in_feedback() input controls both the allpass filter coefficient and the feedback (coefficient = -feedback). The values from in_feedback() are used just once per buffer, so it has really no sense to patch Units running at audio rate to them. in_time() instead supports audio-rate modulation.  The max time of AllPassDelay is settable with setMaxTime() or using the AllPassDelay( float timeMS ) constructor.
    */
class AllPassDelay : public Unit {
public:

    AllPassDelay();

    AllPassDelay( float timeMs);


    /*!
    @brief sets the default delay time, optionally the feedback and returns the unit ready to be patched.
    @param[in] timeMs delay time in milliseconds 
    @param[in] feedback feedback value, if not given 0.0f
    */     
    Patchable& set( float timeMs, float feedback=0.0f);
    
    /*!
    @brief Sets "signal" as selected input and returns this Unit ready to be patched. This is the default input. This is the signal to be delayed. You can patch the output back to this input to make a feedback path.
    */   
    Patchable& in_signal();
    
    /*!
    @brief Sets "time" as selected input and returns this Unit ready to be patched. This is the delay time and obviously support audio-rate modulation. The time value is clamped to the max delay time of the delay buffer and to the minimum by the audio buffer size time.
    */  
    Patchable& in_time();
    
    /*!
    @brief Sets "feedback" as selected output and returns this Unit ready to be patched. The result of the processing of this input is used at control rate, so it has no much sense to patch Units running at audio rate into this input. This input is the feedback amount of delay.
    */      
    Patchable& in_feedback();
    
    /*!
    @brief Sets "signal" as selected output and returns this Unit ready to be patched. This is the default output. This is the delayed signal.
    */       
    Patchable& out_signal();
    
    /*!
    @brief sets maximum delay time of this delay, The old delay buffer will be deleted and a new will be created, so this is absolutely not thread safe if used in the audio callback.
    @param[in] timeMs new maximum delay time.
    */ 
    void setMaxTime(float timeMs);
    
    /*!
    @brief returns the time value at the start of the last processed buffer. This method is thread-safe.
    */ 
    float meter_time() const;


private:
    void process(int bufferSize) noexcept override;

    void prepareUnit( int expectedBufferSize, double sampleRate ) override;
    void releaseResources() override;
    void initDelayBuffer();
    
    InputNode       input;
    InputNode       in_time_ms;
    InputNode       input_damping;
    InputNode       input_feedback;
    
    OutputNode      output;

    float msToSamplesMultiplier;
    float* delayBuffer;
    float maxDelayTimeMs;
    float sampleRate;
    int maxDelayTimeSamples;
    float maxDelayTimeSamples_f;
    
    void process_once(const float* timeBuffer)noexcept;
    
    template<bool inputAR, bool timeAR>
    void process_audio(const float* inputBuffer, const float* timeBuffer, int bufferSize)noexcept;

    void updateBoundaries();
    
    int writeIndex;
    float readIndex;

    bool boundaries;

    float g;
    float outGcoeff;

    std::atomic<float> timeMeter;
};

} // pdsp namespace ned


#endif  // PDSP_DELAYS_ALLPASSDELAY_H_INCLUDED
