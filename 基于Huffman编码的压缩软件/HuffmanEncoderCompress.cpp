#include "HuffmanEncoderCompress.h"
#include<iostream>
#include <cstdio>
#include <cstring>
#include <queue>
using namespace std;


HuffmanEncoderCompress::HuffmanEncoderCompress(const string& inputFileName, bool isCompress) :
	_isCompress(isCompress),
	_inputFileName(inputFileName), _outputFileName(inputFileName + ".txt"),
	_inputFileSize(0), _outputFileSize(0), _huffmanTreeRoot(nullptr)
{
	for (int i = 0; i < _CODE_NUM; ++i) _codes[i].oldCode = i;
}


// 释放节点及其子树 
int HuffmanEncoderCompress::freeNode(HuffmanTreeNode* np) {
	if (np) {
		freeNode(np->left);
		freeNode(np->right);
		delete np;
	}
}

// 析构 
HuffmanEncoderCompress::~HuffmanEncoderCompress() {
	freeNode(_huffmanTreeRoot);
}


// 运行 
void HuffmanEncoderCompress::run() {

	FILE* inputFp = NULL;
	if ((inputFp = fopen(_inputFileName.c_str(), "rb")) == NULL) {
		printf("open file %s failed!\n", _inputFileName.c_str());
		exit(-1);
	}

	char zipName[_ZIP_NAME_LEN];
	fread(zipName, _ZIP_NAME_LEN, 1, inputFp);
	void Look();

	if (_isCompress || strcmp(zipName, _ZIP_NAME)) {		// 无识别符，非压缩文件 

		fclose(inputFp);
		printf("开始压缩文件%s......\n", _inputFileName.c_str());
		printf("正在统计频率......\n");
		statisticalFrequency();			// 统计频率 
		printf("正在构建哈夫曼树......\n");
		_huffmanTreeRoot = buildHuffmanTree();		    //构建哈夫曼树 
		printf("正在产生新编码......\n");
		getNewCodes(_huffmanTreeRoot, 0, string(), 0);	// 获得新编码 
		printHuffmanEncodeInfo();
		printf("正在压缩......\n");
		compress();		// 压缩 
		printInfo("压缩");
		printf("压缩成功\n");

	}
	else {		// 有识别符，是压缩文件 

		printf("开始解压缩文件%s......\n", _inputFileName.c_str());
		printf("读取原始文件信息......\n");
		char outputFileName[_FILE_NAME_LEN];
		fread(outputFileName, _FILE_NAME_LEN, 1, inputFp);	// 读入原文件名 
		printf("原始文件名为%s\n", outputFileName);
		_outputFileName = string(outputFileName);
		fread(&_outputFileSize, _FILE_SIZE_LEN, 1, inputFp); 	// 读入原文件大小

		for (int i = 0; i < _CODE_NUM; ++i) 		       // 读入字符频率表 
			fread(&_codes[i].frequency, _CODE_FREQUENCY_LEN, 1, inputFp);

		fclose(inputFp);
		printf("正在构建哈夫曼树......\n");
		_huffmanTreeRoot = buildHuffmanTree();		     //  构建哈夫曼树 
		printf("正在产生新编码......\n");
		getNewCodes(_huffmanTreeRoot, 0, string(), 0);	// 获得新编码
		printHuffmanEncodeInfo();
		printf("正在解压缩......\n");
		uncompress();	// 解压缩 
		printInfo("解压缩");
		printf("解压成功\n");
	}
}

// 统计频率 
void HuffmanEncoderCompress::statisticalFrequency() {
	// 打开文件 fopen=（其功能是使用给定的模式 rb（读取为二进制） 打开_inputFileName.c_str()）
	FILE* inputFp = NULL;
	if ((inputFp = fopen(_inputFileName.c_str(), "rb")) == NULL) {
		printf("open file %s failed!\n", _inputFileName.c_str());
		exit(-1);
	}
	// 统计频率 foef=(文件结束：返回非0值，文件未结束，返回0值)
	while (!feof(inputFp)) {
		unsigned char inputByte;
		fread(&inputByte, 1, 1, inputFp);	//fread=(从给定输入流inputFp读取1个对象到数组inputByte)
		if (feof(inputFp)) break;
		++_codes[inputByte].frequency;
		++_inputFileSize;
	}
	fclose(inputFp);
}


// 构建 Huffman 树 
HuffmanTreeNode* HuffmanEncoderCompress::buildHuffmanTree() {
	// 使用优先队列，自定义比较器 
	priority_queue<HuffmanTreeNode*, vector<HuffmanTreeNode*>, CmparatorOfHuffmanTreeNode> nps;
	// 初始化叶子节点 
	for (int i = 0; i < _CODE_NUM; ++i)
		nps.push(new HuffmanTreeNode(_codes[i].frequency, &_codes[i]));
	// 取最小两个节点合并 
	while (true) {
		HuffmanTreeNode* np1 = nps.top(); nps.pop();
		HuffmanTreeNode* np2 = nps.top(); nps.pop();
		HuffmanTreeNode* np3 = new HuffmanTreeNode(np1->weight + np2->weight, nullptr, np1, np2);
		if (!nps.empty()) nps.push(np3);
		else return np3;	// 优先队列为空则构成 Huffman 树 
	}
}


// 获得 Huffman 编码 
void HuffmanEncoderCompress::getNewCodes(HuffmanTreeNode* np, unsigned long long newCode, string newCodeStr, int length) {
	// 是叶子节点则结束，记录编码 
	if (np->codePtr) {
		np->codePtr->newCode = newCode;
		np->codePtr->newCodeStr = newCodeStr;
		np->codePtr->length = length;
		return;
	} 
	// 否则向左右分支探索 
	newCode <<= 1;
	++length;
	if (np->left) getNewCodes(np->left, newCode, newCodeStr + "0", length);
	if (np->right) getNewCodes(np->right, newCode + 1, newCodeStr + "1", length);
}


// 压缩文件 
void HuffmanEncoderCompress::compress() {

	FILE* inputFp = NULL;
	if ((inputFp = fopen(_inputFileName.c_str(), "rb")) == NULL) {
		printf("open file %s failed!\n", _inputFileName.c_str());
		exit(-1);
	}

	FILE* outputFp = NULL;
	if ((outputFp = fopen(_outputFileName.c_str(), "wb")) == NULL) {
		printf("open file %s failed!\n", _outputFileName.c_str());
		exit(-1);
	}

	// 写入压缩文件头信息 
	fwrite(_ZIP_NAME, _ZIP_NAME_LEN, 1, outputFp);					// 识别符		 
	fwrite(_inputFileName.c_str(), _FILE_NAME_LEN, 1, outputFp);	// 文件名 
	fwrite(&_inputFileSize, _FILE_SIZE_LEN, 1, outputFp); 			// 文件大小 
	for (int i = 0; i < _CODE_NUM; ++i) 				// 字符频率，用以构建Huffman树 
		fwrite(&_codes[i].frequency, _CODE_FREQUENCY_LEN, 1, outputFp);

	// 压缩所需临时辅助变量 
	unsigned char inputByte;
	unsigned char outputByte = 0;
	unsigned long long newCode;
	int length;
	int cnt = 0;
	unsigned long long currentInputSize = 0;
	unsigned long long currentOutputSize = 0;
	double currRate = 0.0;
	//	string s;		//debug

	while (!feof(inputFp)) {

		fread(&inputByte, 1, 1, inputFp);
		if (feof(inputFp)) break;

		// 记录速率 
		double rate = (double)(++currentInputSize) / _inputFileSize * 100;
		if (rate - currRate >= 10) {
			currRate = rate;
			//	system("cls");
			printf("已压缩：%.1f%%\t压缩率：%.2f%%\n", currRate, (double)_outputFileSize / currentInputSize * 100);
		}

		newCode = _codes[(int)inputByte].newCode;
		length = _codes[(int)inputByte].length;

		while (length--) {

			outputByte <<= 1;
			outputByte += (newCode >> length) & 1;
			if (++cnt == 8) {
				fwrite(&outputByte, 1, 1, outputFp);
				outputByte = 0;
				cnt = 0;
				++_outputFileSize;
			}
		}
	}

	if (cnt < 8) {	 // 最后一个不足8比特填充 0 
		outputByte <<= 8 - cnt;
		fwrite(&outputByte, 1, 1, outputFp);
		++_outputFileSize;
	}

	//system("cls");
	// 打印压缩信息 
	printf("已压缩：%.1f%%\t压缩率：%.2f%%\n", 100.0, (double)_outputFileSize / currentInputSize * 100);

	fclose(inputFp);
	fclose(outputFp);
}


// 搜索节点，辅助于解码 
bool HuffmanEncoderCompress::findNode(HuffmanTreeNode*& np, unsigned char inputByte, int& pos) {
	// 叶子节点则搜索成功 
	if (np->codePtr) return true;
	if (pos < 0) return false;
	int val = (inputByte >> pos) & 1;
	--pos;
	if (val == 0) {
		np = np->left;
		return findNode(np, inputByte, pos);
	}
	else {
		np = np->right;
		return findNode(np, inputByte, pos);
	}
}


// 解压缩 
void HuffmanEncoderCompress::uncompress() {

	FILE* inputFp = NULL;
	if ((inputFp = fopen(_inputFileName.c_str(), "rb")) == NULL) {
		printf("open file %s failed!\n", _inputFileName.c_str());
		exit(-1);
	}

	FILE* outputFp = NULL;
	if ((outputFp = fopen(_outputFileName.c_str(), "wb")) == NULL) {
		printf("open file %s failed!\n", _outputFileName.c_str());
		exit(-1);
	}

	fseek(inputFp, _ZLZIP_HEAD_LEN, SEEK_SET);
	_inputFileSize = _ZLZIP_HEAD_LEN;

	// 解压缩所需临时变量 
	unsigned char inputByte;
	unsigned char outputByte;
	HuffmanTreeNode* np = _huffmanTreeRoot;
	unsigned long long currentOutputSize = 0;
	int pos;
	double currRate = 0.0;

	while (!feof(inputFp)) {

		fread(&inputByte, 1, 1, inputFp);
		if (feof(inputFp)) break;
		++_inputFileSize;
		pos = 7;

		while (findNode(np, inputByte, pos)) {
			outputByte = np->codePtr->oldCode;
			fwrite(&outputByte, 1, 1, outputFp);	//fwrite向指定的文件中写入若干数据块指针，字符大小，个数，还有输出流

			// 记录速率 
			double rate = (double)(++currentOutputSize) / _outputFileSize * 100;
			if (rate - currRate >= 10) {
				currRate = rate;
				//	system("cls");
				printf("已解压缩：%.1f%%\t解压缩率：%.2f%%\n", currRate, (double)currentOutputSize / _inputFileSize * 100);
			}

			if (currentOutputSize == _outputFileSize) {
				printf("已解压缩：%.1f%%\t解压缩率：%.2f%%\n", 100.0, (double)currentOutputSize / _inputFileSize * 100);
				break;
			}

			np = _huffmanTreeRoot;
		}
	}
	fclose(inputFp);
	fclose(outputFp);
}


// 打印哈夫曼编码信息 
void HuffmanEncoderCompress::printHuffmanEncodeInfo() { 
	printf("%-10s %-10s %-20s %-5s %-10s\n", "原码", "频率", "哈夫曼编码", "长度", "十进制");

	for (int i = 0; i < _CODE_NUM; ++i) {
		Code& code = _codes[i];
		printf("%-10d %-10llu %-20s %-5d %-10llu\n", (int)code.oldCode, code.frequency,
			code.newCodeStr.c_str(), code.length, code.newCode);
	}
}


// 打印压缩或解压缩信息 
void HuffmanEncoderCompress::printInfo(const char* type) {

	double compressRate = (double)_outputFileSize / _inputFileSize * 100;
	printf("%s率：%.2f%%\n", type, compressRate);
	double inputFileSize = _inputFileSize, outputFileSize = _outputFileSize;
	if (inputFileSize < 1024) {
		printf("输入文件大小：%.2fB, 输出文件大小：%.2fB\n", inputFileSize, outputFileSize);
		return;
	}

	inputFileSize /= 1024; outputFileSize /= 1024;
	if (inputFileSize < 1024) {
		printf("输入文件大小：%.2fKB, 输出文件大小：%.2fKB\n", inputFileSize, outputFileSize);
		return;
	}

	inputFileSize /= 1024; outputFileSize /= 1024;
	if (inputFileSize < 1024) {
		printf("输入文件大小：%.2fMB, 输出文件大小：%.2fMB\n", inputFileSize, outputFileSize);
		return;
	}

	inputFileSize /= 1024; outputFileSize /= 1024;
	if (inputFileSize < 1024) {
		printf("输入文件大小：%.2fGB, 输出文件大小：%.2fGB\n", inputFileSize, outputFileSize);
		return;
	}
}