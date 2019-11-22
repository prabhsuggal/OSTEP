#include<stdc++.h>

using namespace std;

bool test(const string &slot)
{
    cout << slot <<" " << slot.substr(0,2) <<" " << slot.substr(2) << endl;
    if(slot.substr(0,2) == "cb"){
        cout << "caught the substr" << endl;
        if(slot.substr(2).find_first_not_of( "0123456789" ) == string::npos){
            return true;
        }
        else{
            return false;
        }
    }
    else if(slot.substr(0,3) == "fpc"){
        if(slot.substr(3).find_first_not_of( "0123456789"  ) == string::npos){
            uint8_t slotId = stoi(slot.substr(3));
            printf("FPC slot is %d\n",slotId);
            return true;
        }
        else{
            return false;
        }
    }
    printf("at the end\n");
    return false;
}

int main(int argc, char* argv[]){
    
    assert(argc==2);
    string s = argv[1];
    if(test(s))
        printf("its is working\n");
    else
        printf("not working\n");

    return 0;

}


