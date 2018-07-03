#include "LibJpeg.h"


// ��ȡJpegͼ�񣬽�ͼ�����ݴ����JpegBuffer�ṹ����
// ����: 
//		stringImageName: ����ȡ��ͼ��·��
//		jpegbuffer:      �Զ���JPEGͼ��ṹ��
// ���:
//		1: ͼ���ȡ�ɹ�
//		-1: ͼ���ȡʧ��
int LoadJpegBuffer(std::string strImageName, JpegBuffer* jpegbuffer)
{
	// ************** 1 ΪJPEG�������ռ䲢��ʼ�� **************

	// ��ѹ��ʹ�õ�JPEG������һ��jpeg_decompress_struct�Ľṹ��
	// ͬʱ����Ҫ����һ�����ڴ�����Ľṹ�����IJG�б�׼����ṹ����jpeg_error_mgr

	struct jpeg_decompress_struct cinfo;
	// struct jpeg_error_mgr jerr;	// (���Ƽ�)�����׼�Ĵ�����ṹ��ʹ�����ڳ��ִ���ʱ����exit()�˳�����
	// 2018-03-15 �Զ��������ṹ��
	// �����ϣ��ʹ�ñ�׼�Ĵ�����ʽ�������ͨ���������Զ����˳������ķ����Զ��������ṹ
	struct my_error_mgr jerr;

	errno_t err;


	// ************** 2 ָ����ѹ������Դ **************

	// ���ñ�׼C�е��ļ�ָ���jpeg�ļ�
	FILE * infile;/* source file */

	if ((err = fopen_s(&infile, strImageName.c_str(), "rb")) != 0)
	{
		fprintf(stderr, "can't open %s\n", strImageName);
		return 0;
	}

	// ��������ṹ�󶨵�JPEG������ 
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer))
	{
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		// 2018-03-19 Fangjie Chen: �Ƿ�Ҫɾ������cerr���
		std::cerr << "ͼ���jpeg..." << std::endl;
		return -1;
	}

	// ��ʼ��cinfo�ṹ�� 
	jpeg_create_decompress(&cinfo);

	// ������ṹ����ļ�ָ��
	jpeg_stdio_src(&cinfo, infile);

	// ************** 3 ��ȡ�ļ���Ϣ **************
	// ��Ϊ�����󶨹�ϵ���ɶ�ȡͼ��������Ϣ
	// 2018-03-15 Fangjie Chen: �ж�jpeg_read_header����ֵ
	(void)jpeg_read_header(&cinfo, TRUE);


	// ��ʱ�����Ŀ�����Ϣ����ͼ��Ŀ�cinfo.image_width����cinfo.image_height��ɫ�ʿռ�cinfo.jpeg_color_space����ɫͨ����cinfo.num_components�ȡ�

	jpegbuffer->width = cinfo.image_width;
	jpegbuffer->height = cinfo.image_height;
	jpegbuffer->channels = cinfo.num_components;

	// ************** 4 Ϊ��ѹ���趨��������ͼ���С����ɫ�ռ� **************

	// �����jpeg_read_header���ú󣬿�ʼ��ѹ��֮ǰ�Ϳ��Խ��н�ѹ���������趨��Ҳ����Ϊcinfo�ṹ�ĳ�Ա��ֵ
	// ��������趨�������ͼ��Ĵ�С��Ҳ������ԭͼ�ı�����ʹ��scale_num��scale_denom�����������������ͼ���С����scale_num / scale_denom������IJG��ǰ��֧��1 / 1, 1 / 2, 1 / 4, ��1 / 8�⼸����С������
	// ��ȡ��ԭͼ��ͼ��
	cinfo.scale_num = 1;
	cinfo.scale_denom = 1;

	// ��ȡ��1/2ԭͼ��ͼ��
	/*cinfo.scale_num = 1;
	cinfo.scale_denom = 2;*/

	// Ҳ�����趨���ͼ���ɫ�ʿռ䣬��cinfo.out_color_space�����԰�һ��ԭ����ɫ��ͼ�������ɫJCS_RGB��Ϊ�Ҷ�JCS_GRAYSCALE���磺
	//cinfo.out_color_space = JCS_GRAYSCALE;


	// ************** 5 ��ʼ��ѹ�� **************

	(void)jpeg_start_decompress(&cinfo);

	// ����ɽ�ѹ��������IJG�ͻὫ��ѹ���ͼ����Ϣ�����cinfo�ṹ�С����磬���ͼ����cinfo.output_width,���ͼ��߶�cinfo.output_height��
	// ÿ�������е���ɫͨ����cinfo.output_components������Ҷ�Ϊ1��ȫ��ɫΪ3���ĵ�
	// һ������£���Щ��������jpeg_start_decompress��ű���ֵ��cinfo�еģ����ϣ���ڵ���jpeg_start_decompress֮ǰ�ͻ����Щ����������ͨ������jpeg_calc_output_dimensions()�ķ�����ʵ��
	//���ͼ�����Ϣ  


	// ************** 6 ȡ������ **************

	/* �⿪�������ǰ�����ȡ���ģ��������ذ���scanline���洢��scanline�Ǵ����ң����ϵ��µ�˳��ע��bmp�෴����ÿ�����ض�Ӧ�ĸ���ɫ��Ҷ�ͨ�����������δ洢��
	����һ��24-bitRGB���ɫ��ͼ���У�һ��scanline�е����ݴ洢ģʽ��R,G,B,R,G,B,R,G,B,...��ÿ��scanline��һ��JSAMPLE���͵����飬һ����˵����unsigned char**��������jmorecfg.h�С�

	����JSAMPLE��IJG��������JSAMPROW��JSAMPARRAY���ֱ��ʾһ��JSAMPLE��һ��2D��JSAMPLE���顣

	�ڴˣ����Ƕ���һ��JSAMPARRAY���͵Ļ��������������ͼ�����ݡ�
	*/
	JSAMPARRAY buffer;	//  JSAMPARRAY �ȼ��� unsigned char * *  ���ͣ� ��jpeglib.h�ж���  UNDONE: buffer[1][]

	// Ȼ���Ǽ���ÿ����Ҫ�Ŀռ��С������RGBͼ����ǿ�ȡ�3���Ҷ�ͼ���ǿ�ȡ�1
	int row_stride = cinfo.output_width*cinfo.output_components;

	/*
	Ϊ����������ռ䣬����ʹ����IJG���ڴ����������ɷ��䡣

	JPOOL_IMAGE��ʾ������ڴ�ռ佫�ڵ���jpeg_finish_compress��jpeg_finish_decompress��jpeg_abort���ͷţ�
	������˲�����ΪJPOOL_PERMANENT���ʾ�ڴ潫һֱ��JPEG��������ʱ�ű��ͷš�

	row_stride������˵����ÿ�����ݵ�ʵ�ʴ�С��

	���һ��������Ҫ������������ݡ��˴�ֻ������һ�С�
	*/

	buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

	// UNDONE: ��֪������д���Ƿ���ȷ
	jpegbuffer->data = new unsigned char[cinfo.output_width * cinfo.output_height * cinfo.output_components];

	// output_scanline��ʾ��ǰ�Ѿ���ȡ����������˼������ζ���ͼ����������ݣ�����䵽�������У�����1��ʾ����ÿ�ζ�ȡ��������
	while (cinfo.output_scanline < cinfo.output_height)
	{
		//(void)jpeg_read_scanlines(&cinfo,buffer,1);
		unsigned char * p = jpegbuffer->data + cinfo.output_scanline*cinfo.image_width*cinfo.num_components;
		(void)jpeg_read_scanlines(&cinfo, &p, 1);
		// UNDONE: do something
	}

	// ************** 7 ��ѹ����� **************
	(void)jpeg_finish_decompress(&cinfo);


	// ************** 8 �ͷ���Դ **************
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);

	/*
	���������ҪJPEG������ʹ��jpeg_destroy_decompress(&cinfo); �� jpeg_destroy(&cinfo);
	�������ϣ������ʹ��JPEG�������ʹ��jpeg_abort_decompress(&cinfo); �� jpeg_abort(&cinfo);
	*/

	return 1;
}



// ����Jpegͼ�񣬽�JpegBuffer�ṹ�����ݱ����ͼ��
// ����: 
//		img_buffer:      �Զ���JPEGͼ��ṹ��
//		img_width: ����ȡ��ͼ��·��
//		img_height:
//		img_channels:
//		strImageName
// ���:
//		1: ͼ�񱣴�ɹ�
//		-1: ͼ�񱣴�ʧ��
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