#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "file.h"

using namespace std;

File::File()
{
	filedata = NULL;
	filelen = 0;
}

File::~File()
{
	if(filedata != NULL)
	{
		free(filedata);
	}
}

//读取文件
bool File::Read_File(string filename)
{
	this->filename = filename;
	FILE *fp = fopen(this->filename.c_str(), "r");
	if(fp == NULL)
	{
		printf("File::Read_File(string filename): Read file fail!!! %m\n");
		return false;
	}
	
	//将文件指针置于文件开头
	rewind(fp);
	
	fseek(fp, 0, SEEK_END);

	//填写文件长度
	filelen = ftell(fp);

	//重置文件指针，置文件开头
	rewind(fp);
	
	//分配文件内存空间
	filedata = (unsigned char *)malloc(filelen);

	if(fread(filedata, 1, filelen, fp) == 0)
	{
		printf("File::Read_File(string filename): Read file to buffer fail!!! %m\n");
		return false;
	}
	
	fclose(fp);
	
	return 0;
}

//返回读取的文件的长度
int File::Get_filelen()
{
	return filelen;
}

//返回文件的内容起始地址
unsigned char *File::Get_filedata()
{
	return filedata;
}

//写文件
bool File::Write_File(string filename, unsigned char *filedata, int filelen)
{
	FILE *fp = fopen(filename.c_str(), "w");
	if(fp == NULL)
	{
		printf("File::Write_File(): fail to open file !!! %m\n");
		return false;
	}
	if(fwrite(filedata, 1, filelen, fp) == 0)
	{
		printf("File::Write_File(): fail to write file !!! %m\n");
		return false;
	}
	fflush(fp);
	fclose(fp);
	return true;
}

bool File::Map_file_to_memory(string filename)
{
	this->filename = filename;
	int filefd = open(this->filename.c_str(), O_RDWR, 0);
	this->filelen = lseek(filefd, 0, SEEK_END);
	this->filedata = (unsigned char *)mmap(NULL, this->filelen, PROT_READ | PROT_WRITE, MAP_SHARED, filefd, 0);
	close(filefd);
	return true;
}

void File::Sync_memory_to_disk()
{
	msync(this->filedata, this->filelen, MS_SYNC | MS_INVALIDATE);
}

void File::Unmap_file_from_memory()
{
	munmap(this->filedata, this->filelen);
}

//删除文件
void File::Remove_file(string filename)
{
	this->filename = filename;
	remove(this->filename.c_str());
}

