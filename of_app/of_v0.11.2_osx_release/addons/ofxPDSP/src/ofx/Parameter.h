
// Parameter.h
// ofxPDSP
// Nicola Pisanti, MIT License, 2016-2018

#ifndef OFXPDSP_PDSPPARAMETER_H_INCLUDED
#define OFXPDSP_PDSPPARAMETER_H_INCLUDED

#include "../DSP/pdspCore.h"
#include "../DSP/helpers/UsesSlew.h"
#include "../DSP/control/ValueControl.h"

#include "ofMain.h"

//-------------------------------------------------------------------------------------------------

/*!
@brief Utility class control the audio dsp parameter and bind them to one or more internal ofParameter

pdsp::Parameter contains some ofParameter and a pdsp::ValueControl, that is read and processed in a thread-safe manner. When one of the ofParameters is changed the pdsp::ValueControl is set. The output of this class can be patched to the audio DSP and can be optionally smoothed out. 

*/

namespace pdsp{

class Parameter : public pdsp::Patchable {
    friend class ParameterAmp;  // those friendships can be removed
    friend class ParameterGain; // when the setv method is removed from API
    
public:
    Parameter();
    Parameter(const Parameter & other);
    Parameter& operator=(const Parameter & other);
    
    ~Parameter();
    
    /*!
    @brief sets the and returns the internal ofParameter<float>, useful to set up an UI. This parameter is summed to another ofParameter<int>, that you access passing all ints as parameters
    @param[in] name this will become the name of the ofParameter
    @param[in] value default value
    @param[in] min minimum value 
    @param[in] max maximum value
    */    
    ofParameter<float>& set(const char * name, float value, float min, float max);

    /*!
    @brief sets the and returns the internal ofParameter<int>, useful to set up an UI. This parameter is summed to another ofParameter<float>, that you access passing all ints as parameters
    @param[in] name this will become the name of the ofParameter
    @param[in] value default value
    @param[in] min minimum value 
    @param[in] max maximum value
    */    
    ofParameter<int>& set(const char * name, int value, int min, int max);


    /*!
    @brief sets the value min and max boundary when operated by the ofParameter<float> in the UI and returns the parameter ready to be added to the UI
    @param[in] name this will become the name of the ofParameter
    @param[in] min minimum value 
    @param[in] max maximum value
    */  
    ofParameter<float>& set(const char * name, float min, float max);

    /*!
    @brief sets the value min and max boundary when operated by the ofParameter<int> in the UI and returns the parameter ready to be added to the UI
    @param[in] name this will become the name of the ofParameter
    @param[in] min minimum value 
    @param[in] max maximum value
    */  
    ofParameter<int>& set(const char * name, int min, int max);

    /*!
    @brief sets the and returns the internal ofParameter<float>, useful to set up an UI. This parameter is summed to another ofParameter<int>, that you access passing all ints as parameters
    @param[in] name this will become the name of the ofParameter
    @param[in] value default value
    @param[in] min minimum value 
    @param[in] max maximum value
    */    
    ofParameter<float>& set( std::string name, float value, float min, float max);

    /*!
    @brief sets the and returns the internal ofParameter<int>, useful to set up an UI. This parameter is summed to another ofParameter<float>, that you access passing all ints as parameters
    @param[in] name this will become the name of the ofParameter
    @param[in] value default value
    @param[in] min minimum value 
    @param[in] max maximum value
    */    
    ofParameter<int>& set( std::string name, int value, int min, int max);


    /*!
    @brief sets the value min and max boundary when operated by the ofParameter<float> in the UI and returns the parameter ready to be added to the UI
    @param[in] name this will become the name of the ofParameter
    @param[in] min minimum value 
    @param[in] max maximum value
    */  
    ofParameter<float>& set( std::string name, float min, float max);

    /*!
    @brief sets the value min and max boundary when operated by the ofParameter<int> in the UI and returns the parameter ready to be added to the UI
    @param[in] name this will become the name of the ofParameter
    @param[in] min minimum value 
    @param[in] max maximum value
    */  
    ofParameter<int>& set( std::string name, int min, int max);

    /*!
    @brief sets the and returns the internal ofParameter<bool>, useful to set up an UI. This parameter set the value to min when false and to max when true, 0.0f and 1.0f if not given.
    @param[in] name this will become the name of the ofParameter
    @param[in] value default value
    @param[in] min value when false, 0,0f if not given
    @param[in] max value when true, 1.0f if not given
    */    
    ofParameter<bool>& set( std::string name, bool value, float min=0.0f, float max=1.0f );
    
    /*!
    @brief sets the and returns the internal ofParameter<bool>, useful to set up an UI. This parameter set the value to min when false and to max when true, 0.0f and 1.0f if not given.
    @param[in] name this will become the name of the ofParameter
    @param[in] value default value
    @param[in] min value when false, 0,0f if not given
    @param[in] max value when true, 1.0f if not given
    */    
    ofParameter<bool>& set( const char * name, bool value, float min=0.0f, float max=1.0f  );

    /*!
    @brief returns the ofParameter ready to be added to the UI
    */  
    ofParameter<float>& getOFParameterFloat();

    /*!
    @brief returns the ofParameter ready to be added to the UI
    */  
    ofParameter<int>& getOFParameterInt();

    /*!
    @brief returns the ofParameter ready to be added to the UI
    */  
    ofParameter<bool>& getOFParameterBool();

    /*!
    @brief enables the smoothing of the setted values
    @param[in] timeMs how many milliseconds will take to reach the setted value
    */  
    void enableSmoothing(float timeMs);
    
    /*!
    @brief disable the smoothing of the setted values. smoothing is disabled by default
    */  
    void disableSmoothing();

    /*!
    @brief sets the value and updates the ofParameters, this is safe to call on the main oF thread but not from other threads.
    @param[in] value new value
    
    */   
    void set( float value );

    /*!
    @brief gets the value
    */       
    float get() const { return valueControl.get(); }

    /*!
    @brief returns the actual output value. Thread-safe.
    */ 
    float meter_output() const{ return valueControl.meter_output(); }


    ofParameter<float>& set(const char * name, double value, double min, double max);
    ofParameter<float>& set(const char * name, double min, double max);
    ofParameter<float>& set( std::string name, double value, double min, double max);
    ofParameter<float>& set( std::string name, double min, double max);


    [[deprecated("setv(float value) method deprecated, use the set(float value) method that also updates the ofParameters or use the pdsp::ValueControl class with its set() method if you don't need ofParameters")]]  
    void setv(float value){ valueControl.set(value); }


private:

    ofParameter<float>  parameter;
    ofParameter<int>    parameter_i;
    ofParameter<bool>   parameter_b;
    
    void onSet(float &newValue);
    void onSetI(int   &newValue);
    void onSetB(bool   &newValue);

    float boolmin;
    float boolmax;
    
    ValueControl valueControl;
    
    int mode;
    std::atomic<bool> bCanUpdate;
    
    static const int modeUnsetted = 0;
    static const int modeFloat = 1;
    static const int modeInt = 2;
    static const int modeCombined = 3;
    static const int modeBool = 4;
};

}

#endif //OFXPDSP_PDSPPARAMETER_H_INCLUDED
