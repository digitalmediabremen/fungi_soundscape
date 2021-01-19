
#include "SequencerGateOutput.h"


pdsp::SequencerGateOutput::SequencerGateOutput(){
    addOutput("trig", output_trig);
    resetOutputToDefault();

    meter_ticks = 10000;
    meter_value = 0.0f;
    singleTrigger = false;
    gateState = false;
    messageBuffer = nullptr;
        
}

pdsp::SequencerGateOutput::SequencerGateOutput(const SequencerGateOutput& other){
    std::cout<<"[pdsp] do not copy construct SequencerGateOutputs!\n";
    pdsp_trace();
}

int pdsp::SequencerGateOutput::meter_last_ticks() const{
    return meter_ticks.load();
}

float pdsp::SequencerGateOutput::meter_last_value() const{
    return meter_value.load();    
}

void pdsp::SequencerGateOutput::link(MessageBuffer &messageBuffer){
    this->messageBuffer = &messageBuffer;
    messageBuffer.destination = this;
    messageBuffer.connectedToGate = true;
}

void pdsp::SequencerGateOutput::unLink(){

    if(messageBuffer!=nullptr){
        messageBuffer->connectedToGate = false;
        messageBuffer->destination = nullptr;
        messageBuffer = nullptr;            
    }
    gateState = false;
    //lastGateValue = pdspGateNull;
}

void pdsp::SequencerGateOutput::setSingleTrigger(bool state){
    singleTrigger = state;
}

pdsp::Patchable& pdsp::SequencerGateOutput::out_trig(){
    return out("trig");
}

void pdsp::SequencerGateOutput::prepareUnit( int expectedBufferSize, double sampleRate ) {}

void pdsp::SequencerGateOutput::releaseResources (){ }

void pdsp::SequencerGateOutput::process (int bufferSize) noexcept {
        meter_ticks++;
        
        //if( messageBuffer!=nullptr ){         
    
        if( ( messageBuffer->empty() ) ){
                setOutputToZero(output_trig);
        }else{
                float* trigBuffer = getOutputBufferToFill( output_trig );
                ofx_Aeq_Zero(trigBuffer, bufferSize); //clean buffer
                
                int imax = messageBuffer->size();
                for( int i=0; i<imax; ++i){
                        ControlMessage &msg = messageBuffer->messages[i];
                        
                        if(msg.value <=0.0f){
                                trigBuffer[ msg.sample  * getOversampleLevel() ] = PDSP_TRIGGER_OFF;
                                gateState = false;
                        }else{
                                if(!singleTrigger || (singleTrigger && !gateState) ){
                                        //normal trigger or first trigger with legato
                                        trigBuffer[ msg.sample  * getOversampleLevel() ] =  msg.value;
                                        meter_ticks = 0;
                                        
                                }else{
                                        //legato triggers
                                        trigBuffer[ msg.sample  * getOversampleLevel() ] = - msg.value;
                                }
                                gateState = true;   
                                meter_value = msg.value;
                        }
                }
        }
                
        //}else{
        //        setOutputToZero(output_trig);
        //}

}
