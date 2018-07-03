/*******************************************
// Summary: 利用libjpeg-turbo读取并保存JPEG图像
// Author:  Amusi
// Date:    2018-07-02
// Note:    注意需要添加相应的静态库
*******************************************/

#ifndef _LIBJPGE_H
#define _LIBJPEG_H

// libjpeg-turbo
extern "C" {
#ifdef _WIN32
#include "windows/jconfig.h"
#include "windows/jpeglib.h"
#else
#include "linux/jconfig.h"
#include "linux/jpeglib.h"
#endif
}
#include <iostream>


// 定义Jpeg图像数据类型
typedef struct JpegBufferInfo
{
	unsigned int   width;
	unsigned int   height;
	unsigned int   channels;
	unsigned char *data;
}JpegBuffer;

// 自定义错误处理结构体
struct my_error_mgr {
	struct jpeg_error_mgr pub;  /* "public" fields */

	jmp_buf setjmp_buffer;  /* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void) my_error_exit(j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr)cinfo->err;
	(*cinfo->err->output_message) (cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}


// 调用函数
int LoadJpegBuffer(std::string strImageName, JpegBuffer* jpegbuffer);
int write_JPEG_file(unsigned char* img_buffer, int img_width, int img_height, int img_channels, std::string strImageName);


#endif