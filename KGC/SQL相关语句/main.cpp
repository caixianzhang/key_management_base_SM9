#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string.h>
#include <stdio.h>

#include "sql.h"

using namespace std;

void print(unsigned char *read, int len)
{
	for(int i = 0; i < len ; i++)
	{
		printf("%02X", *(read + i));
	}
}

int main()
{
	MyDB *db = new MyDB();
	PublicKeyMessage *Machine = (PublicKeyMessage *)malloc(sizeof(PublicKeyMessage));
	Machine->MachineID = 11;
	
	memset(Machine->PublicKey, 0xAA, 32);
	memset(Machine->Hash, 0xBB, 32);
	memset(Machine->Sign, 0xCC, 32);
	
	db->Write(Machine);
	
	memset(Machine->PublicKey, 0, 32);
	memset(Machine->Hash, 0, 32);
	memset(Machine->Sign, 0, 32);
	
	db->Search(Machine);
	
	print(Machine->PublicKey, 32);
	cout << endl;
	print(Machine->Hash, 32);
	cout << endl;
	print(Machine->Sign, 32);
	cout << endl;
	
	return 0;
}


