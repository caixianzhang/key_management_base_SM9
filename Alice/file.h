#ifndef FILE_H_
#define FILE_H_

#include <string>
using namespace std;

class File{
	private:
		//文件名
		string filename;
		
		//16进制形式，文本文件长度
		int filelen;

		unsigned char *filedata;
	public:
		File();
		virtual ~File();

		//读取文件
		bool Read_File(string filename);

		//返回读取的文件的长度
		int Get_filelen();

		//返回文件的内容起始地址
		unsigned char *Get_filedata();

		//写文件
		bool Write_File(string filename, unsigned char *filedata, int filelen);

		bool Map_file_to_memory(string filename);

		void Sync_memory_to_disk();

		void Unmap_file_from_memory();
		
		//删除文件
		void Remove_file(string filename);
		
};
#endif