// Office.cpp
// static field holds rent due date for an office -
// rents are due on the 1st
#include<iostream>
#include<string>
using namespace std;
class Office
{
  private:
   int officeNum;
   string tenant;
   int rent;
   static const int rentDueDate =1;
  public:
    void setOfficeData(int, string, int);
    static void showRentDueDate();
    void showOffice();
};
//const int rentDueDate = 1;

void Office::setOfficeData(int num, string occupant, int rent)
 {
   officeNum = num;
   this->tenant = occupant;
   Office::rent = rent;
 }
void Office::showOffice()
{
  cout<<"Office "<< officeNum <<" is occupied by " << tenant <<endl;
  cout<<"The rent $"<< rent <<" is due on day "<< rentDueDate <<" of the month"<<endl;
  cout<<"because ALL rents are due on day "<<  rentDueDate <<" of the month"<<endl;
}
void Office::showRentDueDate()
{
   cout<<"All rents are due on day "<< rentDueDate <<" of the month"<<endl;
}
int main()
{
     Office::showRentDueDate();
     Office myOffice;
     myOffice.setOfficeData(234, "Dr. Smith", 450);
     myOffice.showOffice();
}

