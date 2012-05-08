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
    finished_layer_id = 0;
    
    received_data_packets.clear();
    received_stamps.clear();
    
    //Making the decoder factory
    decoder_factory = new rlnc_decoder::factory(MAX_NUMBER_OF_SYMBOLS, MAX_SYMBOL_SIZE);
    
    
}

std::vector<uint8_t> decoder::decode(stamp* header, void* data){
    
    

    
    //Store the generation status info
    for (int i=0; i<generation_status.size(); i++) {
        if (generation_status[i].Layer_ID == header->Layer_ID) {
            generation_status[i].number_of_packets_received++;
            break;
        }
        else if (generation_status.size() == i-1){
            generation_status.resize(i+1);
            generation_status[i+1].Layer_ID = header->Layer_ID;
            generation_status[i+1].number_of_packets_received++;
        }
    }
    
    
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
    
    char data_stored[sizeof(*data)];
    memcpy(data_stored , data , sizeof(*data));
    received_data_packets.insert(received_data_packets.begin(), data_stored);
    
    received_stamps.insert(received_stamps.begin(),*header);
    
    
    
    //Check if it's the first generation or a different one than before
    if (CurrentGenerationID == 0 || CurrentGenerationID != header->Generation_ID) {

        //cout << "new generation detected" <<endl;
        
        

        //If there is any decoders, then check if they are done and empty them
        if (decoderinfo.size() != 0 && is_finished == false) {
            
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
            
            print_status();
            
            finished_layer_id = decoderinfo[finishedDecoderWithHighestLayerID].Layer_ID;
            
            
        }
        

        
        is_finished = false;
        
        finished_layer_id = 0;
        
        
        //Clear the vectors holding the decoders from the old generation (Also calls the destructors of the elements in the vectors)
        decoderinfo.clear();
        decoders.clear();
        generation_status.clear();
        received_stamps.resize(1);
        received_data_packets.resize(1);
        
        
        //Store the first layer information for the new generation
        generation_status.resize(1);
        generation_status[0].Layer_ID = header->Layer_ID;
        generation_status[0].number_of_packets_received++; 
        
        
        
        
        
        //Create a decoder for the first packet
        createDecoderWithHeader(header);

        CurrentGenerationID = header->Generation_ID;
        
    }
    else{
        
        //Create a new decoder if the current Layer_ID is not represented
        if (decoderAlreadyCreated == false) {
            //cout << "New Layer detected!" << endl;
            createDecoderWithHeader(header);
        }

    }
    
    
    //distribute the received packet to where it belongs by deciding upon the Layer_ID
    for (int i=0; i<decoderinfo.size(); i++) {
                
        if (decoderinfo[i].Layer_ID >= header->Layer_ID) {
            
            //print_stamp(header);
            
            decoders[i]->decode((uint8_t *)data);
            
            //cout << "Decoding Layer: " << decoderinfo[i].Layer_ID*1 << endl;
        }
        
        
        //if the largest decoder is complete then set the generation to finished!
        if (decoders[i]->is_complete() && decoderinfo[i].Layer_ID == header->Number_Of_Layers && is_finished == false) {
            print_status();

            //copy the decoded symbols
            
            data_out.resize(decoders[i] -> block_size());
            kodo::copy_symbols(kodo::storage(data_out), decoders[i]);
            
            finished_layer_id = decoderinfo[i].Layer_ID;
            
            is_finished = true;
            
                        
            
        }
        
    }
    
    return data_out;

}



void decoder::createDecoderWithHeader(stamp* header){
    
    
    decoders.push_back(decoder_factory->build(header->Layer_Size, header->Symbol_Size));
    
    decoderInfoStruct newDecoderInfo = {header->Layer_Size,header->Layer_ID,false};
    
    decoderinfo.push_back(newDecoderInfo);
        
    
    
    //Distribute the relevant old packets to the new decoder
    
    for (int i=0; i<decoderinfo.size(); i++) {        
        
        if (decoderinfo[i].Layer_ID <= header->Layer_ID) {
            
            for (int n=0; n<received_data_packets.size(); n++) {
                                
                if (received_stamps[n].Layer_ID < decoderinfo[i].Layer_ID) {
                    
                    decoders[i]->decode((uint8_t *)received_data_packets[n]);
                    
                }
                
                
            }  
        }
    }
    
}



void decoder::print_status(){
    
    for (int i=0; i<generation_status.size(); i++) {
        //cout << "Layer ID: " << generation_status[i].Layer_ID*1 << " Packets received: " << generation_status[i].number_of_packets_received << endl;
    }
    
    
}


uint8_t decoder::has_finished_decoding(){
    
    if (finished_layer_id) {
        
        return finished_layer_id;
    }
    
    return 0;
}


uint8_t decoder::get_current_generation_id(){
    return CurrentGenerationID;
}

