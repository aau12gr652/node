#include <iostream>
#include "kodo/rlnc/full_vector_codes.h"

#include "../postoffice/Postoffice.h"

int main(){

	int packet_counter=1;
    bool has_completed_1 = false;
    bool has_completed_2 = false;
    bool has_completed_3 = false;
    
    // Set the number of symbols (i.e. the generation size in RLNC
    // terminology) and the size of a symbol in bytes 
    uint32_t symbols_max = 13;
    uint32_t symbol_size = 1;
    
    
    uint32_t symbols_1 = 5; //The size of HELLO
    uint32_t symbols_2 = 8; // The size of HELLO WO
    uint32_t symbols_3 = symbols_max; //The size of HELLO WORLD!
    
    
    // Typdefs for the encoder/decoder type we wish to use
    typedef kodo::full_rlnc_decoder<fifi::binary8> rlnc_decoder;
    
    //Making the factories
    rlnc_decoder::factory decoder_factory(symbols_max, symbol_size);
    
    
    //Making the decoders with different sizes
    rlnc_decoder::pointer decoder_1 = decoder_factory.build(symbols_1, symbol_size);
    rlnc_decoder::pointer decoder_2 = decoder_factory.build(symbols_2, symbol_size);
    rlnc_decoder::pointer decoder_3 = decoder_factory.build(symbols_3, symbol_size);
    
    
    serial_data letter;
    
    //Generate postoffice
    postoffice po("4000");
    
    while (1337) {
    
    
        //Strip of the header
        char data[100];
        std::cout << "Jeg modtager!" << std::endl;
        serial_data received_letter = {po.receive(data,100), data};
        
        std::cout << "Jeg har modtaget!" << std::endl;
        stamp* header = (stamp*)malloc(sizeof(stamp));
        serial_data letter_payload = unfrank(received_letter, header);
        
        //Zero-pad the received packet up to the generation size
        memset((uint16_t*)letter_payload.data + header->Layer_Size, 0, header->Generation_Size - header->Layer_Size);
        
        std::cout << "Packet received for layer: "<< header->Layer_ID*1 << "size: " << header->Layer_Size*1 << "rec size: " << received_letter.size*1 << std::endl;
        //Fordel pakkerne til deres respektive decoders
        
        
        if (header->Layer_ID == 3){
			decoder_3->decode( (uint8_t*)letter_payload.data ); 
        }
        else if (header->Layer_ID == 2){
        	decoder_2->decode( (uint8_t*)letter_payload.data );
        	decoder_3->decode( (uint8_t*)letter_payload.data );
        }
        else{
        	decoder_1->decode( (uint8_t*)letter_payload.data );
        	decoder_2->decode( (uint8_t*)letter_payload.data );
        	decoder_3->decode( (uint8_t*)letter_payload.data );
        }
        
        
        
        
        if (decoder_1->is_complete() && has_completed_1==false) {
            
            //Create data_out_1 vector with the decoded data
            std::vector<uint8_t> data_out_1(decoder_1->block_size());
            kodo::copy_symbols(kodo::storage(data_out_1), decoder_1); 
            std::cout << "Decoder 1 done with message: ";
            for (int i=0; i<data_out_1.size(); i++) {
                 std::cout << data_out_1[i];
            }
            std::cout << "\nnumber of packets received: " << packet_counter << std::endl;
            has_completed_1 = true;
        
            
            
        }
        else if (decoder_2->is_complete() && has_completed_2==false) {
            
            //Create data_out_2 vector with the decoded data
            std::vector<uint8_t> data_out_2(decoder_2->block_size());
            kodo::copy_symbols(kodo::storage(data_out_2), decoder_2); 
            std::cout << "Decoder 2 done with message: ";
            for (int i=0; i<data_out_2.size(); i++) {
                std::cout << data_out_2[i] ;
            }
            std::cout << "\nnumber of packets received: " << packet_counter <<std::endl;
            has_completed_2 = true;
            break;
            
        }
        else if (decoder_3->is_complete() && has_completed_3==false) {
            
            //Create data_out_3 vector with the decoded data
            std::vector<uint8_t> data_out_3(decoder_3->block_size());
            kodo::copy_symbols(kodo::storage(data_out_3), decoder_3); 
            std::cout << "Decoder 3 done with message: ";
            for (int i=0; i<data_out_3.size(); i++) {
                std::cout << data_out_3[i];
            }
            
            std::cout << "\nnumber of packets received: " << packet_counter << std::endl;
            
            has_completed_3 = true;
            
            break;
        }
        
        packet_counter++;
    }
    
    po.closeConnection();
    return 0;
}
