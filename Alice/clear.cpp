#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#include "clear.h"

using namespace std;


bool clearfile(string filename)
{
	int filefd = open(filename.c_str(), O_RDWR, 0);
	if(filefd == 0)
	{
		printf("clear.c, clearfile(string filename): fail to open %s\n", filename.c_str());
		return false;
	}
	int filelen = lseek(filefd, 0, SEEK_END);
	unsigned char *filedata = (unsigned char *)mmap(NULL, filelen, PROT_READ | PROT_WRITE, MAP_SHARED, filefd, 0);
	close(filefd);
	
	memset(filedata, 0, filelen);
	msync(filedata, filelen, MS_SYNC | MS_INVALIDATE);
	munmap(filedata, filelen);
	
	remove(filename.c_str());

	return true;
}


