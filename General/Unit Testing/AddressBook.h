/*
name: Liam Conway
Date: 16/8/15
*/

class MyRecord;

typedef Myrecord* RecordPtr;

class AddressBook {
public:
    AddressBook();
    ~AddressBook();
    void add(RecordPtr prec);
    const map<std::string, RecordPtr>& viewData() const {return this->entries;}
    bool keyExists(std::string& id) const;
    RecordPtr getRecord(std::string& id) const;

private:
    map<std::string, RecordPtr> entries;
};

//menu select
#include <iostream>
#include <cstdlib>
#include <regex/boost.hpp>//??
#include 'AddressBook.h'
#include 'MyRecord.h'
#include 'stringtrimmer.h'
using namespace std;
static string getTrimmedString() {};
static void doAddRecord(AddressBook& abook){};
static void doListIds(const AddressBook& abook) {};
static void doDisplayRecords(const AddressBook& abook) {};
static void consumeNewLine () {};


cout << "demo address book application" << endl;
bool continuing = true;
while(continuing){
    cout << "Menu Selection \n 1:Quit \n 2:Add Record \n 3:List Identifiers \n 4 Display Record \n";
    cout << "Select:";
    int choice;
    cin >> choice;
    if(!cin.good()) break;
    consumeNewLine();
    switch(choice){
        case 1: continuing = false; break;
        case 2: doAddRecord(mybook); break;
        case 3: doListIds(mybook); break;
        case 4: doDisplayRecord(mybook);break;

    }

}
return 0;
