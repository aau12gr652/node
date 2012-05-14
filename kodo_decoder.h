//File: decoder.h
//Created by Benjamin Krebs 3/5 2012



#ifndef _decoder_h
#define _decoder_h

#include "kodo/rlnc/full_vector_codes.h"
#include <postoffice/Postoffice.h>
#include <iostream>
#include <deque>

class kodo_decoder {

    uint8_t CurrentGenerationID;
    // Typdefs for the decoder type we wish to use
    typedef kodo::full_rlnc_decoder<fifi::binary> rlnc_decoder;

    void createDecoderWithHeader(stamp* header);


    rlnc_decoder::factory* decoder_factory;

    struct decoderInfoStruct{
        uint8_t Layer_Size;
        uint8_t Layer_ID;
        bool isFinishedDecoding;
    };

    std::vector<stamp> received_stamps;
    std::vector<std::vector<void*> > received_data_packets;

    std::vector<uint8_t> data_out;
    std::vector<rlnc_decoder::pointer> decoders;
    std::vector<decoderInfoStruct> decoderinfo;

    void distribute_packet_to_decoders(stamp* header, void* data);

    struct layer_status{
        uint8_t Layer_ID;
        uint32_t number_of_packets_received;
    };


    std::vector<layer_status> generation_status;

    uint8_t finished_layer_id;



    void print_status();
    void release_received_data_packets(int start);


public:

    uint8_t is_finished; //Has the value 0 if false, 1 if it finished the highest layer, 2 if it was forced to finish

    kodo_decoder();
    bool status_output;

    std::vector<uint8_t> decode(stamp* header, serial_data letter);

    uint8_t get_current_generation_id();
    uint8_t has_finished_decoding();

    int is_layer_finish(int L);
};



#define MAX_SYMBOL_SIZE 1500
#define MAX_NUMBER_OF_SYMBOLS 1500

#endif
