
// SequencerBridge.h
// ofxPDSP
// Nicola Pisanti, MIT License, 2016

#ifndef PDSP_CONTROL_SEQUENCER_H_INCLUDED
#define PDSP_CONTROL_SEQUENCER_H_INCLUDED

#include "../pdspCore.h"

namespace pdsp{
/*!
    @cond HIDDEN_SYMBOLS
*/ 
class MessageBuffer;
        
class SequencerBridge : public Unit{
        friend Patchable& linkBufferToSequencer (MessageBuffer& output, SequencerBridge& input);
        friend MessageBuffer;

public:
        virtual void link(MessageBuffer &messageBuffer) = 0;
        virtual void unLink() = 0;
        
        //TODO: add operator for linking MessageSources to sequencer
protected:
        virtual void resetMessageBufferSelector() {};
        MessageBuffer* messageBuffer;
        
};
        
Patchable& linkBufferToSequencer (MessageBuffer& output, SequencerBridge& input);
Patchable& operator>> (MessageBuffer& output, SequencerBridge& input);
        
/*!
    @endcond
*/         
        
} // pdsp namespace end



#endif  // PDSP_CONTROL_SEQUENCER_H_INCLUDED
