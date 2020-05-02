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
    Dataset D(trans_len);
    D.swap_data(dat);

    vector<size_t> items;


    cout<<"The 5th byte is ";
    for (int i=7;i>=0;i--) cout<< ((D[0][4]>>i)&1);
    cout<<endl;


    cout<<"The 2nd transaction: {";
    items=D[1].items();
    for(auto i=items.begin();i<items.end();i++) {cout<<" ";cout<<*i;cout<<" ";}
    cout<<"}"<<endl;


    cout<<"Length of the dataset = "<<D.get_length()<<endl;

    // Create a pattern and compute its support
    Itemset I1(trans_len);
    I1.add_item(66);
    I1.add_item(71);
    I1.add_item(75);
    I1.add_item(76);
    cout<<"The itemset: {";
    items=I1.items();
    for(auto i=items.begin();i<items.end();i++) {cout<<" ";cout<<*i;cout<<" ";}
    cout<<"}"<<endl;

    unsigned sup = compute_support(I1,D);
    cout<<"The support of I1 is "<<sup<<endl;

    // Create a second patter
    Itemset I2(trans_len);
    I2.add_item(66);
    I2.add_item(71);
    I2.add_item(146);
    I2.add_item(75);
    
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

    cout<<"Match I1 to I1 = "<<I1.match_start(I1)<<endl;
    cout<<"Match I2 to I2 = "<<I2.match_start(I2)<<endl;
    cout<<"Match I1 to I2 = "<<I1.match_start(I2)<<endl;


    // Apriori apriori(trans_len, 1000);

    // //C1
    // apriori.extend_tree();
    // cout<<"Length of C1 = "<<apriori.patterns.get_length()<<endl;
    // items=apriori.patterns[199].items();
    // cout<<"The itemset: {";
    // for(auto i=items.begin();i<items.end();i++) {cout<<" ";cout<<*i;cout<<" ";}
    // cout<<"}"<<endl;
    
    // compute_support(apriori.patterns.get_data(), apriori.patterns.get_length(),
    //  D.get_data(), D.get_length(), trans_len, apriori.supports.data());
    // cout<<"Support of 133 is "<<apriori.supports[133]<<endl;
    // cout<<"Size of P = " << apriori.patterns.get_length()<<endl;

    // apriori.remove_infrequent();
    // cout<<"After infrequent removal size of S = " << apriori.supports.size()<<endl;
    
    // //C2
    // apriori.extend_tree();
    // cout<<"Length of C2 = "<<apriori.patterns.get_length()<<endl;
    // cout<<"Length of C2 = "<<apriori.supports.size()<<endl;
    // items=apriori.patterns[199].items();
    // cout<<"The itemset: {";
    // for(auto i=items.begin();i<items.end();i++) {cout<<" ";cout<<*i;cout<<" ";}
    // cout<<"}"<<endl;

    // compute_support(apriori.patterns.get_data(), apriori.patterns.get_length(),
    //  D.get_data(), D.get_length(), trans_len, apriori.supports.data());
    // cout<<"Support of 133 is "<<apriori.supports[133]<<endl;
    // cout<<"Size of P = " << apriori.patterns.get_length()<<endl;

    // apriori.remove_infrequent();
    // cout<<"After infrequent removal size of S = " << apriori.supports.size()<<endl;

    // // C version (For Neehal and Shoron)
    // vector<unsigned int> supports(P.get_length(),0);
    // compute_support(P.get_data(), P.get_length(),
    //  D.get_data(), D.get_length(), trans_len, S.data());
    // cout<<"The supports are "<<supports[0]<<", "<< supports[1]<<endl;

    return 0;
}

