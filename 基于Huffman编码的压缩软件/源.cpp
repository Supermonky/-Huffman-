#include "HuffmanEncoderCompress.h"

void test(int argc, char** argv);

int main(int argc, char** argv)
{

	test(argc, argv);		// 界面拖动 
	return 0;
}


// 读取输入（可在界面中拖动） 
void test(int argc, char** argv) {
	if (argc < 2) exit(-1);
	HuffmanEncoderCompress* hecp = new HuffmanEncoderCompress(argv[1]);
	hecp->run();
	delete hecp;
	getchar();		// 防止运行界面闪退 
}
