//File: decoder.cpp
//Created by Benjamin Krebs 3/5 2012

#include "kodo_decoder.h"

// **** decoder class implementation **** //
using namespace std;

kodo_decoder::kodo_decoder(){
    //Initialize instance variables
    is_finished = false;
    CurrentGenerationID = -10;
    finished_layer_id = 0;
    status_output=false;

    received_data_packets.clear();
    received_stamps.clear();

    //Making the decoder factory
    decoder_factory = new rlnc_decoder::factory(MAX_NUMBER_OF_SYMBOLS, MAX_SYMBOL_SIZE);


}

std::vector<uint8_t> kodo_decoder::decode(stamp* header, serial_data letter){


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



    ///////TOUCHING THE DATA!
    void* data = letter.data;

    //Store the received stamp as the first element
    received_stamps.insert(received_stamps.begin(),*header);

    vector<void *> received_data_packet_for_layer(header->Number_Of_Layers);


    for (int n=0; n<header->Number_Of_Layers; n++) {

        void *data_stored = malloc(1500);

        ///////TOUCHING THE DATA!
        memcpy(data_stored , data , letter.size);

        //Zero pad the incoming data
        memset((uint8_t*)data_stored + letter.size, 0, 1500-letter.size);

        ///////TOUCHING THE DATA!
        received_data_packet_for_layer[n] = data_stored;
    }

    ///////TOUCHING THE DATA!
    received_data_packets.insert(received_data_packets.begin(), received_data_packet_for_layer);

    //Check if it's the first generation or a different one than before
    if (CurrentGenerationID != header->Generation_ID) {
        //cout << "new generation detected" <<endl;


        //If there is any old decoders, then check if they are done and empty them
        if (decoderinfo.size() != 0 && is_finished == 0) {

            //cout << "finished ON new generation" << endl;
            print_status();

            //Find the finished decoder with the highest layer ID
            int finishedDecoderWithHighestLayerID = -1;

            for (int i=0; i<decoders.size(); i++) {
                if (decoders[i]->is_complete())
                    if (finishedDecoderWithHighestLayerID < 0)
                        finishedDecoderWithHighestLayerID = i;
                    else
                        if (decoderinfo[finishedDecoderWithHighestLayerID].Layer_ID<=decoderinfo[i].Layer_ID)
                            finishedDecoderWithHighestLayerID = i;

            }

            if (finishedDecoderWithHighestLayerID>=0) {
                //copy the decoded symbols
                data_out.resize(decoders[finishedDecoderWithHighestLayerID] -> block_size());
                kodo::copy_symbols(kodo::storage(data_out), decoders[finishedDecoderWithHighestLayerID]);

                //print_status();

                is_finished = 2;
                finished_layer_id = decoderinfo[finishedDecoderWithHighestLayerID].Layer_ID;
            }
            else{
                   //cout << "finishedDecoderWithHighestLayerID == -1: " << finishedDecoderWithHighestLayerID*1 << " Layer: " << decoderinfo[finishedDecoderWithHighestLayerID].Layer_ID*1 << endl;
                is_finished = 0;
                finished_layer_id=0;
            }

            ///////TOUCHING THE DATA!
            release_received_data_packets(1);

        }
        else{
            //cout << "Decoderinfo.size == 0: " << decoderinfo.size()*1 << " OR is_finished != 0: " << decoderinfo.size()*1 << endl;
            is_finished = 0;
            finished_layer_id=0;
        }




///////TOUCHING THE DATA!///////TOUCHING THE DATA!///////TOUCHING THE DATA!
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
        distribute_packet_to_decoders(header, data);

        CurrentGenerationID = header->Generation_ID;

    }
    else{

        is_finished = 0;

        //Check if the Layer_ID received is already represented by a decoder
        for (int i=0; i<decoderinfo.size(); i++) {

            if (header->Layer_ID == decoderinfo[i].Layer_ID) {break;}
            else if (i == decoderinfo.size()-1)
                createDecoderWithHeader(header);
        }

        distribute_packet_to_decoders(header, data);
    }


    return data_out;

}


void kodo_decoder::distribute_packet_to_decoders(stamp* header, void* data){

    //distribute the received packet to where it belongs by deciding upon the Layer_ID
    for (int i=0; i<decoderinfo.size(); i++) {

        if (decoderinfo[i].Layer_ID >= header->Layer_ID) { // FJERNET >

            //print_stamp(header);

            decoders[i]->decode((uint8_t*)received_data_packets[0][decoderinfo[i].Layer_ID-1]);

            //cout << "Decoding Layer: " << decoderinfo[i].Layer_ID*1 << endl;
        }
        if (decoders[i]->is_complete())
            decoderinfo[i].isFinishedDecoding = true;


        //if the largest decoder is complete then set the generation to finished!
        if (decoders[i]->is_complete() && decoderinfo[i].Layer_ID == header->Number_Of_Layers && is_finished == 0) {

            //cout << "finished before new generation" << endl;
            //print_status();

            //copy the decoded symbols

            data_out.resize(decoders[i] -> block_size());
            kodo::copy_symbols(kodo::storage(data_out), decoders[i]);

            finished_layer_id = decoderinfo[i].Layer_ID;

            is_finished = 1;

            release_received_data_packets(0);
            break;

        }

    }


}

void kodo_decoder::createDecoderWithHeader(stamp* header){

    decoders.push_back(decoder_factory->build(header->Layer_Size, header->Symbol_Size));


    decoderInfoStruct newDecoderInfo = {header->Layer_Size,header->Layer_ID,false};

    decoderinfo.push_back(newDecoderInfo);

    //Distribute the relevant old packets to the new decoder

    for (int i=0; i<decoderinfo.size(); i++) {

        if (decoderinfo[i].Layer_ID <= header->Layer_ID) {

            for (int n=0; n<received_data_packets.size(); n++) {

                if (received_stamps[n].Layer_ID >= decoderinfo[i].Layer_ID) { // HER HAR VI FJERNET ET >

                    decoders[i]->decode((uint8_t *)(received_data_packets[n][decoderinfo[i].Layer_ID-1]));

                    //cout << "Packet: " << n*1 << " ->Decoder: " << decoderinfo[i].Layer_ID*1 << endl;
                }


            }
        }
        if (decoders[i]->is_complete())
            decoderinfo[i].isFinishedDecoding = true;
    }
}



void kodo_decoder::print_status(){
    if (status_output==true) {
        for (int i=0; i<generation_status.size(); i++) {
            cout << "Layer ID: " << generation_status[i].Layer_ID*1 << " Packets received: " << generation_status[i].number_of_packets_received << endl;
        }
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

void kodo_decoder::release_received_data_packets(int start)
{
    int stop = received_data_packets.size();
    for (int n = start; n < stop; n++)
        for (int i=0; i<received_data_packets[start].size(); i++)
            free(received_data_packets[n][i]);

    received_data_packets.erase(received_data_packets.begin()+start,received_data_packets.end());
}

int kodo_decoder::is_layer_finish(int L)
{
    for (int n = 0; n < decoderinfo.size(); n++)
        if (decoderinfo[n].Layer_ID == L)
            if (decoderinfo[n].isFinishedDecoding)
                return 1;
            else
                return 0;
    return -1;
}
