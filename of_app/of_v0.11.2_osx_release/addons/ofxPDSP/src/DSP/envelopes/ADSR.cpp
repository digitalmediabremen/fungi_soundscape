

#include "ADSR.h"


pdsp::ADSR::ADSR(){
        
        stageSwitch = off;
        
        addInput("trig", input_trig);
        addOutput("signal", output);
        addOutput("bias", output_bias);
        addInput("attack", input_attack);
        addInput("decay", input_decay);
        addInput("sustain", input_sustain);
        addInput("release", input_release);
        addInput("velocity", input_velocity);
        updateOutputNodes();
        
        attackTCO                       = 0.99999;    //digital 
        decayTCO = riseTCO = releaseTCO = exp(-4.95); //digital 
        
        dBtrig = false;
        trigcount = 0;
        
        sustainLevel = 0.0f;
        decayNextStageLevel = 0.0f;
        riseNextStageLevel = 0.0f;

        input_attack.setDefaultValue(0.0f);
        input_decay.setDefaultValue(150.0f);
        input_sustain.setDefaultValue(0.5f);
        input_release.setDefaultValue(150.0f);
        input_velocity.setDefaultValue(1.0f);

        if(dynamicConstruction){
                prepareToPlay(globalBufferSize, globalSampleRate);
        }
}

pdsp::Patchable& pdsp::ADSR::set(float attackTimeMs, float decayTimeMs, float sustainLevel, float releaseTimeMs, float velocity ){
        input_attack.setDefaultValue( attackTimeMs );
        input_decay.setDefaultValue(decayTimeMs);
        input_sustain.setDefaultValue(sustainLevel);
        input_release.setDefaultValue(releaseTimeMs);
        input_velocity.setDefaultValue( velocity );
        
        return *this;
}


void pdsp::ADSR::setAttackCurve(float hardness){
    if(hardness < 0.0f) hardness = 0.0f;
    if(hardness > 1.0f) hardness = 1.0f;
    
    float attackTCO = interpolate_linear(EnvelopeStage::digitalAttackTCO, EnvelopeStage::analogAttackTCO, hardness);
    setAttackTCO(attackTCO);
}

void pdsp::ADSR::setReleaseCurve(float hardness){
    if(hardness < 0.0f) hardness = 0.0f;
    if(hardness > 1.0f) hardness = 1.0f;
    
    float decayTCO  = interpolate_linear(EnvelopeStage::digitalDecayTCO, EnvelopeStage::analogDecayTCO, hardness);
    setDecayTCO(decayTCO);
    setRiseTCO(decayTCO);
    setReleaseTCO(decayTCO);
}


void pdsp::ADSR::setCurve(float hardness){
    setAttackCurve(hardness);
    setReleaseCurve(hardness);
}

void pdsp::ADSR::enableDBTriggering(float dBmin, float dBmax){
    this->dBmin = dBmin;
    this->dBmax = dBmax;
    dBtrig = true;
}

void pdsp::ADSR::disableDBTriggering(){
    dBtrig = false;
}

float pdsp::ADSR::meter_output() const{
    return meter.load();
}

int pdsp::ADSR::meter_triggers() const{
    return trigcount.load();
}

pdsp::Patchable& pdsp::ADSR::in_trig(){
    return in("trig");
}
    
pdsp::Patchable& pdsp::ADSR::in_attack(){
    return in("attack");
}

pdsp::Patchable& pdsp::ADSR::in_decay(){
    return in("decay");
}

pdsp::Patchable& pdsp::ADSR::in_sustain(){
    return in("sustain");
}

pdsp::Patchable& pdsp::ADSR::in_release(){
    return in("release");
}

pdsp::Patchable& pdsp::ADSR::in_velocity(){
    return in("velocity");
}

pdsp::Patchable& pdsp::ADSR::out_signal(){
    return out("signal");
}

pdsp::Patchable& pdsp::ADSR::out_bias(){
    return out("bias");
}

void pdsp::ADSR::prepareUnit( int expectedBufferSize, double sampleRate){
        setEnvelopeSampleRate( sampleRate );
        stageSwitch = off;
        envelopeOutput = 0.0;

        calculateAttackTime();
        calculateDecayTime();
        calculateReleaseTime();
        calculateRiseTime();
}


void pdsp::ADSR::releaseResources(){}


void pdsp::ADSR::process(int bufferSize) noexcept{
        
        int trigBufferState;
        const float* trigBuffer = processInput(input_trig, trigBufferState);

        switch(trigBufferState){
            case Unchanged: case Changed:
                switch(stageSwitch){
                case off:
                    setOutputToZero(output);
                    meter.store(0.0f);
                    break;
                case sustainStage:
                    setControlRateOutput(output, sustainLevel);
                    meter.store(sustainLevel);
                    break;
                
                default:
                    process_run(bufferSize);
                    break;
                }
                break;                
           
            case AudioRate:
                process_T(trigBuffer, bufferSize);
                break;
            
            default: break;
        }   
}

void pdsp::ADSR::onRetrigger(float triggerValue, int n) {
    
        /* 
        // LEGACY CODE, still testing the dB function
        if(triggerValue == pdspTriggerOff){
                stageSwitch = releaseStage;

        }else if( triggerValue > 0.0f ){
                triggerValue = (triggerValue > 1.0f) ? 1.0f : triggerValue;
                float veloCtrl = processAndGetSingleValue(input_velocity, n);
                this->intensity = (triggerValue * veloCtrl)  + (1.0f-veloCtrl); 
                stageSwitch = attackStage;

        }else if( triggerValue < 0.0f ){ //legato triggers
                triggerValue = (triggerValue < -1.0f) ? -1.0f : triggerValue;
                float newIntensity = -triggerValue;
                if(stageSwitch!=attackStage){
                        if(newIntensity <= this->intensity){
                                stageSwitch = decayStage;
                        }else{
                                stageSwitch = riseStage;
                        }   
                }
                float veloCtrl = processAndGetSingleValue(input_velocity, n);
                this->intensity = (newIntensity * veloCtrl)  + (1.0f-veloCtrl); 
        }
        */
        
        if(triggerValue == PDSP_TRIGGER_OFF){
                stageSwitch = releaseStage;
        }else if( triggerValue > 0.0f ){
                stageSwitch = attackStage;
                trigcount++;
        }else if( triggerValue < 0.0f ){ //legato triggers
                triggerValue = -triggerValue;
                if(stageSwitch!=attackStage){
                        if(triggerValue <= this->envelopeOutput){
                                stageSwitch = decayStage;
                        }else{
                                stageSwitch = riseStage;
                        }   
                }
        }
        
        triggerValue = (triggerValue > 1.0f) ? 1.0f : triggerValue;

        if(dBtrig){
            float dB = dBmin*(1.0f - triggerValue) + dBmax*triggerValue;
            this->intensity = pow(10.0f, dB*0.05f);
        }else{
            float veloCtrl = processAndGetSingleValue(input_velocity, n);            
            this->intensity = (triggerValue * veloCtrl)  + (1.0f-veloCtrl); 
        }

        setSustainLevel(processAndGetSingleValue(input_sustain, n));

        float riseT;
        float riseNextLevel;
        float decayT = processAndGetSingleValue(input_decay, n);
        float attackT = processAndGetSingleValue(input_attack, n);
        if(decayT < attackT){
            riseT = decayT;
        }else{
            riseT = attackT;
        }
        if( this->intensity > envelopeOutput){
            riseNextLevel = this->intensity;
        }else{
            riseNextLevel = sustainLevel;
        }


        setAttackTime( attackT );
        setDecayTime( decayT, sustainLevel);
        setRiseTime( riseT, riseNextLevel);
        setReleaseTime(processAndGetSingleValue(input_release, n));
}
        

void pdsp::ADSR::process_run(int bufferSize){

    float* outputBuffer = getOutputBufferToFill(output);
    
    for (int n = 0; n < bufferSize; ++n){
        doEnvelope();
        outputBuffer[n] = envelopeOutput;
    }
    
    meter.store(outputBuffer[0]); 
}


void pdsp::ADSR::process_T( const float* &trigBuffer, const int &bufferSize){
        
    float* outputBuffer = getOutputBufferToFill(output);

    for (int n = 0; n < bufferSize; ++n){
        float t = trigBuffer[n];
        if ( notTrigger( t ) ){
            //likely
        }else{ 
            onRetrigger( t, n ); 
        };

        doEnvelope();
        outputBuffer[n] = envelopeOutput;
    }
    
    meter.store(outputBuffer[0]); 
}

