#include <iostream>
#include <fstream>
#include "dataset.h"
#include "functions.h"
#include "apriori.h"

int main(int argc, char* argv[]){
    size_t trans_len=TRANS_LEN;

    ifstream file ("mnist_train_25.dat",ios::binary);

    if (!file.is_open()){
        cout<<"Failed to open file!!"<<endl;
        exit(-1);
    }

    istreambuf_iterator<etype> begin(file), end;
    dtype dat(begin,end);
    file.close();
    Dataset D(dat,trans_len);

    cout<<"The 5th byte is ";
    for (int i=7;i>=0;i--) cout<< ((D[0][4]>>i)&1);
    cout<<endl;


    cout<<"The 2nd transaction: {";
    auto items=D[1].items();
    for(auto i=items.begin();i<items.end();i++) {cout<<" ";cout<<*i;cout<<" ";}
    cout<<"}"<<endl;


    cout<<"Length of the dataset = "<<D.get_length()<<endl;

    // Create a pattern and compute its support
    Itemset I1(trans_len);
    I1.add_item(66);
    I1.add_item(67);
    cout<<"The itemset: {";
    items=I1.items();
    for(auto i=items.begin();i<items.end();i++) {cout<<" ";cout<<*i;cout<<" ";}
    cout<<"}"<<endl;

    unsigned sup = compute_support(I1,D);
    cout<<"The support of I1 is "<<sup<<endl;

    // Create a second patter
    Itemset I2(trans_len);
    I2.add_item(145);
    I2.add_item(155);
    I2.add_item(146);
    
    // Create the Patterns set and add the patterns
    Dataset P(trans_len);
    P.push_back(I1);P.push_back(I2);

    auto supports=compute_support(P,D);
    cout<<"The supports are "<<supports[0]<<", "<< supports[1]<<endl;

    cout<<"Number of items in I2 "<<I2.count_items()<<endl;
    cout<<"The itemset: {";
    items=I2.items();
    for(auto i=items.begin();i<items.end();i++) {cout<<" ";cout<<*i;cout<<" ";}
    cout<<"}"<<endl;

    Dataset C1(trans_len);
    Apriori apriori(&C1);
    apriori.extend_tree();
    cout<<"Length of C1 = "<<C1.get_length()<<endl;
    items=C1[196].items();
    cout<<"The itemset: {";
    for(auto i=items.begin();i<items.end();i++) {cout<<" ";cout<<*i;cout<<" ";}
    cout<<"}"<<endl;
    // // C version (For Neehal and Shoron)
    // vector<unsigned int> supports(P.get_length(),0);
    // compute_support(P.get_data(), P.get_length(),
    //  D.get_data(), D.get_length(), trans_len, supports.data());
    // cout<<"The supports are "<<supports[0]<<", "<< supports[1]<<endl;

    return 0;
}

