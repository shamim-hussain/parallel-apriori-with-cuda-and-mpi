#include <iostream>
#include <fstream>
using namespace std;

int main(int argc, char* argv[]){
    ifstream file ("mnist_train_25.dat",ios::binary|ios::ate);
    int file_len;
    char* buffer;

    if (file.is_open()){
        file_len = file.tellg();
        file.seekg(0,ios::beg);
        buffer = new char[file_len];
        file.read(buffer,file_len);
        file.close();
    } else {
        cout<<"Failed to open file!!"<<endl;
    }

    
    cout<<"Length of the file = "<<file_len<<endl;
    return 0;
}