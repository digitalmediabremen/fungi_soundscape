//
//  Connection.cpp
//  ofxLibwebsockets
//
//  Created by Brett Renfer on 4/11/12.
//

#include "ofxLibwebsockets/Connection.h"
#include "ofxLibwebsockets/Reactor.h"
#include "ofxLibwebsockets/Protocol.h"

namespace ofxLibwebsockets {
    
    //--------------------------------------------------------------
    Connection::Connection(Reactor* const _reactor, Protocol* const _protocol)
    : reactor(_reactor)
    , protocol(_protocol)
    , ws(NULL)
    //, buf(LWS_SEND_BUFFER_PRE_PADDING+1024+LWS_SEND_BUFFER_POST_PADDING)
    {
        if (_protocol != NULL){
            bufferSize = _protocol->rx_buffer_size;
            buf = (unsigned char*)calloc(LWS_SEND_BUFFER_PRE_PADDING+bufferSize+LWS_SEND_BUFFER_POST_PADDING, sizeof(unsigned char));
            binaryBuf = (unsigned char*)calloc(LWS_SEND_BUFFER_PRE_PADDING+bufferSize+LWS_SEND_BUFFER_POST_PADDING, sizeof(unsigned char));
        }
        idle = false;
    }
    
    //--------------------------------------------------------------
    Connection::~Connection(){
        ofLogNotice() << "Connection destructor...";
        close();
        free(buf);
        free(binaryBuf);
        
        // delete all pending frames
        messages_binary.clear();
        messages_text.clear();
    }
    //--------------------------------------------------------------
    void Connection::close() {
        // delete all pending frames
        ofLogNotice() << "Closing connection...";
        messages_binary.clear();
        messages_text.clear();
//        if (reactor != NULL){
//            reactor->close(this);
//        }
    }
    
    //--------------------------------------------------------------
    std::string Connection::getClientIP(){
        return client_ip;
    }
    
    //--------------------------------------------------------------
    std::string Connection::getClientName(){
        return client_name;
    }
    
    //--------------------------------------------------------------
    void Connection::setupAddress(){                        
        // old way:
        //int fd = lws_get_socket_fd( ws );
        //lws_get_peer_addresses(ws, fd, &client_name[0], client_name.size(),&client_ip[0], client_ip.size());

        char ip[128];
        lws_get_peer_simple(ws,ip,128);
        client_ip = ip;
        client_name = client_ip;
    }
    
    //--------------------------------------------------------------
    void Connection::send(const std::string& message)
    {
        if ( ws == NULL) return;
        if ( message.size() == 0 ) return;
        
        TextPacket tp;
        tp.index = 0;
        tp.message = message;
        messages_text.push_back(tp);
    }
    
    //--------------------------------------------------------------
    void Connection::sendBinary( ofBuffer & buffer ){
        sendBinary(buffer.getData(), buffer.size());
    }
    
    //--------------------------------------------------------------
    void Connection::sendBinary( unsigned char * data, unsigned int size ){
        sendBinary(reinterpret_cast<char *>(data), size);
    }
    
    //--------------------------------------------------------------
    void Connection::sendBinary( char * data, unsigned int size ){
        int n = -1;
        
        // changed 3/6/15: buffer all messages to prevent threading errors
        // we have a big frame, so we need to send a few times
        //        if ( bufferSize < size ){
        
        // need to split into packets
        BinaryPacket bp;
        bp.index = 0;
        bp.size = size;
        
        // copy data into array, in case user frees it
        bp.data = (unsigned char*)calloc(size, sizeof(unsigned char));
        memcpy(bp.data, data, size);
        
        messages_binary.push_back(bp);
    }
    
    //--------------------------------------------------------------
    void Connection::update(){

        // process standard ws messages
        if ( messages_text.size() > 0 && idle ){

            // grab first packet
            TextPacket & packet = messages_text[0];
            
            // either send a part of the message or just the message itself
            int dataSize = bufferSize > packet.message.size() ? packet.message.size() : bufferSize;
            
            // if "start" set 'write text'; otherwise we're sending a continuation
            int writeMode = packet.index == 0 ? LWS_WRITE_TEXT : LWS_WRITE_CONTINUATION;
            
            bool bDone = false;
            
            // are we going to write the whole packet here?
            if ( packet.index + dataSize >= packet.message.size() ){
                dataSize = packet.message.size() - packet.index;
                bDone = true;
            } else {
                writeMode |= LWS_WRITE_NO_FIN; // add "we're not finished" flag
            }
            
            // actual write to libwebsockets
            memcpy(&buf[LWS_SEND_BUFFER_PRE_PADDING], packet.message.c_str() + packet.index, dataSize );
            idle = false;
            
            int n = lws_write(ws, &buf[LWS_SEND_BUFFER_PRE_PADDING], dataSize, (lws_write_protocol) writeMode );
            
            if ( n < -1 ){
                ofLogError("ofxLibwebsockets")<< "Error writing to socket";
            }
            
            lws_callback_on_writable(ws);
            packet.index += dataSize;
            
            // packet sent completed, erase front of dequeue
            if ( bDone ){
                messages_text.pop_front();
            }
            
        } else if ( messages_text.size() > 0 && messages_text[0].index ){
            ofLogNotice() << "lws_callback_on_writable() called";
            lws_callback_on_writable(ws);
        }
        
        // process binary messages
        if ( messages_binary.size() > 0 && idle ){
            if ( messages_binary.size() > 0 ){
                ofLogVerbose() << "Process binary message...";
                BinaryPacket & packet = messages_binary[0];
            
                int dataSize = bufferSize > packet.size ? packet.size : bufferSize;
                int writeMode = packet.index == 0 ? LWS_WRITE_BINARY : LWS_WRITE_CONTINUATION;
                
                bool bDone = false;
                if ( packet.index + dataSize >= packet.size ){
                    dataSize = packet.size - packet.index;
                    bDone = true;
                } else {
                    writeMode |= LWS_WRITE_NO_FIN; // add "we're not finished" flag
                }
                
                memcpy(&binaryBuf[LWS_SEND_BUFFER_PRE_PADDING], packet.data + packet.index, dataSize );
                
                // this sets the protocol to wait until "idle"
                idle = false; // todo: this should be automatic on write!
                
                int n = lws_write(ws, &binaryBuf[LWS_SEND_BUFFER_PRE_PADDING], dataSize, (lws_write_protocol) writeMode );
                lws_callback_on_writable(ws);
                packet.index += dataSize;
                
                if ( n < -1 ){
                    ofLogError()<<"[ofxLibwebsockets] ERROR writing to socket";
                }
                
                if ( bDone ){
                    free(packet.data);
                    messages_binary.pop_front();
                }
            }
        } else if ( messages_binary.size() > 0 && messages_binary[0].index ){
            lws_callback_on_writable(ws);
        }
    }
    //--------------------------------------------------------------
    void Connection::setIdle( bool isIdle ){
        idle = isIdle;
        static string dum ="";
        if ( protocol != NULL ){
            Event args(*this, dum);
            ofNotifyEvent(protocol->onidleEvent, args);
        }
    }
    
    //--------------------------------------------------------------
    bool Connection::isIdle(){
        return idle;
    }
    
    //--------------------------------------------------------------
    bool Connection::operator==( const Connection &other ){
        if ( other.ws == NULL ) return false;
        return other.ws == ws;
    }
    
    //--------------------------------------------------------------
    bool Connection::operator!=( const Connection &other ){
        return other.ws != ws;
    }
}


