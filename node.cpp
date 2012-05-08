#include <iostream>

#include "../postoffice/Postoffice.h"
#include "decoder.h"

using namespace std;

int main(){
    char data[100];
    
    vector<uint8_t> returnval(13);
    
    decoder Mydecoder=decoder();
    
    postoffice po("4000");
    
    stamp* header = (stamp*)malloc(sizeof(stamp));
    
    cout << "Receiving" << endl;
    
    int decoded_generation_id=0;
    
    while (1) {
        po.receive(data, 100, header);
        
        
        if (Mydecoder.has_finished_decoding() && Mydecoder.get_current_generation_id() != decoded_generation_id) {
            
            cout << "Has finished layer: " <<  Mydecoder.has_finished_decoding()*1 << endl;
            for (int i=0; i<returnval.size(); i++) {
                cout << returnval[i];
            }
            cout << " Generation: " << Mydecoder.get_current_generation_id()*1 << endl;
            
            decoded_generation_id = Mydecoder.get_current_generation_id();
            
        }
        returnval = Mydecoder.decode(header,(void*)data);
    }
        
        
        
    

    po.closeConnection();
    
    return 0;
}
