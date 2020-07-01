/*********************************************
 * main.cpp - Contains driver program for class CardSet
 *     DO NOT ALTER THIS FILE
 *********************************************/

#include <iostream>
#include "CardSet.h"
using namespace std;


int main()
{
	CardSet CardSet1(12), CardSet2(12), CardSet3, CardSet4, CardSet5, CardSet6;

	cout << "BEGINNING OF CARDSET TESTS . . .  "<< endl << endl;

	cout << "Testing constructors:" << endl; cout << CardSet::Card;
	cout << "CardSet1 Size: " << CardSet1.Size() << endl;
	cout << "CardSet3 Size: " << CardSet3.Size() << endl;
	cout << "Printout of CardSet1:" << endl;
	CardSet1.Print();
	cout << "Printout of CardSet2:" << endl;
	CardSet2.Print();
	cout << endl;

	cout << "Testing Adding and removing cards to CardSet1: "<< endl;
	int Card = CardSet1.Deal();
	cout << "Dealing 1 card: "<< Card << endl;
	cout << "CardSet1 Size: " << CardSet1.Size() << endl;
	cout << "Adding card "<< Card << endl;
	CardSet1.AddCard(Card);
	cout << "CardSet1 Size: " << CardSet1.Size() << endl;
	cout << "Printout of CardSet1:" << endl;
	CardSet1.Print();
	cout << endl;

	cout << "Testing Normal Shuffle: "<< endl;
	CardSet1.Shuffle();
	cout << "Printout of shuffled CardSet1:" << endl;
	CardSet1.Print();
	cout << endl;

	cout << "Testing Merge Shuffle: "<< endl;
	cout << "Printout of CardSet2:" << endl;
	CardSet2.Print();
	CardSet1.MergeShuffle(CardSet2);
	cout << "Printout of CardSet1 Merged with CardSet2:" << endl;
	CardSet1.Print();
	cout << endl;

	cout << "Testing Dealing 3 Cards from CardSet1 into CardSet3 and CardSet4: " << endl;
	CardSet1.Deal(3,CardSet3,CardSet4);
	cout << "Printout of CardSet1:" << endl;
	CardSet1.Print();
	cout << "Printout of CardSet3:" << endl;
	CardSet3.Print();
	cout << "Printout of CardSet4:" << endl;
	CardSet4.Print();
	cout << endl;

	cout << "Testing Dealing 3 Cards from CardSet1 into CardSet 3, 4, 5 & 6: " << endl;
	CardSet1.Deal(3,CardSet3,CardSet4,CardSet5,CardSet6);
	cout << "Printout of CardSet1:" << endl;
	CardSet1.Print();
	cout << "Printout of CardSet3:" << endl;
	CardSet3.Print();
	cout << "Printout of CardSet4:" << endl;
	CardSet4.Print();
	cout << "Printout of CardSet5:" << endl;
	CardSet5.Print();
	cout << "Printout of CardSet6:" << endl;
	CardSet6.Print();
	cout << endl;

	cout << ". . . END OF TESTS"<< endl;
	return 0;
}
