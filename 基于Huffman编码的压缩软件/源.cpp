#include "HuffmanEncoderCompress.h"

void test(int argc, char** argv);

int main(int argc, char** argv)
{

	test(argc, argv);		// �����϶� 
	return 0;
}


// ��ȡ���루���ڽ������϶��� 
void test(int argc, char** argv) {
	if (argc < 2) exit(-1);
	HuffmanEncoderCompress* hecp = new HuffmanEncoderCompress(argv[1]);
	hecp->run();
	delete hecp;
	getchar();		// ��ֹ���н������� 
}
