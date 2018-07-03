#include <iostream>
#include "../../../LibJpeg.h"

using namespace std;

int main(){
	
	string inputImgPath = "lena.jpg";
	string outputImgPath = "result.jpg";

	JpegBuffer * jpegBuffer = new JpegBuffer;

	// ��ͼ��
	if (LoadJpegBuffer(inputImgPath, jpegBuffer) != 1){
		cerr << "ͼ���ʧ�ܣ���������ͼ��·����ͼ�����������Ƿ���ȷ..."<< endl;
		return 0;
	}
	cout << "ͼ��򿪳ɹ�..."<< endl;

	// ����ͼ��
	
	if (write_JPEG_file(jpegBuffer->data, jpegBuffer->width, jpegBuffer->height, jpegBuffer->channels, outputImgPath) != 1){
		cerr << "ͼ�񱣴�ʧ�ܣ����鱣��ͼ��·���Ƿ���ȷ..."<< endl;
		return 0;
	}
	cout << "ͼ�񱣴�ɹ�..." << endl;


	return 1;
}