#include <iostream>

#include "../postoffice/Postoffice.h"
#include "kodo_decoder.h"

using namespace std;

int main(){
    char data[100];
    
    vector<uint8_t> returnval(13);
    
    kodo_decoder Mydecoder=kodo_decoder();
    
    postoffice po("4000");
    
    stamp* header = (stamp*)malloc(sizeof(stamp));
    
    cout << "Receiving" << endl;
    
    int decoded_generation_id=0;
    
    serial_data* received_letter = (serial_data*)malloc(sizeof(serial_data));;
    
    while (1) {
        
        received_letter->size = po.receive(data, 100, header);
        
        received_letter->data = data;
        
        
        
        //cout<< "modtog fra lag: " << header->Layer_ID*1 << endl;
        
        if (Mydecoder.has_finished_decoding() && Mydecoder.get_current_generation_id() != decoded_generation_id) {
            
            cout << "Has finished layer: " <<  Mydecoder.has_finished_decoding()*1 << endl;
            for (int i=0; i<returnval.size(); i++) {
                cout << returnval[i];
            }
            cout << endl << " Generation: " << Mydecoder.get_current_generation_id()*1 << endl << endl;
            
            decoded_generation_id = Mydecoder.get_current_generation_id();
            
        }
        
        returnval = Mydecoder.decode(header,*received_letter);
        
    }
        
        
        
    

    po.closeConnection();
    
    return 0;
}
