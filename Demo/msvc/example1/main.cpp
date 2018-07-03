#include <iostream>
#include "../../../LibJpeg.h"

using namespace std;

int main(){
	
	string inputImgPath = "lena.jpg";
	string outputImgPath = "result.jpg";

	JpegBuffer * jpegBuffer = new JpegBuffer;

	// 打开图像
	if (LoadJpegBuffer(inputImgPath, jpegBuffer) != 1){
		cerr << "图像打开失败，请检查输入图像路径或图像数据类型是否正确..."<< endl;
		return 0;
	}
	cout << "图像打开成功..."<< endl;

	// 保存图像
	
	if (write_JPEG_file(jpegBuffer->data, jpegBuffer->width, jpegBuffer->height, jpegBuffer->channels, outputImgPath) != 1){
		cerr << "图像保存失败，请检查保存图像路径是否正确..."<< endl;
		return 0;
	}
	cout << "图像保存成功..." << endl;


	return 1;
}