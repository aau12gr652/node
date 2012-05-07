#include <iostream>

#include "../postoffice/Postoffice.h"
#include "../decoder/decoder.h"

using namespace std;

int main(){
    char data[100];
    
    vector<uint8_t> returnval(13);
    
    decoder Mydecoder=decoder();
    
    postoffice po("4000");
    
    stamp* header = (stamp*)malloc(sizeof(stamp));
    
    cout << "Receiving" << endl;
    
    for (int i=0; i<100; i++) {
        
        cout << po.receive(data, 100, header);

        returnval = Mydecoder.decode(header,(void*)data);
        
    }

    po.closeConnection();
    
    return 0;
}
