#pragma once
#ifndef __HUFFMAN_ENCODER_COMPRESS_H__
#define __HUFFMAN_ENCODER_COMPRESS_H__
#pragma warning(disable:4996)
#pragma warning(disable:4716)
#include <string>
using std::string;


constexpr int _CODE_NUM = 256;			// 码数 
constexpr int _ZIP_NAME_LEN = 16;		// 压缩识别符长度 
constexpr int _FILE_NAME_LEN = 256;		//  文件名长度 
constexpr int _FILE_SIZE_LEN = 8; 		// 文件大小值长度 
constexpr int _CODE_FREQUENCY_LEN = 8;	//  码频值长度 
constexpr int _ZLZIP_HEAD_LEN = _ZIP_NAME_LEN + _FILE_NAME_LEN + _FILE_SIZE_LEN 	// 头文件信息长度 
+ _CODE_FREQUENCY_LEN * _CODE_NUM;
static const char _ZIP_NAME[_ZIP_NAME_LEN] = "txt";	// 压缩 识别符 


// 编码类 
struct Code {
	unsigned char oldCode;				// 旧码 
	unsigned long long frequency;		// 频率 
	unsigned long long newCode;			// 新码 
	string newCodeStr;					// 新码字符串表示 debug 
	int length;							// 新码长度 

	Code() : oldCode(0), frequency(0), newCode(0), length(0) { }
};


// Huffman 节点 
struct HuffmanTreeNode {

	unsigned long long weight;			// 权重 
	Code* codePtr; 						// 指向码（叶子节点才有指向） 
	HuffmanTreeNode* left;				// 左分支 
	HuffmanTreeNode* right;				// 右分支 


	HuffmanTreeNode(unsigned long long w = 0, Code* cp = nullptr,
		HuffmanTreeNode* l = nullptr, HuffmanTreeNode* r = nullptr) :
		weight(w), codePtr(cp), left(l), right(r)
	{ }
};


// 自定义比较器，用于优先队列 
class CmparatorOfHuffmanTreeNode {
public:
	bool operator() (HuffmanTreeNode*& lhs, HuffmanTreeNode*& rhs) const {
		return lhs->weight > rhs->weight;
	}
};


class HuffmanEncoderCompress {

private:

	Code _codes[_CODE_NUM];		// 码 
	bool _isCompress; 				//是否压缩（用于多次压缩 ） 
	string _inputFileName;			//输入文件名 
	string _outputFileName;			//输出文件名 

	unsigned long long _inputFileSize;		//输入文件大小 
	unsigned long long _outputFileSize;		//输出文件大小 

	HuffmanTreeNode* _huffmanTreeRoot;		//Huffman树根节点 


private:
	// 释放节点 
	int freeNode(HuffmanTreeNode* np);

public:
	// 构造 
	HuffmanEncoderCompress(const string& inputFileName, bool isCompress = false);

	// 析构 
	~HuffmanEncoderCompress();

	// 无法拷贝构造 
	HuffmanEncoderCompress(const HuffmanEncoderCompress& hec) = delete;

	// 运行 
	void run();

	// 统计频率 
	void statisticalFrequency();

	// 构建 Huffman 树 
	HuffmanTreeNode* buildHuffmanTree();

	// 获得 Huffman 编码 
	void getNewCodes(HuffmanTreeNode* np, unsigned long long newCode, string newCodeStr, int length);

	// 压缩文件 
	void compress();

	// 搜索节点，辅助于解码 
	bool findNode(HuffmanTreeNode*& np, unsigned char inputByte, int& pos);

	// 解压缩 
	void uncompress();

	// 打印哈夫曼编码信息 
	void printHuffmanEncodeInfo();

	// 打印压缩或解压缩信息 
	void printInfo(const char* type);

	// 获取输出文件名 
	string getOutputFileName() const { return _outputFileName; }

	
};


#endif