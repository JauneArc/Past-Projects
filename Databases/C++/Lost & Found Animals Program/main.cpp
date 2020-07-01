// main.cpp - Driver for Pets DB program

#include <iostream>
#include "ass2.h"
using namespace std;

char Menu();

int main()
{
	bool Quit=false;
	while(!Quit){
		switch(Menu()){
			case 'r':
				ReadFile();
				break;
			case 'd':
				DisplayRecs();
				break;
			case 'a':
				AddRecord();
				break;
			case 's':
				SearchArray();
				break;
			case 'q':
				Quit = true;
				break;
			default:
				cout << "Invalid command!\n";
		}
	}
	cout << "\nThank you for using the Lost and Found Pet Database\n";
	CleanUp();
	return 0;
}

char Menu(){
  char Cmd;
  cout << "*********************************"<< endl;
  cout << "*  Lost and Found Pet Database  *"<< endl;
  cout << "*     (r)ead data file          *"<< endl;
  cout << "*     (d)isplay records         *"<< endl;
  cout << "*     (a)dd record to DB        *"<< endl;
  cout << "*     (s)earch records          *"<< endl;
  cout << "*     (q)uit                    *"<< endl;
  cout << "*********************************"<< endl;
  cout << "Command > ";
  cin  >> Cmd;
  cin.ignore();
  return  Cmd;
}


