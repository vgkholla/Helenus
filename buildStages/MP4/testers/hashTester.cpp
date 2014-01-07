#include <iostream>
#include <string>
#include "../headers/Hash.h"

using namespace std;

int main() {
	string a = "5000";
	long int b = 4000;
	for(int i = 0; i < 10; i++) {
		cout<<Hash::calculateKeyHash(a)<<endl;
	}
	for(int i = 0; i < 10; i++) {
		cout<<Hash::calculateKeyHash(a)<<endl;
	}
}

