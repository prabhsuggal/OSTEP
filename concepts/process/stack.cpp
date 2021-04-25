#include <iostream>
using namespace std;
int global = 30;

int main()
{
    int local = 10;
    cout << "Hello world " << local << " " << global <<"\n";
    return 0;
}