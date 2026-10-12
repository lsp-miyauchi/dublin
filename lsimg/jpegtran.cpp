// JPEGTRAN DIRTY-functional version
//
// 'Rota' uses lossless-rotate only, most of functions
// are not needed. I placed only nessecelly functions of
// transupp and jpeglib. If you need original source of
// jpegtran.c, visit http://www.ijg.org/ and get jpeg-6b.
//
/* @@@
#include <setjmp.h>
#include "cdjpeg.h"
#include "transupp.h"
@@@ */

#include "stdafx.h"
#include "lsimg.h"


#ifdef __cplusplus
extern "C" {                    // Assume C declarations for C++
#endif

#include "jpgcode.h"		/* Common decls for cjpeg/djpeg applications */

#ifdef __cplusplus
}
#endif


struct my_error_mgr{
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)my_error_exit(j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	longjmp(myerr->setjmp_buffer, 1);
}


int jpegtran(int angle, char *infile, char *outfile)
{
	struct jpeg_decompress_struct srcinfo;
	struct jpeg_compress_struct dstinfo;
	struct jpeg_error_mgr jsrcerr, jdsterr;
	jvirt_barray_ptr * src_coef_arrays;
	jvirt_barray_ptr * dst_coef_arrays;
	FILE * input_file;
	FILE * output_file;
	JCOPY_OPTION copyoption;
	jpeg_transform_info transformoption;
	struct my_error_mgr jerr_src, jerr_dst;

	copyoption = JCOPYOPT_ALL;
	transformoption.trim = FALSE;
	transformoption.force_grayscale = FALSE;

	switch(angle){
	case 0:
		return 1;
	case 1:
		transformoption.transform = JXFORM_ROT_90;
		break;
	case 2:
		transformoption.transform = JXFORM_ROT_180;
		break;
	case 3:
		transformoption.transform = JXFORM_ROT_270;
		break;
	default:
		return 1;
	}
	srcinfo.err = jpeg_std_error(&jerr_src.pub);
	jerr_src.pub.error_exit = my_error_exit;
	if (setjmp(jerr_src.setjmp_buffer)) {
		jpeg_destroy_decompress(&srcinfo);
		return 2;
	}
	srcinfo.err->trace_level = 0;
	jpeg_create_decompress(&srcinfo);

	dstinfo.err = jpeg_std_error(&jerr_dst.pub);
	jerr_dst.pub.error_exit = my_error_exit;
	if (setjmp(jerr_dst.setjmp_buffer)) {
		jpeg_destroy_compress(&dstinfo);
		return 3;
	}
	dstinfo.err->trace_level = 0;
	jpeg_create_compress(&dstinfo);
	jsrcerr.trace_level = jdsterr.trace_level;
	srcinfo.mem->max_memory_to_use = dstinfo.mem->max_memory_to_use;

    if ((input_file = fopen(infile, READ_BINARY)) == NULL) {
    	return 4;
    }
    if ((output_file = fopen(outfile, WRITE_BINARY)) == NULL) {
    	fclose(input_file);
    	return 5;
    }

	jpeg_stdio_src(&srcinfo, input_file);
	jcopy_markers_setup(&srcinfo, copyoption);
	(void)jpeg_read_header(&srcinfo, TRUE);
	jtransform_request_workspace(&srcinfo, &transformoption);
	src_coef_arrays = jpeg_read_coefficients(&srcinfo);
	jpeg_copy_critical_parameters(&srcinfo, &dstinfo);
	dstinfo.write_JFIF_header = FALSE;	// for Exif format
	dst_coef_arrays = jtransform_adjust_parameters(&srcinfo, &dstinfo,
					 src_coef_arrays, &transformoption);
	jpeg_stdio_dest(&dstinfo, output_file);
	jpeg_write_coefficients(&dstinfo, dst_coef_arrays);
	jcopy_markers_execute(&srcinfo, &dstinfo, copyoption);
	jtransform_execute_transformation(&srcinfo, &dstinfo,
				    src_coef_arrays, &transformoption);
	jpeg_finish_compress(&dstinfo);
	jpeg_destroy_compress(&dstinfo);
	(void) jpeg_finish_decompress(&srcinfo);
    jpeg_destroy_decompress(&srcinfo);

    fclose(input_file);
    fclose(output_file);

	return 0;
}

