/****************************************************************** 

** 文件名: Buffer.h

** Copyright (c) 

** 创建人: 谭东亮

** 日  期: 2018-6-3

** 描  述: MiniSQL buffer模块所有类和结构
           通过该模块实现物理文件的写入和删除
** 版  本: 1.00

******************************************************************/

#ifndef _BUFFER_H_
#define _BUFFER_H_
#include <iostream>
#include <vector>
#include <string>
#include <vector>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define FILE_PAGESIZE		4096	// 内存页(==文件页)大小
#define MEM_PAGEAMOUNT		1024	// 内存页数量
#define MAX_FILENAME_LEN    256		// 文件名（包含路径）最大长度

class Clock;
Clock* GetGlobalClock();

/*********************************************************
*             页头信息，用以标识文件页
**********************************************************/
class PAGEHEAD
{
public:
	void Initialize();
	unsigned long pageId;		// 页编号
	bool isFixed;				// 页是否常驻内存
};


/*********************************************************
*             文件地址
**********************************************************/
class FileAddr
{
	friend class FILECOND;
public:
	FileAddr() :filePageID(0), offSet(sizeof(PAGEHEAD)) {}
	//void SetFileAddr();
	FileAddr(unsigned long _filePageID, unsigned int  _offSet) :filePageID(_filePageID), offSet(_offSet) {}
public:
	unsigned long filePageID;     // 页编号
	unsigned int  offSet;         // 页内偏移量
};


/*********************************************************
*               文件头信息
**********************************************************/
class FILECOND
{
public:
	void Initialize();
	FILECOND()=default;
	FileAddr DelFirst;                // 第一条被删除记录地址
	FileAddr DelLast;                 // 最后一条被删除记录地址  
	FileAddr NewInsert;               // 文件末尾可插入新数据的地址
	unsigned long total_page;         // 目前文件中共有页数
};

/*********************************************************
*
*   名称：内存页类
*   功能：提供保存文件页的空间，以及该页相关的信息
*   约束：内存页的大小固定
*
**********************************************************/
class MemPage
{
public:
	MemPage();
	~MemPage();

	// 把内存中的页写回到文件中
	void Back2File() const;
	// 设置为脏页
	bool SetModified();
	std::string MemPageInfo;
public:
	unsigned long fileId;         // 文件指针，while fileId==0 时为被抛弃的页
	unsigned long filePageID;     // 文件页号
	bool bIsLastUsed;             // 最近一次访问内存是否被使用，用于Clock算法
	bool isModified;              // 是否脏页
	void *Ptr2PageBegin;          // 实际保存物理文件数据的地址
	PAGEHEAD *pageHead;           // 页头指针
	FILECOND* GetFileCond();      // 文件头指针（while filePageID == 0）
};

/*********************************************************
*
*   名称：内存页管理类（Clock页面置换算法）
*   功能：物理页面在内存中的缓存，加速对物理文件的读写
*   约束：调用者保证需要被载入的物理文件都存在，且加载的页面不越界
*
**********************************************************/
class Clock
{
	friend class MemFile;
public:
	Clock();
	~Clock();
	// 返回磁盘文件内存地址
	MemPage* GetMemAddr(unsigned long fileId, unsigned long filePageID);

	// 创建新页，适用于创建新文件或者添加新页的情况下
	MemPage* CreatNewPage(unsigned long fileId, unsigned long filePageID);

private:
	// return the file page memory address if it is in memory
	// otherwise return nullptr;
	MemPage* GetExistedPage(unsigned long fileId, unsigned long filePageID);
	MemPage* LoadFromFile(unsigned long fileId, unsigned long filePageID);

	// 返回一个可替换的内存页索引
	// 原页面内容该写回先写回
	unsigned int GetReplaceablePage();  
private:
	MemPage* MemPages[MEM_PAGEAMOUNT+1];  // 内存页对象数组
};

/*********************************************************
*   名称：内存文件类
*   功能：通过物理文件在内存中的映射文件的操作，从而操作物理文件
*   约束：假设所有被操作的文件都存在且已经打开
**********************************************************/
class MemFile
{
	friend class BUFFER;
public:
	// 写入数据
	FileAddr MemWrite(const void* source, size_t length, FileAddr* dest);
	MemPage * AddOnePage();  // 当前文件添加一页空间
private:
	// 构造
	MemFile(const char *file_name, unsigned long file_id);
private:
	
	MemPage* GetFileFirstPage();  //得到文件首页
private:
	char fileName[MAX_FILENAME_LEN];
	unsigned long fileId;             // 文件指针
	unsigned long total_page;         // 目前文件中共有页数
};


class BUFFER
{
public:
	BUFFER() = default;
	~BUFFER();
	MemFile* GetMemFile(const char *fileName);
	void CreateFile(const char *fileName);
	Clock* GetPtr2Clock();
public:
	std::vector<MemFile*> memFile;  // 保存已经打开的文件列表
	Clock MemClock;
};


//FileAddr MemWrite(const void*, size_t, FileAddr*);

#endif //define _BUFFER_H_
