//File: decoder.cpp
//Created by Benjamin Krebs 3/5 2012

#include <iostream>
#include "decoder.h"
#include "../postoffice/Postoffice.h"
#include "kodo/rlnc/full_vector_codes.h"

// **** decoder class implementation **** //
using namespace std;

decoder::decoder(){
    //Initialize instance variables
    is_finished = false;
    CurrentGenerationID = 0;
    
    
    received_data_packets.clear();
    received_stamps.clear();
    
    //Making the decoder factory
    decoder_factory = new rlnc_decoder::factory(MAX_NUMBER_OF_SYMBOLS, MAX_SYMBOL_SIZE);
    
    
}

std::vector<uint8_t> decoder::decode(stamp* header, void* data){
    
    
    
    //Zero pad the incoming data
    memset((uint16_t*)data + header->Layer_Size, 0, header->Generation_Size - header->Layer_Size);
     
    
    //Decide wether the new Layer_ID is represented by a decoder
    bool decoderAlreadyCreated = false;
    
    
    //Check if the Layer_ID received is already represented by a decoder
    for (int i=0; i<decoderinfo.size(); i++) {
        
        if (header->Layer_ID == decoderinfo[i].Layer_ID) {
            decoderAlreadyCreated = true;
            break;
        }
        
    }
    
    received_data_packets.insert(received_data_packets.begin(), data);
    received_stamps.insert(received_stamps.begin(),*header);
    
    
    //Check if it's the first generation or a different one than before
    if (CurrentGenerationID == 0 || CurrentGenerationID != header->Generation_ID) {

        cout << "new generation detected" <<endl;

        //If there is any decoders, then check if they are done and empty them
        if (decoderinfo.size() != 0) {
            
            int finishedDecoderWithHighestLayerID = 0;
            
            
            for (int i=0; i<decoders.size(); i++) {
                if (decoders[i]->is_complete()) {
                    
                    if (finishedDecoderWithHighestLayerID<decoderinfo[i].Layer_ID) {
                        finishedDecoderWithHighestLayerID = i;
                    }
                    
                }
            }
            
            
            //copy the decoded symbols
            
            data_out.resize(decoders[finishedDecoderWithHighestLayerID] -> block_size());
            kodo::copy_symbols(kodo::storage(data_out), decoders[finishedDecoderWithHighestLayerID]);
            
            
            //A little debugging
            cout << "Decoder: " << decoderinfo[finishedDecoderWithHighestLayerID].Layer_ID*1<< " chosen" << endl << endl;
            
            
            for (int i=0; i<decoderinfo[finishedDecoderWithHighestLayerID].Layer_Size; i++) {
                cout << data_out[i];
            }
            cout << endl << endl;
            
        }
        

        
        
        //Clear the vectors holding the decoders from the old generation (Also calls the destructors of the elements in the vectors)
        decoderinfo.clear();
        decoders.clear();
        received_stamps.resize(1);
        received_data_packets.resize(1);
        
        
        is_finished = true;
        
        
        
        //Create a decoder for the first packet
        createDecoderWithHeader(header);

        CurrentGenerationID = header->Generation_ID;
        
    }
    else{
        
        is_finished = false;
        
        //Create a new decoder if the current Layer_ID is not represented
        if (decoderAlreadyCreated == false) {
            cout << "New Layer detected!" << endl;
            createDecoderWithHeader(header);
        }

    }
    
    
    
    //distribute the received packet to where it belongs by deciding upon the Layer_ID
    for (int i=0; i<decoderinfo.size(); i++) {
        
        cout << *(char*)data << " " << header->Layer_Size*1 << endl;
        
        if (decoderinfo[i].Layer_ID >= header->Layer_ID) {
            decoders[i]->decode((uint8_t *)data);
            cout << "Decoding Layer: " << decoderinfo[i].Layer_ID*1 << endl;
        }
        
        if (decoders[i]->is_complete()) {
            cout << "Decoder: " << decoderinfo[i].Layer_ID*1 << " Completed"<<endl;
        }
        
        
    }

    return data_out;
    

    
    
}

void decoder::createDecoderWithHeader(stamp* header){
    
    
    decoders.push_back(decoder_factory->build(header->Layer_Size, header->Symbol_Size));
    
    decoderInfoStruct newDecoderInfo = {header->Layer_Size,header->Layer_ID,false};
    
    decoderinfo.push_back(newDecoderInfo);
    
    //cout << "Decoder for layer: " << header->Layer_ID*1 << " Generation: " << header->Generation_ID*1 <<endl;
    
    
    //Distribute the relevant old packets to the new decoder
    
    for (int i=0; i<decoderinfo.size(); i++) {
        //cout << "\n\nChecking decoder: " << decoderinfo[i].Layer_ID*1 << endl;
        
        
        if (decoderinfo[i].Layer_ID <= header->Layer_ID) {
            
            for (int n=0; n<received_data_packets.size(); n++) {
                
                //cout << received_stamps[n].Layer_ID*1 << " " << decoderinfo[i].Layer_ID*1 << " " << endl;
                
                if (received_stamps[n].Layer_ID < decoderinfo[i].Layer_ID) {
                    
                    decoders[i]->decode((uint8_t *)received_data_packets[n]);
                    
                    cout << "Added packet from Layer: " << received_stamps[n].Layer_ID*1 << " to decoder: " << decoderinfo[i].Layer_ID*1 << endl;
                }
                
                
            }  
        }
    }
    
}
