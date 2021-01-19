
#include "AAHighShelfEQ.h"


pdsp::AAHighShelfEQ::Submodule::Submodule(){
    addModuleInput( "signal", upsampler );
    addModuleInput( "freq", eq.in_freq() );
    addModuleInput( "Q", eq.in_Q() );
    addModuleInput("gain", eq.in_gain() );  
    addModuleOutput( "signal",  downsampler );
    eq.setOversampleLevel(2);
    upsampler >> eq >> downsampler;
}


void pdsp::AAHighShelfEQ::patch(){
    channels(1);
    addModuleInput(  "signal", *submodules[0]);
    addModuleOutput( "signal", *submodules[0]);
       
    addModuleInput("freq", freq);
    addModuleInput("Q", Q);
    addModuleInput("gain", gain );  
    
    freq.set(10000.0f);
    Q.set(0.707f);
    gain.set(0.0f);   
}


void pdsp::AAHighShelfEQ::channels( size_t size ){
    
    size_t oldsize = submodules.size();
    
    if( size >= oldsize ){
        submodules.resize( size );
                
        for (size_t i=oldsize; i<submodules.size(); ++i ){
            submodules[i] = new pdsp::AAHighShelfEQ::Submodule();
            freq >> submodules[i]->in("freq");
            Q >> submodules[i]->in("Q");
            gain >> submodules[i]->in("gain");            
        }        
    }else{
        for( size_t i=size; i<oldsize; ++i ){
            delete submodules[i];
        }
        submodules.resize( size );
    }
}

pdsp::Patchable& pdsp::AAHighShelfEQ::ch( size_t index ){
    if( index >= submodules.size() ){
        channels(index+1);
    }
    return *(submodules[index]);
}

pdsp::Patchable& pdsp::AAHighShelfEQ::in_signal(){
    return in("signal");
}

pdsp::Patchable& pdsp::AAHighShelfEQ::out_signal(){
    return out("signal");
}

pdsp::Patchable& pdsp::AAHighShelfEQ::in_freq(){
    return in("freq");
}

pdsp::Patchable& pdsp::AAHighShelfEQ::in_Q(){
    return in("Q");
}

pdsp::Patchable& pdsp::AAHighShelfEQ::in_gain(){
    return in("gain");
}



// ----------------- backward compatibility -------------------------

pdsp::Patchable& pdsp::AAHighShelfEQ::operator[]( size_t index ){
    return ch( index );
}

pdsp::Patchable& pdsp::AAHighShelfEQ::in_0(){
    if( submodules.size()<2 ){
        channels(2);
        addModuleInput( "0", *submodules[0] );
        addModuleInput( "1", *submodules[1] );
        addModuleOutput( "0", *submodules[0] );
        addModuleOutput( "1", *submodules[1] );
    } 
    return in("0");
}
    
pdsp::Patchable& pdsp::AAHighShelfEQ::in_1(){
    if( submodules.size()<2 ){
        channels(2);
        addModuleInput( "0", *submodules[0] );
        addModuleInput( "1", *submodules[1] );
        addModuleOutput( "0", *submodules[0] );
        addModuleOutput( "1", *submodules[1] );
    } 
    return in("1");
}

pdsp::Patchable& pdsp::AAHighShelfEQ::out_0(){
    if( submodules.size()<2 ){
        channels(2);
        addModuleInput( "0", *submodules[0] );
        addModuleInput( "1", *submodules[1] );
        addModuleOutput( "0", *submodules[0] );
        addModuleOutput( "1", *submodules[1] );
    } 
    return out("0");
}
    
pdsp::Patchable& pdsp::AAHighShelfEQ::out_1(){
    if( submodules.size()<2 ){
        channels(2);
        addModuleInput( "0", *submodules[0] );
        addModuleInput( "1", *submodules[1] );
        addModuleOutput( "0", *submodules[0] );
        addModuleOutput( "1", *submodules[1] );
    } 
    return out("1");
}

