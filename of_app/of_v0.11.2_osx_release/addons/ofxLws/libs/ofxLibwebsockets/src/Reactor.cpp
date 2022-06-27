//
//  Reactor.cpp
//  ofxLibwebsockets
//
//  Created by Brett Renfer on 4/12/12. 
//

#include "ofxLibwebsockets/Reactor.h"
#include "ofxLibwebsockets/Util.h"

namespace ofxLibwebsockets { 

	vector<Reactor *> reactors = vector<Reactor *>();

    //--------------------------------------------------------------
    Reactor::Reactor()
    : context(NULL), waitMillis(20){
        //reactors.push_back(this);
        bParseJSON = true;
        largeMessage = "";
        largeBinaryMessage.clear();
        largeBinarySize     = 0;
        bReceivingLargeMessage  = false;
        bAllowDuplicateConnections = true;
    }

    //--------------------------------------------------------------
    Reactor::~Reactor(){
        //exit();
    }

    //--------------------------------------------------------------
    void Reactor::registerProtocol(const std::string& name, Protocol& protocol){
        protocol.idx = protocols.size();
        protocol.rx_buffer_size = OFX_LWS_MAX_BUFFER;
        protocol.reactor = this;
        protocols.push_back(make_pair(name, &protocol));
    }

    //--------------------------------------------------------------
    Protocol* const Reactor::protocol(const unsigned int idx){
        return (idx < protocols.size())? protocols[idx].second : NULL;
    }

    //--------------------------------------------------------------
    void Reactor::setWaitMillis(int millis){
        waitMillis = millis;
    }

    //--------------------------------------------------------------
//    void Reactor::exit(){
//        if (context != NULL)
//        {
//            ofLogNotice() << "Reactor exiting....";
//            if (isThreadRunning()){
//                // this is the strategy from ofxKinect
//                stopThread();
//                ofSleepMillis(10);
//                waitForThread(false,5000);
//            }
//			// on windows the app does crash if the context is destroyed
//			// while the thread or the library still might hold pointers
//			// better to live with non deleted memory, or?
//            lws_context_destroy(context);
//            //context = NULL;
//        }
//    }


    //--------------------------------------------------------------
    struct lws_context * Reactor::getContext(){
        return context;
    }
    
    //--------------------------------------------------------------
    vector<Connection *> Reactor::getConnections(){
        return connections;
    }
    
    //--------------------------------------------------------------
    Connection * Reactor::getConnection( int index ){
        if ( index < connections.size() ){
            return connections[ index ];
        }
        return NULL;
    }

    //--------------------------------------------------------------
    unsigned int
    Reactor::_allow(struct lws *ws, Protocol* const protocol, const long fd){
        std::string client_ip(128, 0);
        std::string client_name(128, 0);
        
        lws_get_peer_addresses(ws, lws_get_socket_fd(ws),
                                         &client_name[0], client_name.size(),
                                         &client_ip[0], client_ip.size());
        return protocol->_allowClient(client_name, client_ip);
    }

    //--------------------------------------------------------------
    unsigned int Reactor::_notify(Connection* conn,
                                enum lws_callback_reasons const reason,
                                const char* const _message,
                                const unsigned int len){
        
        // this happens with events that don't use the connection so not always a problem
        if (conn == NULL || conn->protocol == NULL || conn->ws == NULL ){
            if (conn == NULL){
                ofLogVerbose("ofxLibwebsockets") << "Connection is NULL. Reason: " << getCallbackReason(reason);
            } else {
                ofLogVerbose("ofxLibwebsockets") << "Protocol is NULL. Reason: " << getCallbackReason(reason);
            }
            return 1;
        }
        
        std::string message;
        Event args(*conn, message);
        
        switch (reason) {
            // connection was not successful
            case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
                ofLogError()<<"[ofxLibwebsockets] Connection error";
                
                for (size_t i=0; i<connections.size(); i++){
                    if ( connections[i] == conn ){
                        connections.erase( connections.begin() + i );
                        break;
                    }
                }
                ofNotifyEvent(conn->protocol->oncloseEvent, args);
                break;
                
            // last thing that happens before connection goes dark
            case LWS_CALLBACK_WSI_DESTROY:
            {
                bool bFound = false;
                for (size_t i=0; i<connections.size(); i++){
                    if ( connections[i] == conn ){
                        bFound = true; // valid connection
                        connections.erase( connections.begin() + i );
                        break;
                    }
                }
                
                if ( bFound ) ofNotifyEvent(conn->protocol->oncloseEvent, args);
            }
                break;
            
            case LWS_CALLBACK_CLIENT_ESTABLISHED:   // client connected with server
                connections.push_back( conn );
                ofNotifyEvent(conn->protocol->onconnectEvent, args);
                break;
            case LWS_CALLBACK_ESTABLISHED:          // server connected with client
                if(bAllowDuplicateConnections) {
                    connections.push_back( conn );
                    ofNotifyEvent(conn->protocol->onconnectEvent, args);
                }  else {
                    for (size_t i=0; i<connections.size(); i++){
                        if ( strcmp(connections[i]->getClientIP().c_str(),conn->getClientIP().c_str()) == 0)
                        {
                            //close the connection
                            return 1;
                        }
                    }
                    connections.push_back( conn );
                    ofNotifyEvent(conn->protocol->onconnectEvent, args);
                }
                break;
                
            case LWS_CALLBACK_CLOSED:
                // erase connection from vector
                for (size_t i=0; i<connections.size(); i++){
                    if ( connections[i] == conn ){
                        ofLogNotice() << "Deleting connection";
                        connections.erase( connections.begin() + i );
                        break;
                    }
                }
                
                ofNotifyEvent(conn->protocol->oncloseEvent, args);
                break;
                
            case LWS_CALLBACK_SERVER_WRITEABLE:
            case LWS_CALLBACK_CLIENT_WRITEABLE:
                // idle is good! means you can write again
                conn->setIdle();
                break;
                
            case LWS_CALLBACK_RECEIVE:              // server receive
            case LWS_CALLBACK_CLIENT_RECEIVE:       // client receive
            case LWS_CALLBACK_CLIENT_RECEIVE_PONG:
                {
                    
                    bool bFinishedReceiving = false;
                    
                    // decide if this is part of a larger message or not
                    size_t bytesLeft = lws_remaining_packet_payload( conn->ws );
                    
                    if ( !bReceivingLargeMessage && (bytesLeft > 0 || !lws_is_final_fragment( conn->ws )) ){
                        bReceivingLargeMessage = true;
                    }
                    
                    // text or binary?
                    int isBinary = lws_frame_is_binary(conn->ws);
                    
                    if (isBinary == 1 ){
                        // set binary flag on event
                        args.isBinary = true;
                        
                        if ( bReceivingLargeMessage){
                            // need to allocate data...
                            if ( largeBinarySize == 0 ){
                                largeBinaryMessage.set( _message, len );
                                largeBinarySize = len;
                            } else {
                                largeBinarySize += len;
                                largeBinaryMessage.append(_message, len);
                            }
                            
                            if ( bytesLeft == 0 && lws_is_final_fragment( conn->ws )){
                                // copy into event
                                args.data.set(largeBinaryMessage.getData(), largeBinaryMessage.size());
                                
                                bFinishedReceiving      = true;
                                bReceivingLargeMessage  = false;
                                largeBinaryMessage.clear();
                                largeBinarySize = 0;
                            }
                        } else {
                            args.data.set(_message, len);
                            
                            bFinishedReceiving      = true;
                            bReceivingLargeMessage  = false;
                            largeBinaryMessage.clear();
                            largeBinarySize = 0;
                        }
                    } else {
                        if (_message != NULL && len > 0){
                            args.message = std::string(_message, len);
                        }
                        
                        if ( bReceivingLargeMessage){
                            largeMessage += args.message;
                            if ( bytesLeft == 0 && lws_is_final_fragment( conn->ws )){
                                args.message = largeMessage;
                                bFinishedReceiving      = true;
                                bReceivingLargeMessage  = false;
                                largeMessage = "";
                            }
                        }
                        
                        if (_message != NULL && len > 0 && (!bReceivingLargeMessage || bFinishedReceiving) ){
                            
                            if ( bParseJSON ){
                                try {
                                    args.json = ofJson::parse( args.message );
                                }
                                catch( std::exception& e ){
                                    // report to the user the failure
                                    args.json.clear();
                                    ofLogVerbose() << "[ofxLibwebsockets] Failed to parse JSON: " <<  e.what();
                                }
                            }
                            
                        }
                    }
                    
                    // only notify if we have a complete message
                    if (!bReceivingLargeMessage || bFinishedReceiving){
                        ofNotifyEvent(conn->protocol->onmessageEvent, args);
                    }
                }
                break;

            case LWS_CALLBACK_PROTOCOL_DESTROY:
            case LWS_CALLBACK_CLIENT_CLOSED:
            case LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP:
            case LWS_CALLBACK_EVENT_WAIT_CANCELLED:                
                break;
                
            default:
                ofLogNotice("Reactor") << "Received unknown event " << reason;
                break;
        }
        
        return 0;
    }

    //--------------------------------------------------------------
    unsigned int Reactor::_http(struct lws *ws,
                              const char* const _url){
        std::string url(_url);
        if (url == "/")
            url = "/index.html";
        
        // why does this need to be done?
        std::string ext = url.substr(url.find_last_of(".")+1);
        // watch out for query strings!
        size_t find = url.find("?");
        if ( find!=string::npos ){
            url = url.substr(0,url.find("?"));
        }
        std::string file = document_root+url;
        std::string mimetype = "text/html";
        
        if (ext == "ico")
            mimetype = "image/x-icon";
        if (ext == "manifest")
            mimetype = "text/cache-manifest";
        if (ext == "swf")
            mimetype = "application/x-shockwave-flash";
        if (ext == "js")
            mimetype = "application/javascript";
        if (ext == "css")
            mimetype = "text/css";
        
        if (lws_serve_http_file(ws, file.c_str(), mimetype.c_str(), "", 0) < 0){
            ofLog( OF_LOG_WARNING, "[ofxLibwebsockets] Failed to send HTTP file "+ file + " for "+ url);
        }
        
        return 1; // 1 will close the HTTP connection when done
    }
}
