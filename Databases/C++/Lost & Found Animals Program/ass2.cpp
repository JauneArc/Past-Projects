/***********************************************************************************
 * CSCI 124 - ass2.cpp - Contains function definitions for pet database program
 * Put you name, login and the date last modified here.
 *
 ***********************************************************************************/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <cstring>
using namespace std;

// ============== Constants ==========================================

const char cDataFileName[] = "pets.txt";
const int cMaxRecs = 100;
const int cMaxChars = 30;


// ============= User Defined types ==================================

enum   StatusType{Lost,Found};
enum   PetType{Dog,Cat};
enum   GenderType{Male,Female,Unknown};
struct AgeType{ int Yrs; int Mths;}; // -1 if unknown

struct PetRecord{
	StatusType Status;           // Lost or Found
	PetType Type;                // Dog or Cat
	GenderType Gender;           // Male, Female or Unknown
	char Breed[cMaxChars];       // Bread of dog or cat. eg Bull terrier
	AgeType Age;                 // Age in years and mths. (-1 -1 if unknown)
	char Colour[cMaxChars];      // Colour of pet. eg tan
	char Location[cMaxChars];    // Suburb where found or lost. eg Mt Keira
	char PhoneNo[cMaxChars];     // Ph. of person to contact
};


// ============= Global Data =========================================

PetRecord *gPetRecs[cMaxRecs];
int gNumRecs=0;


// ============= Private Function Prototypes =========================

void DisplayRecord(int i); // Displays record i on screen


// ============= Public Function Definitions =========================

void ReadFile()//DONE
{
	ifstream fin;
	fin.open(cDataFileName);
	if (!fin.good())
	{
		cerr << "Could not open file!\n";
		exit(1);
	}
	gNumRecs=0;
	int i;
	for(i=0;i<cMaxRecs;i++)
	{
		char Tmp[cMaxChars];
		fin >> Tmp;
		if(fin.fail())break;
		gPetRecs[i] = new PetRecord;
		switch(Tmp[0])
		{
			case'l': gPetRecs[i]->Status = Lost; break;
			case'f': gPetRecs[i]->Status = Found;break;
		}
		fin >> Tmp;
		switch(Tmp[0]){
			case'c': gPetRecs[i]->Type = Cat; break;
			case'd': gPetRecs[i]->Type = Dog; break;
		}
		fin >> Tmp;
		switch(Tmp[0])
		{
			case'm': gPetRecs[i]->Gender = Male;    break;
			case'f': gPetRecs[i]->Gender = Female;  break;
			default: gPetRecs[i]->Gender = Unknown; break;
		}
		fin.ignore(); //eat tailing '\n'
		fin.getline(gPetRecs[i]->Breed,cMaxChars);
		fin >> gPetRecs[i]->Age.Yrs>>gPetRecs[i]->Age.Mths;
		fin.ignore(); //eat tailing '\n'
		fin.getline(gPetRecs[i]->Colour,cMaxChars);
		fin.getline(gPetRecs[i]->Location,cMaxChars);
		fin.getline(gPetRecs[i]->PhoneNo,cMaxChars);
	}
	gNumRecs=i;
	fin.close();
	cout<< "\nThere are "<< gNumRecs <<" records in the Lost and Found Pet Database\n";
}

void DisplayRecs()//DONE
{// Displays records one at a time
	for(int i=0;i<gNumRecs;i++)
	{
		DisplayRecord(i);
		cout<<"Display next record (y/n)> ";
		char Ans;
		cin>>Ans;
		cout<<endl;
		if(Ans=='n') return;
	}
}

void AddRecord()//DONE
{// Adds a new record to data file
    ofstream fout(cDataFileName, ios::app);
    int i =0;
    if(gNumRecs == cMaxRecs)
    {
        cout<< "Maximum number of records in the array"<<endl;
        return;
    }
    //modify readfile for this
    i = gNumRecs+1;
    char Tmp[cMaxChars];
    char Temp;

    gPetRecs[i] = new PetRecord;
    cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    cout << "+ Add Record to Database +"<< endl;
    cout << "+ (Enter your pets details) +" << endl;
    cout << "+ (Enter ? or -1 if detail is unknown) +" << endl;
    cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    cout << "Is the pet lost or found? (l/f) => ";
    cin >> Tmp;

    switch(Tmp[0])
    {
        case'l': gPetRecs[i]->Status = Lost; break;
        case'f': gPetRecs[i]->Status = Found;break;
    }

    cout <<"Is the pet a dog or a cat? (m/f) =>";
    cin >> Tmp;

    switch(Tmp[0])
    {
        case'c': gPetRecs[i]->Type = Cat; break;
        case'd': gPetRecs[i]->Type = Dog; break;
    }

    cout <<"What gender is the pet? =>";
    cin >> Tmp;
		switch(Tmp[0])
		{
			case'm': gPetRecs[i]->Gender = Male;    break;
			case'f': gPetRecs[i]->Gender = Female;  break;
			default: gPetRecs[i]->Gender = Unknown; break;
		}
		cin.ignore(); //eat tailing '\n'
		cout <<"What breed is the pet? =>";
		cin.getline(gPetRecs[i]->Breed,cMaxChars);
		cout <<"What age is the pet? (yy mm ) =>";
		cin >> gPetRecs[i]->Age.Yrs>>gPetRecs[i]->Age.Mths;
		cin.ignore(); //eat tailing '\n'
		cout <<"What colour is the pet? =>";
		cin.getline(gPetRecs[i]->Colour,cMaxChars);
		cout <<"In what suburb was the pet lost? =>";
		cin.getline(gPetRecs[i]->Location,cMaxChars);
		cout <<"What is your phone number? =>";
		cin.getline(gPetRecs[i]->PhoneNo,cMaxChars);

    //Save to file
    fout << tolower(gPetRecs[i]->Status);
    fout << tolower(gPetRecs[i]->Type);
    fout << tolower(gPetRecs[i]->Gender);
    for(int x =0; x <cMaxChars; x++)
    {
        Temp = gPetRecs[i]->Breed[x];
        Temp = tolower(Temp);
        gPetRecs[i]->Breed[x] = Temp;
    }
    fout << gPetRecs[i]->Breed;
    fout << gPetRecs[i]->Age.Yrs<< " " <<gPetRecs[i]->Age.Mths;
    for(int x =0; x <cMaxChars; x++)
    {
        Temp = gPetRecs[i]->Colour[x];
        Temp = tolower(Temp);
        gPetRecs[i]->Colour[x] = Temp;
    }
    fout << gPetRecs[i]->Colour;
    for(int x =0; x <cMaxChars; x++)
    {
        Temp = gPetRecs[i]->Location[x];
        Temp = tolower(Temp);
        gPetRecs[i]->Location[x] = Temp;
    }
    fout << gPetRecs[i]->Location;
    fout << gPetRecs[i]->PhoneNo;
    fout.close();
    return ;
}

void SearchArray()//Current
{
    char Tmp[cMaxChars];
    char Temp;
    int tmpi, tmpi2;
    cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    cout << "+ Search Database +" << endl;
    cout << "+ (Enter your pets details) +" << endl;
    cout << "+ (Enter ? or -1 if detail is unknown) +" << endl;
    cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    cout << "Search lost or found pets? (l/f) =>";
    cin >> Tmp;
    switch(Tmp[0])
    {
        case'l': tmpi = 0; break;
        case'f': tmpi = 1;break;
    }
    for(int i = 0; i < gNumRecs+1; i++)
    {
        if(tmpi != gPetRecs[i]->Status)
        {
            delete gPetRecs[i];
            gNumRecs--;
        }
    }

    cout << "Search for a dog or cat? (d/c) =>";
    cin >> Tmp;
    switch(Tmp[0])
    {
        case'c': tmpi =0; break;
        case'd': tmpi =1; break;
    }
    for(int i = 0; i < gNumRecs+1; i++)
    {
        if(tmpi != gPetRecs[i]->Type)
        {
            delete gPetRecs[i];
            gNumRecs--;
        }
    }

    cout << "What gender is the pet? =>";
    cin >> Tmp;
    switch(Tmp[0])
		{
			case'm': tmpi = 0; break;
			case'f': tmpi = 1; break;
			default: tmpi = 2; break;
		}
    for(int i = 0; i < gNumRecs+1; i++)
    {
        if(tmpi != gPetRecs[i]->Gender)
        {
            delete gPetRecs[i];
            gNumRecs--;
        }
    }

    cout << "What breed to search for? =>";
    cin >> Tmp;
    for(int x =0; x <cMaxChars; x++)
    {
        Temp = Tmp[x];
        Temp = tolower(Temp);
        Tmp[x] = Temp;
    }
    for(int i = 0; i < gNumRecs+1; i++)
    {
        if(strcmp(Tmp, gPetRecs[i]->Breed) != 0)
        {
            delete gPetRecs[i];
            gNumRecs--;
        }
    }

    cout << "What age to search for? (yy mm ) =>";
    cin >> tmpi >> tmpi2;
    for(int i = 0; i < gNumRecs+1; i++)
    {
        if(tmpi != -1)
        {
        if(tmpi != gPetRecs[i]->Age.Yrs)
        {
            delete gPetRecs[i];
            gNumRecs--;
        }
        }
        if(tmpi2 != -1)
        {
        if(tmpi2 != gPetRecs[i]->Age.Mths)
        {
            delete gPetRecs[i];
            gNumRecs--;
        }
        }
    }

    cout << "What colour to search for? =>";
    cin >> Tmp;
    if(strcmp(Tmp, "?") != 0)
    {
    for(int x =0; x <cMaxChars; x++)
    {
        Temp = Tmp[x];
        Temp = tolower(Temp);
        Tmp[x] = Temp;
    }
    for(int i = 0; i < gNumRecs+1; i++)
    {
        if(strcmp(Tmp, gPetRecs[i]->Colour) != 0)
        {
            delete gPetRecs[i];
            gNumRecs--;
        }
    }
    }
    cout << "What suburb was the pet found? =>";
    cin >> Tmp;
    if(strcmp(Tmp, "?") != 0)
    {
    for(int x =0; x <cMaxChars; x++)
    {
        Temp = Tmp[x];
        Temp = tolower(Temp);
        Tmp[x] = Temp;
    }
    for(int i = 0; i < gNumRecs+1; i++)
    {
        if(strcmp(Tmp, gPetRecs[i]->Location) != 0)
        {
            delete gPetRecs[i];
            gNumRecs--;
        }
    }
    }
    cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
    cout << "+ There are" << gNumRecs << "pets in the database +" << endl;
    cout << "+ that match the search criteria +" << endl;
    cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;

    return;
}

void CleanUp()//DONE
{// Deletes all dynamic data in gPetRecs array
    for(int i =0; i<gNumRecs+1;i++)
    {
        delete gPetRecs[i];
    }
}

// ============= Private Functions Definitions =========================



void DisplayRecord(int i)//DONE
{// Displays record i on screen
	cout<<"Status:        ";
	switch(gPetRecs[i]->Status)
	{
		case Lost:  cout << "lost\n"; break;
		case Found: cout << "found\n";break;
	}
	cout<<"Type  :        ";

	switch(gPetRecs[i]->Type)
	{
		case Cat: cout << "cat\n";break;
		case Dog: cout << "dog\n";break;
	}
	cout<<"Gender:        ";

	switch(gPetRecs[i]->Gender)
	{
		case Male:   cout << "male\n";  break;
		case Female: cout << "female\n";break;
		default: cout << "unknown\n"; break;
	}
	cout<<"Breed:         "<<gPetRecs[i]->Breed<<endl;
	cout<<"Age:           "<<gPetRecs[i]->Age.Yrs<<" Yrs "<<gPetRecs[i]->Age.Mths<<" Mths\n";
	cout<<"Colour:        "<<gPetRecs[i]->Colour<<endl;
	cout<<"Location:      "<<gPetRecs[i]->Location<<endl;
	cout<<"Phone No:      "<<gPetRecs[i]->PhoneNo<<endl;
}


