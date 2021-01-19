// CombFilter.h
// ofxPDSP
// Nicola Pisanti, MIT License, 2016

#ifndef PDSP_MODULE_COMBFILTER_H_INCLUDED
#define PDSP_MODULE_COMBFILTER_H_INCLUDED

#include "../../DSP/pdspCore.h"
#include "../../DSP/delays/Delay.h"
#include "../../DSP/utility/PitchToFreq.h"
#include "../../DSP/utility/FreqToMs.h"


namespace pdsp{
/*!
@brief A comb filter is a delay tuned to a specific pitch frequency (mix it with the dry signal). Multichannel.
* 
*/       

class CombFilter : public Patchable {

public:
    CombFilter();
    CombFilter(const CombFilter& other);
    CombFilter& operator=(const CombFilter& other);
    ~CombFilter();

    /*!
    @brief Sets "signal" as selected input and returns this module ready to be patched. This is the default input. This is the filter input.
    */      
    Patchable& in_signal();
    
    /*!
    @brief Sets "signal" as selected output and returns this module ready to be patched. This is the default output. This is filter signal output.
    */  
    Patchable& out_signal();
        
    /*!
    @brief Sets "pitch" as selected input and returns this module ready to be patched. This is the tuning of the delay in semitones.
    */  
    Patchable& in_pitch();
    
    /*!
    @brief Sets "feedback" as selected input and returns this module ready to be patched. This is the delay feedback value, mostly it behaves like a resonance parameter.
    */     
    Patchable& in_feedback();

    /*!
    @brief Sets "damping" as selected input and returns this module ready to be patched. This is the damping of the delay feedback.
    */  
    Patchable& in_damping();

    /*!
    @brief Allocate a number of channels for processing different inputs. This is automatically called if you query for a channel outside the allocated range. You can access different channels with the ch() method.
    @param[in] size number of channels
    */        
    void channels( size_t size );    

    /*!
    @brief Uses the selected channel as input/output for the patching operation.
    @param[in] index channel index
    */  
    Patchable& ch( size_t index );
    
    
    /*!
    @brief returns the actual pitch value.This method is thread-safe.
    */  
    float meter_pitch() const;

/*!
    @cond HIDDEN_SYMBOLS
*/
    [[deprecated("operator[] deprecated, use the ch( int index ) method instead")]]    
    Patchable& operator[]( size_t index );
    
    [[deprecated("in_fb() deprecated, use in_feedback() instead")]]    
    Patchable& in_fb();
/*!
    @endcond HIDDEN_SYMBOLS
*/    
private:
    
    void patch();


    std::vector<Delay*> delays;
    PitchToFreq p2f;
    FreqToMs    f2ms;
    
    PatchNode   fbcontrol;
    PatchNode   dampcontrol;


};


}

#endif //PDSP_MODULE_COMBFILTER_H_INCLUDED
