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


// �ͷŽڵ㼰������ 
int HuffmanEncoderCompress::freeNode(HuffmanTreeNode* np) {
	if (np) {
		freeNode(np->left);
		freeNode(np->right);
		delete np;
	}
}

// ���� 
HuffmanEncoderCompress::~HuffmanEncoderCompress() {
	freeNode(_huffmanTreeRoot);
}


// ���� 
void HuffmanEncoderCompress::run() {

	FILE* inputFp = NULL;
	if ((inputFp = fopen(_inputFileName.c_str(), "rb")) == NULL) {
		printf("open file %s failed!\n", _inputFileName.c_str());
		exit(-1);
	}

	char zipName[_ZIP_NAME_LEN];
	fread(zipName, _ZIP_NAME_LEN, 1, inputFp);
	void Look();

	if (_isCompress || strcmp(zipName, _ZIP_NAME)) {		// ��ʶ�������ѹ���ļ� 

		fclose(inputFp);
		printf("��ʼѹ���ļ�%s......\n", _inputFileName.c_str());
		printf("����ͳ��Ƶ��......\n");
		statisticalFrequency();			// ͳ��Ƶ�� 
		printf("���ڹ�����������......\n");
		_huffmanTreeRoot = buildHuffmanTree();		    //������������ 
		printf("���ڲ����±���......\n");
		getNewCodes(_huffmanTreeRoot, 0, string(), 0);	// ����±��� 
		printHuffmanEncodeInfo();
		printf("����ѹ��......\n");
		compress();		// ѹ�� 
		printInfo("ѹ��");
		printf("ѹ���ɹ�\n");

	}
	else {		// ��ʶ�������ѹ���ļ� 

		printf("��ʼ��ѹ���ļ�%s......\n", _inputFileName.c_str());
		printf("��ȡԭʼ�ļ���Ϣ......\n");
		char outputFileName[_FILE_NAME_LEN];
		fread(outputFileName, _FILE_NAME_LEN, 1, inputFp);	// ����ԭ�ļ��� 
		printf("ԭʼ�ļ���Ϊ%s\n", outputFileName);
		_outputFileName = string(outputFileName);
		fread(&_outputFileSize, _FILE_SIZE_LEN, 1, inputFp); 	// ����ԭ�ļ���С

		for (int i = 0; i < _CODE_NUM; ++i) 		       // �����ַ�Ƶ�ʱ� 
			fread(&_codes[i].frequency, _CODE_FREQUENCY_LEN, 1, inputFp);

		fclose(inputFp);
		printf("���ڹ�����������......\n");
		_huffmanTreeRoot = buildHuffmanTree();		     //  ������������ 
		printf("���ڲ����±���......\n");
		getNewCodes(_huffmanTreeRoot, 0, string(), 0);	// ����±���
		printHuffmanEncodeInfo();
		printf("���ڽ�ѹ��......\n");
		uncompress();	// ��ѹ�� 
		printInfo("��ѹ��");
		printf("��ѹ�ɹ�\n");
	}
}

// ͳ��Ƶ�� 
void HuffmanEncoderCompress::statisticalFrequency() {
	// ���ļ� fopen=���书����ʹ�ø�����ģʽ rb����ȡΪ�����ƣ� ��_inputFileName.c_str()��
	FILE* inputFp = NULL;
	if ((inputFp = fopen(_inputFileName.c_str(), "rb")) == NULL) {
		printf("open file %s failed!\n", _inputFileName.c_str());
		exit(-1);
	}
	// ͳ��Ƶ�� foef=(�ļ����������ط�0ֵ���ļ�δ����������0ֵ)
	while (!feof(inputFp)) {
		unsigned char inputByte;
		fread(&inputByte, 1, 1, inputFp);	//fread=(�Ӹ���������inputFp��ȡ1����������inputByte)
		if (feof(inputFp)) break;
		++_codes[inputByte].frequency;
		++_inputFileSize;
	}
	fclose(inputFp);
}


// ���� Huffman �� 
HuffmanTreeNode* HuffmanEncoderCompress::buildHuffmanTree() {
	// ʹ�����ȶ��У��Զ���Ƚ��� 
	priority_queue<HuffmanTreeNode*, vector<HuffmanTreeNode*>, CmparatorOfHuffmanTreeNode> nps;
	// ��ʼ��Ҷ�ӽڵ� 
	for (int i = 0; i < _CODE_NUM; ++i)
		nps.push(new HuffmanTreeNode(_codes[i].frequency, &_codes[i]));
	// ȡ��С�����ڵ�ϲ� 
	while (true) {
		HuffmanTreeNode* np1 = nps.top(); nps.pop();
		HuffmanTreeNode* np2 = nps.top(); nps.pop();
		HuffmanTreeNode* np3 = new HuffmanTreeNode(np1->weight + np2->weight, nullptr, np1, np2);
		if (!nps.empty()) nps.push(np3);
		else return np3;	// ���ȶ���Ϊ���򹹳� Huffman �� 
	}
}


// ��� Huffman ���� 
void HuffmanEncoderCompress::getNewCodes(HuffmanTreeNode* np, unsigned long long newCode, string newCodeStr, int length) {
	// ��Ҷ�ӽڵ����������¼���� 
	if (np->codePtr) {
		np->codePtr->newCode = newCode;
		np->codePtr->newCodeStr = newCodeStr;
		np->codePtr->length = length;
		return;
	} 
	// ���������ҷ�֧̽�� 
	newCode <<= 1;
	++length;
	if (np->left) getNewCodes(np->left, newCode, newCodeStr + "0", length);
	if (np->right) getNewCodes(np->right, newCode + 1, newCodeStr + "1", length);
}


// ѹ���ļ� 
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

	// д��ѹ���ļ�ͷ��Ϣ 
	fwrite(_ZIP_NAME, _ZIP_NAME_LEN, 1, outputFp);					// ʶ���		 
	fwrite(_inputFileName.c_str(), _FILE_NAME_LEN, 1, outputFp);	// �ļ��� 
	fwrite(&_inputFileSize, _FILE_SIZE_LEN, 1, outputFp); 			// �ļ���С 
	for (int i = 0; i < _CODE_NUM; ++i) 				// �ַ�Ƶ�ʣ����Թ���Huffman�� 
		fwrite(&_codes[i].frequency, _CODE_FREQUENCY_LEN, 1, outputFp);

	// ѹ��������ʱ�������� 
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

		// ��¼���� 
		double rate = (double)(++currentInputSize) / _inputFileSize * 100;
		if (rate - currRate >= 10) {
			currRate = rate;
			//	system("cls");
			printf("��ѹ����%.1f%%\tѹ���ʣ�%.2f%%\n", currRate, (double)_outputFileSize / currentInputSize * 100);
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

	if (cnt < 8) {	 // ���һ������8������� 0 
		outputByte <<= 8 - cnt;
		fwrite(&outputByte, 1, 1, outputFp);
		++_outputFileSize;
	}

	//system("cls");
	// ��ӡѹ����Ϣ 
	printf("��ѹ����%.1f%%\tѹ���ʣ�%.2f%%\n", 100.0, (double)_outputFileSize / currentInputSize * 100);

	fclose(inputFp);
	fclose(outputFp);
}


// �����ڵ㣬�����ڽ��� 
bool HuffmanEncoderCompress::findNode(HuffmanTreeNode*& np, unsigned char inputByte, int& pos) {
	// Ҷ�ӽڵ��������ɹ� 
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


// ��ѹ�� 
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

	// ��ѹ��������ʱ���� 
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
			fwrite(&outputByte, 1, 1, outputFp);	//fwrite��ָ�����ļ���д���������ݿ�ָ�룬�ַ���С�����������������

			// ��¼���� 
			double rate = (double)(++currentOutputSize) / _outputFileSize * 100;
			if (rate - currRate >= 10) {
				currRate = rate;
				//	system("cls");
				printf("�ѽ�ѹ����%.1f%%\t��ѹ���ʣ�%.2f%%\n", currRate, (double)currentOutputSize / _inputFileSize * 100);
			}

			if (currentOutputSize == _outputFileSize) {
				printf("�ѽ�ѹ����%.1f%%\t��ѹ���ʣ�%.2f%%\n", 100.0, (double)currentOutputSize / _inputFileSize * 100);
				break;
			}

			np = _huffmanTreeRoot;
		}
	}
	fclose(inputFp);
	fclose(outputFp);
}


// ��ӡ������������Ϣ 
void HuffmanEncoderCompress::printHuffmanEncodeInfo() { 
	printf("%-10s %-10s %-20s %-5s %-10s\n", "ԭ��", "Ƶ��", "����������", "����", "ʮ����");

	for (int i = 0; i < _CODE_NUM; ++i) {
		Code& code = _codes[i];
		printf("%-10d %-10llu %-20s %-5d %-10llu\n", (int)code.oldCode, code.frequency,
			code.newCodeStr.c_str(), code.length, code.newCode);
	}
}


// ��ӡѹ�����ѹ����Ϣ 
void HuffmanEncoderCompress::printInfo(const char* type) {

	double compressRate = (double)_outputFileSize / _inputFileSize * 100;
	printf("%s�ʣ�%.2f%%\n", type, compressRate);
	double inputFileSize = _inputFileSize, outputFileSize = _outputFileSize;
	if (inputFileSize < 1024) {
		printf("�����ļ���С��%.2fB, ����ļ���С��%.2fB\n", inputFileSize, outputFileSize);
		return;
	}

	inputFileSize /= 1024; outputFileSize /= 1024;
	if (inputFileSize < 1024) {
		printf("�����ļ���С��%.2fKB, ����ļ���С��%.2fKB\n", inputFileSize, outputFileSize);
		return;
	}

	inputFileSize /= 1024; outputFileSize /= 1024;
	if (inputFileSize < 1024) {
		printf("�����ļ���С��%.2fMB, ����ļ���С��%.2fMB\n", inputFileSize, outputFileSize);
		return;
	}

	inputFileSize /= 1024; outputFileSize /= 1024;
	if (inputFileSize < 1024) {
		printf("�����ļ���С��%.2fGB, ����ļ���С��%.2fGB\n", inputFileSize, outputFileSize);
		return;
	}
}