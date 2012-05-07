//File: decoder.h
//Created by Benjamin Krebs 3/5 2012



#ifndef _decoder_h
#define _decoder_h

#include "kodo/rlnc/full_vector_codes.h"
#include "../postoffice/Postoffice.h"

class decoder {

    uint8_t CurrentGenerationID;
    // Typdefs for the decoder type we wish to use
    typedef kodo::full_rlnc_decoder<fifi::binary8> rlnc_decoder;
    
    void createDecoderWithHeader(stamp* header);
    
    
    rlnc_decoder::factory* decoder_factory;

    struct decoderInfoStruct{
        uint8_t Layer_Size;
        uint8_t Layer_ID;
        bool isFinishedDecoding;
    };
    
    std::vector<stamp> received_stamps;
    std::vector<void*> received_data_packets;
    
    std::vector<uint8_t> data_out;
    std::vector<rlnc_decoder::pointer> decoders;
    std::vector<decoderInfoStruct> decoderinfo;
    
    
    
    
    
public:
    
    bool is_finished;
    
    decoder();
    
    std::vector<uint8_t> decode(stamp* header, void* data);
    
    
    uint8_t hasNumberOfLayers();
    void* getSymbolsInLayer(uint8_t LayerID);
    bool hasDecoderCompleted(uint8_t decoderForLayerID);
};



#define MAX_SYMBOL_SIZE 1400
#define MAX_NUMBER_OF_SYMBOLS 1400
                 
#endif
