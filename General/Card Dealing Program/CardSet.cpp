/**********************************************
* CardSet.cpp - Contains implementation of class CardSet
* <Liam Conway, 4066716, ljc878, 14/9/2012, etc>
***********************************************/

#include <iostream>
#include <cstdlib>
#include "CardSet.h"
using namespace std;

CardSet::CardSet() //
{//sets up a set of 0 cards
     Card = NULL;
     nCards =0;
}
CardSet::CardSet(int n) //
{   //sets up a set of cards based on the number passed to it(never exceeding 51)
     int *Card = new int[n];
     nCards = n;
     for( static int i =0; i < n; i++)
        Card[i] = i % 52;
}
CardSet::~CardSet() //Done
{//cleans up dynamic memory
     delete [] Card;
}
int CardSet::Size() const //DONE
{
     return nCards;
//returns the size of nCards
}
bool CardSet::IsEmpty() const //DONE
{//returns true if the set is empty
     if(nCards == 0)
     return true;
     return false;
}
void CardSet::Shuffle()//see messages webpage
{
//rearanges set cards in a random manner
     int n = Size();
     for(int i =0; i < n; i++)
     {
      int j = (rand()%nCards)+1;
      if(i != j)
	{
	 int Tmp = Card[i];
	 Card[j] = Card[i];
	 Card[i] = Tmp;
	}	

     }
}
int CardSet::Deal() //TO DO
{
        if(nCards==0)
        {
                cerr<<"Error: No cards left!\n";
                exit(1);
        }
        int Tmp = Card[0];
        nCards--;
        int *NewCards = new int[nCards];
        for(int i=0;i<nCards;i++)
                NewCards[i] = Card[i+1];
        
        delete [] Card;
        Card = NewCards;
        return Tmp;
}
void CardSet::Deal(int n,CardSet& C1,CardSet& C2)//see messages webpage
{
        if(nCards < n*2){
                cerr<<"Error: Not enough cards to deal out!\n";
                exit(1);
        }
        for(int i=0; i<n*2; )
        {
                C1.AddCard(Card[i++]);
                C2.AddCard(Card[i++]);
        }
        nCards = nCards - n*2;
        int *NewCard = new int[nCards];
        for(int i=0,j = n * 2; i<nCards; i++,j++)
        {
     	NewCard[i] = Card[j];
        }
        delete [] Card;
        Card = NewCard;

}
void CardSet::Deal(int n,CardSet& C1,CardSet& C2,CardSet& C3,CardSet& C4) //lika Deal() above
{
        if(nCards < n*4){
                cerr<<"Error: Not enough cards to deal out!\n";
                exit(1);
        }
        for(int i=0; i<n*4; )
        {
                C1.AddCard(Card[i++]);
                C2.AddCard(Card[i++]);
                C3.AddCard(Card[i++]);
                C4.AddCard(Card[i++]);
        }
        nCards = nCards - n*4;
        int *NewCard = new int[nCards];
        for(int i=0,j=n*4; i<nCards; i++,j++)
        {
     	NewCard[i] = Card[j];
        delete [] Card;
        Card = NewCard;
        }
}

void CardSet::MergeShuffle(CardSet & C) //Done
{
     int *NewCards = new int[nCards+1];
//you have to create new memory array big enough for all cards
//put cards from both decks into the new memory alternately
//delete old memory
//set Card to new memory
//set nCards to total no of cards

//Be careful putting cards into new memory...this works...

     for(int i=0;i < nCards+1;)
     {
         if(!IsEmpty()) NewCards[i++]= Deal();
         if(!C.IsEmpty()) NewCards[i++]= C.Deal();
         if(IsEmpty() && C.IsEmpty()) break;
     }
     delete [] Card;
     Card = NewCards;
}
void CardSet::AddCard(int CardValue) //its a bit like deal()
 {
          nCards++;
          int *NewCards = new int[nCards];
         NewCards[0] = CardValue;
         for(int i=1;i<nCards;i++)
                  NewCards[i] = Card[i-1];
          
          delete [] Card;
          Card = NewCards;
 }

 void CardSet::Print() const //like this...
 {
       for(int i=0;i<nCards;i++)
       {  
           PrintCard(Card[i]);
           if(i%5==4) cout<<endl; else cout<<' ';
       }
 }




// Private function to print out usual representation of playing card.
// Input is integer from 0 to 51.  There is no error checking.

void CardSet::PrintCard(int c) const
{
	int Rank = c%13;
	int Suit = c/13;
	const char NameSuit[5] = "SCDH";
	const char NameRank[14] = "23456789XJQKA";
	cout << NameRank[Rank] << NameSuit[Suit];
}
