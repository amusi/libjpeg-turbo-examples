/*******************************************
// Summary: ����libjpeg-turbo��ȡ������JPEGͼ��
// Author:  Amusi
// Date:    2018-07-02
// Note:    ע����Ҫ�����Ӧ�ľ�̬��
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


// ����Jpegͼ����������
typedef struct JpegBufferInfo
{
	unsigned int   width;
	unsigned int   height;
	unsigned int   channels;
	unsigned char *data;
}JpegBuffer;

// �Զ��������ṹ��
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


// ���ú���
int LoadJpegBuffer(std::string strImageName, JpegBuffer* jpegbuffer);
int write_JPEG_file(unsigned char* img_buffer, int img_width, int img_height, int img_channels, std::string strImageName);


#endif