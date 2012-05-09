//File: decoder.cpp
//Created by Benjamin Krebs 3/5 2012

#include <iostream>
#include "kodo_decoder.h"
#include "../postoffice/Postoffice.h"
#include "kodo/rlnc/full_vector_codes.h"

// **** decoder class implementation **** //
using namespace std;

kodo_decoder::kodo_decoder(){
    //Initialize instance variables
    is_finished = false;
    CurrentGenerationID = 0;
    finished_layer_id = 0;
    
    received_data_packets.clear();
    received_stamps.clear();
    
    //Making the decoder factory
    decoder_factory = new rlnc_decoder::factory(MAX_NUMBER_OF_SYMBOLS, MAX_SYMBOL_SIZE);
    
    
}

std::vector<uint8_t> kodo_decoder::decode(stamp* header, serial_data letter){
    
    void* data = letter.data;
    

     
    
    //Decide wether the new Layer_ID is represented by a decoder
    bool decoderAlreadyCreated = false;
    
    
    //Check if the Layer_ID received is already represented by a decoder
    for (int i=0; i<decoderinfo.size(); i++) {
        
        if (header->Layer_ID == decoderinfo[i].Layer_ID) {
            decoderAlreadyCreated = true;
            break;
        }
        
    }
    
    
    void *data_stored = malloc(1500);
    
    memcpy(data_stored , data , letter.size);
    
    //Zero pad the incoming data
    memset((uint16_t*)data_stored + letter.size, 0, 1500-letter.size);
    
    //Store the received packet as the first element
    
    received_data_packets.insert(received_data_packets.begin(), data_stored);
    
    
    //print_some_shit(data, 11);
    //cout << "her: " << *(char*)received_data_packets[0]<< " " << *(char*)data << endl;
    
    //Store the received stamp as the first element
    received_stamps.insert(received_stamps.begin(),*header);
    
    
    
    //Check if it's the first generation or a different one than before
    if (CurrentGenerationID == 0 || CurrentGenerationID != header->Generation_ID) {

        //cout << "new generation detected" <<endl;
        
                
        //If there is any old decoders, then check if they are done and empty them
        if (decoderinfo.size() != 0 && is_finished == false) {
            
            //cout << "finished ON new generation" << endl;
            
            int finishedDecoderWithHighestLayerID = 0;
            
            
            for (int i=0; i<decoders.size(); i++) {
                if (decoders[i]->is_complete()) {
                    
                    if (finishedDecoderWithHighestLayerID<=decoderinfo[i].Layer_ID) {
                        finishedDecoderWithHighestLayerID = i;
                    }
                    
                }
            }
            
            
            //copy the decoded symbols
            data_out.resize(decoders[finishedDecoderWithHighestLayerID] -> block_size());
            kodo::copy_symbols(kodo::storage(data_out), decoders[finishedDecoderWithHighestLayerID]);
            
            //print_status();
            
            is_finished = true;
            finished_layer_id = decoderinfo[finishedDecoderWithHighestLayerID].Layer_ID;
            
            
        }
        else{
            is_finished = false;
            
            finished_layer_id = 0;
        }
        

        
        
        
        
        //Clear the vectors holding the decoders from the old generation (Also calls the destructors of the elements in the vectors)
        decoderinfo.clear();
        decoders.clear();
        generation_status.clear();
        received_stamps.resize(1);
        received_data_packets.resize(1);
        
        
        //Store the first layer information for the new generation
        generation_status.resize(1);
        generation_status[0].Layer_ID = header->Layer_ID;
        generation_status[0].number_of_packets_received=1; 
        
        
        
        
        
        //Create a decoder for the first packet
        createDecoderWithHeader(header);

        
        
    }
    else{
        
        is_finished = false;
        
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
            
            //cout << "finished before new generation" << endl;
            //print_status();

            //copy the decoded symbols
            
            data_out.resize(decoders[i] -> block_size());
            kodo::copy_symbols(kodo::storage(data_out), decoders[i]);
            
            finished_layer_id = decoderinfo[i].Layer_ID;
            
            is_finished = true;
            
                        
            
        }
        
    }
    
    
    //Store the generation status info
    for (int i=0; i<generation_status.size(); i++) {
        if (generation_status[i].Layer_ID == header->Layer_ID) {
            generation_status[i].number_of_packets_received++;
            break;
        }
        else if (generation_status.size() == i+1){
            layer_status newlayerstatus = {header->Layer_ID,1};
            generation_status.push_back(newlayerstatus);
        }
    }
    
    
    CurrentGenerationID = header->Generation_ID;
    return data_out;

}

void print_some_shit(void* data, int size)
{
    for (int n = 0; n < size; n++)
        cout << ((uint8_t*)data)[n]*1 << " ";
    cout << endl;
}

void kodo_decoder::createDecoderWithHeader(stamp* header){
    
    decoders.push_back(decoder_factory->build(header->Layer_Size, header->Symbol_Size));
    
    
    decoderInfoStruct newDecoderInfo = {header->Layer_Size,header->Layer_ID,false};
    
    decoderinfo.push_back(newDecoderInfo);
    
    
        
    
    
    //Distribute the relevant old packets to the new decoder
    
    for (int i=0; i<decoderinfo.size(); i++) {        
        
        if (decoderinfo[i].Layer_ID <= header->Layer_ID) {
            
            for (int n=0; n<received_data_packets.size(); n++) {
                                
                if (received_stamps[n].Layer_ID < decoderinfo[i].Layer_ID) {
                    
                    decoders[i]->decode((uint8_t *)(received_data_packets[n]));
                    //print_some_shit(received_data_packets[n], 11);
                    
                    //cout << "Packet: " << n*1 << " ->Decoder: " << decoderinfo[i].Layer_ID*1 << endl;
                }
                
                
            }  
        }
    }
    
}



void kodo_decoder::print_status(){
    
    for (int i=0; i<generation_status.size(); i++) {
        cout << "Layer ID: " << generation_status[i].Layer_ID*1 << " Packets received: " << generation_status[i].number_of_packets_received << endl;
    }
    
    
}



uint8_t kodo_decoder::has_finished_decoding(){
    
    if (finished_layer_id) {
        
        return finished_layer_id;
    }
    
    return 0;
}


uint8_t kodo_decoder::get_current_generation_id(){
    return CurrentGenerationID;
}

