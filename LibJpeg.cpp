#include "LibJpeg.h"


// 读取Jpeg图像，将图像数据存放在JpegBuffer结构体中
// 输入: 
//		stringImageName: 待读取的图像路径
//		jpegbuffer:      自定义JPEG图像结构体
// 输出:
//		1: 图像读取成功
//		-1: 图像读取失败
int LoadJpegBuffer(std::string strImageName, JpegBuffer* jpegbuffer)
{
	// ************** 1 为JPEG对象分配空间并初始化 **************

	// 解压缩使用的JPEG对象是一个jpeg_decompress_struct的结构体
	// 同时还需要定义一个用于错误处理的结构体对象，IJG中标准错误结构体是jpeg_error_mgr

	struct jpeg_decompress_struct cinfo;
	// struct jpeg_error_mgr jerr;	// (不推荐)这个标准的错误处理结构将使程序在出现错误时调用exit()退出程序
	// 2018-03-15 自定义错误处理结构体
	// 如果不希望使用标准的错误处理方式，则可以通过下述的自定义退出函数的方法自定义错误处理结构
	struct my_error_mgr jerr;

	errno_t err;


	// ************** 2 指定解压缩数据源 **************

	// 利用标准C中的文件指针打开jpeg文件
	FILE * infile;/* source file */

	if ((err = fopen_s(&infile, strImageName.c_str(), "rb")) != 0)
	{
		fprintf(stderr, "can't open %s\n", strImageName);
		return 0;
	}

	// 将错误处理结构绑定到JPEG对象上 
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer))
	{
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		// 2018-03-19 Fangjie Chen: 是否要删除下述cerr语句
		std::cerr << "图像非jpeg..." << std::endl;
		return -1;
	}

	// 初始化cinfo结构体 
	jpeg_create_decompress(&cinfo);

	// 绑定输入结构体和文件指针
	jpeg_stdio_src(&cinfo, infile);

	// ************** 3 获取文件信息 **************
	// 因为上述绑定关系，可读取图像已有信息
	// 2018-03-15 Fangjie Chen: 判断jpeg_read_header返回值
	(void)jpeg_read_header(&cinfo, TRUE);


	// 此时常见的可用信息包括图像的宽cinfo.image_width，高cinfo.image_height，色彩空间cinfo.jpeg_color_space，颜色通道数cinfo.num_components等。

	jpegbuffer->width = cinfo.image_width;
	jpegbuffer->height = cinfo.image_height;
	jpegbuffer->channels = cinfo.num_components;

	// ************** 4 为解压缩设定参数，如图像大小、颜色空间 **************

	// 在完成jpeg_read_header调用后，开始解压缩之前就可以进行解压缩参数的设定，也就是为cinfo结构的成员赋值
	// 比如可以设定解出来的图像的大小，也就是与原图的比例。使用scale_num和scale_denom两个参数，解出来的图像大小就是scale_num / scale_denom，但是IJG当前仅支持1 / 1, 1 / 2, 1 / 4, 和1 / 8这几种缩小比例。
	// 如取得原图的图像
	cinfo.scale_num = 1;
	cinfo.scale_denom = 1;

	// 或取得1/2原图的图像
	/*cinfo.scale_num = 1;
	cinfo.scale_denom = 2;*/

	// 也可以设定输出图像的色彩空间，即cinfo.out_color_space，可以把一个原本彩色的图像由真彩色JCS_RGB变为灰度JCS_GRAYSCALE。如：
	//cinfo.out_color_space = JCS_GRAYSCALE;


	// ************** 5 开始解压缩 **************

	(void)jpeg_start_decompress(&cinfo);

	// 在完成解压缩操作后，IJG就会将解压后的图像信息填充至cinfo结构中。比如，输出图像宽度cinfo.output_width,输出图像高度cinfo.output_height，
	// 每个像素中的颜色通道数cinfo.output_components（比如灰度为1，全彩色为3）的等
	// 一般情况下，这些参数是在jpeg_start_decompress后才被赋值到cinfo中的，如果希望在调用jpeg_start_decompress之前就获得这些参数，可以通过调用jpeg_calc_output_dimensions()的方法来实现
	//输出图像的信息  


	// ************** 6 取出数据 **************

	/* 解开的数据是按照行取出的，数据像素按照scanline来存储，scanline是从左到右，从上到下的顺序（注意bmp相反），每个像素对应的各颜色或灰度通道数据是依次存储，
	比如一个24-bitRGB真彩色的图像中，一个scanline中的数据存储模式是R,G,B,R,G,B,R,G,B,...，每条scanline是一个JSAMPLE类型的数组，一般来说就是unsigned char**，定义于jmorecfg.h中。

	除了JSAMPLE，IJG还定义了JSAMPROW和JSAMPARRAY，分别表示一行JSAMPLE和一个2D的JSAMPLE数组。

	在此，我们定义一个JSAMPARRAY类型的缓冲区变量来存放图像数据。
	*/
	JSAMPARRAY buffer;	//  JSAMPARRAY 等价于 unsigned char * *  类型， 在jpeglib.h中定义  UNDONE: buffer[1][]

	// 然后是计算每行需要的空间大小，比如RGB图像就是宽度×3，灰度图就是宽度×1
	int row_stride = cinfo.output_width*cinfo.output_components;

	/*
	为缓冲区分配空间，这里使用了IJG的内存管理器来完成分配。

	JPOOL_IMAGE表示分配的内存空间将在调用jpeg_finish_compress，jpeg_finish_decompress，jpeg_abort后被释放，
	而如果此参数改为JPOOL_PERMANENT则表示内存将一直到JPEG对象被销毁时才被释放。

	row_stride如上所说，是每行数据的实际大小。

	最后一个参数是要分配多少行数据。此处只分配了一行。
	*/

	buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

	// UNDONE: 不知道下述写法是否正确
	jpegbuffer->data = new unsigned char[cinfo.output_width * cinfo.output_height * cinfo.output_components];

	// output_scanline表示当前已经读取的行数，如此即可依次读出图像的所有数据，并填充到缓冲区中，参数1表示的是每次读取的行数。
	while (cinfo.output_scanline < cinfo.output_height)
	{
		//(void)jpeg_read_scanlines(&cinfo,buffer,1);
		unsigned char * p = jpegbuffer->data + cinfo.output_scanline*cinfo.image_width*cinfo.num_components;
		(void)jpeg_read_scanlines(&cinfo, &p, 1);
		// UNDONE: do something
	}

	// ************** 7 解压缩完毕 **************
	(void)jpeg_finish_decompress(&cinfo);


	// ************** 8 释放资源 **************
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);

	/*
	如果不再需要JPEG对象，则使用jpeg_destroy_decompress(&cinfo); 或 jpeg_destroy(&cinfo);
	而如果还希望继续使用JPEG对象，则可使用jpeg_abort_decompress(&cinfo); 或 jpeg_abort(&cinfo);
	*/

	return 1;
}



// 保存Jpeg图像，将JpegBuffer结构体数据保存成图像
// 输入: 
//		img_buffer:      自定义JPEG图像结构体
//		img_width: 待读取的图像路径
//		img_height:
//		img_channels:
//		strImageName
// 输出:
//		1: 图像保存成功
//		-1: 图像保存失败
int write_JPEG_file(unsigned char* img_buffer, int img_width, int img_height, int img_channels, std::string strImageName)
{
	//unsigned char* image_buffer;    /* Points to large array of R,G,B-order data */
	int image_height = img_height; /* Number of rows in image */
	int image_width = img_width;  /* Number of columns in image */


	/* This struct contains the JPEG compression parameters and pointers to
	* working space (which is allocated as needed by the JPEG library).
	* It is possible to have several such structures, representing multiple
	* compression/decompression processes, in existence at once.  We refer
	* to any one struct (and its associated working data) as a "JPEG object".
	*/
	struct jpeg_compress_struct cinfo;

	/* This struct represents a JPEG error handler.  It is declared separately
	* because applications often want to supply a specialized error handler
	* (see the second half of this file for an example).  But here we just
	* take the easy way out and use the standard error handler, which will
	* print a message on stderr and call exit() if compression fails.
	* Note that this struct must live as long as the main JPEG parameter
	* struct, to avoid dangling-pointer problems.
	*/
	struct jpeg_error_mgr jerr;
	/* More stuff */
	FILE * outfile;     /* target file */
	JSAMPROW row_pointer[1];    /* pointer to JSAMPLE row[s] */
	int row_stride;     /* physical row width in image buffer */

	/* Step 1: allocate and initialize JPEG compression object */

	/* We have to set up the error handler first, in case the initialization
	* step fails.  (Unlikely, but it could happen if you are out of memory.)
	* This routine fills in the contents of struct jerr, and returns jerr's
	* address which we place into the link field in cinfo.
	*/
	cinfo.err = jpeg_std_error(&jerr);
	/* Now we can initialize the JPEG compression object. */
	jpeg_create_compress(&cinfo);

	/* Step 2: specify data destination (eg, a file) */
	/* Note: steps 2 and 3 can be done in either order. */

	/* Here we use the library-supplied code to send compressed data to a
	* stdio stream.  You can also write your own code to do something else.
	* VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
	* requires it in order to write binary files.
	*/
	if ((outfile = fopen(strImageName.c_str(), "wb")) == NULL) {
		fprintf(stderr, "can't open %s\n", strImageName);
		//exit(1);  
		return -1;
	}
	jpeg_stdio_dest(&cinfo, outfile);

	/* Step 3: set parameters for compression */
	/* First we supply a description of the input image.
	* Four fields of the cinfo struct must be filled in:
	*/
	cinfo.image_width = image_width;    /* image width and height, in pixels */
	cinfo.image_height = image_height;
	cinfo.input_components = img_channels;     /* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB;     /* colorspace of input image */
	/* Now use the library's routine to set default compression parameters.
	* (You must set at least cinfo.in_color_space before calling this,
	* since the defaults depend on the source color space.)
	*/
	jpeg_set_defaults(&cinfo);
	/* Now you can set any non-default parameters you wish to.
	* Here we just illustrate the use of quality (quantization table) scaling:
	*/
	int quality = 100;
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	/* Step 4: Start compressor */
	/* TRUE ensures that we will write a complete interchange-JPEG file.
	* Pass TRUE unless you are very sure of what you're doing.
	*/
	jpeg_start_compress(&cinfo, TRUE);

	/* Step 5: while (scan lines remain to be written) */
	/*           jpeg_write_scanlines(...); */
	/* Here we use the library's state variable cinfo.next_scanline as the
	* loop counter, so that we don't have to keep track ourselves.
	* To keep things simple, we pass one scanline per call; you can pass
	* more if you wish, though.
	*/
	row_stride = image_width * 3;   /* JSAMPLEs per row in image_buffer */

	// UNDONE
	//image_buffer = new unsigned char[row_stride * cinfo.image_height];
	//memset(image_buffer, 0xff, row_stride * cinfo.image_height);

	int line = 0;
	//while (cinfo.next_scanline < cinfo.image_height) {  
	while (line < cinfo.image_height) {
		/* jpeg_write_scanlines expects an array of pointers to scanlines.
		* Here the array is only one element long, but you could pass
		* more than one scanline at a time if that's more convenient.
		*/
		//row_pointer[0] = &image_buffer[cinfo.next_scanline * row_stride];  
		row_pointer[0] = &img_buffer[line * row_stride];
		jpeg_write_scanlines(&cinfo, row_pointer, 1);

		line++;
	}

	//delete img_buffer;

	/* Step 6: Finish compression */
	jpeg_finish_compress(&cinfo);
	/* After finish_compress, we can close the output file. */
	fclose(outfile);

	/* Step 7: release JPEG compression object */
	/* This is an important step since it will release a good deal of memory. */
	jpeg_destroy_compress(&cinfo);

	return 1;
}