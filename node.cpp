#include <iostream>
#include "kodo/rlnc/full_vector_codes.h"

#include "../postoffice/Postoffice.h"

int main(){
    
    uint32_t symbols = 42; //Generation size
    uint32_t symbol_size = 100; //Size of every symbol (bytes)
    
    // Typdefs for the encoder/decoder type we wish to use
    typedef kodo::full_rlnc_encoder<fifi::binary8> rlnc_encoder;//Passing af binary finite field obj to the encoder obj
    typedef kodo::full_rlnc_decoder<fifi::binary8> rlnc_decoder;
    
    //Make encoding factory with the symbols and their size
    rlnc_encoder::factory encoder_factory(symbols, symbol_size);    
    rlnc_encoder::pointer encoder = encoder_factory.build(symbols, symbol_size);
    
    rlnc_decoder::factory decoder_factory(symbols, symbol_size);
    rlnc_decoder::pointer decoder = decoder_factory.build(symbols, symbol_size);
    
    //Allocate storage for the payload
    std::vector<uint8_t> payload(encoder->payload_size());
    
    const char* PORT = "4000";
    
    
    
    Postoffice::createRxSocket(true, PORT, encoder->block_size());
    
    std::cout << encoder->block_size();
    
    int i=1;
    
    std::cout << "Ready to recieve\n\n" << std::endl;
//    for (i=0; i!=(symbols); i++) {
//        
//        const char *intermediate_char = Postoffice::recieve();
//        
//        payload[0] = *intermediate_char; //Recieve the encoded packets
//        
//        // Pass that packet to the decoder
//        decoder->decode( &payload[0] );
//        
//        std::cout << "Symbol " << i << " recieved: " << payload[0] << "\n" << std::endl;
//    }
    
    std::vector<uint8_t> data_out(decoder->block_size());
    
    while( 1 )
    {
        const char *intermediate_char = Postoffice::recieve();
        
        payload[0] = *intermediate_char; //Recieve the encoded packets
        
        
        std::cout << "Symbol " << i << " recieved: " << *intermediate_char << "\n" << std::endl;
        
        // Pass that packet to the decoder
        decoder->decode( &payload[0] );
        
        
        i++;
        
       
        if (decoder->is_complete()) {
            break;
        }
        
    }
    
    
    kodo::copy_symbols(kodo::storage(data_out), decoder); 
    
    std::cout << "\n\n Recieved: " << data_out[0] << data_out[1] << data_out[2] << "\n\n";
    
    
    Postoffice::closeConnection();
        
    
    // The decoder is complete, now copy the symbols from the decoder
    
    
    
    

    
}
