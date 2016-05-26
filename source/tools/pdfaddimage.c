// this file is create by zl(2016/5/17)
// purpose to insert picture into pdf!!!
#include "mupdf/pdf/pdf_extension.h"
#include "../../thirdparty/zlib/zlib.h"

int muaddimage_main(int argc, char *argv[]) 
{
	char *infile = NULL;
	char *outfile = NULL;
	char *imgfile = NULL;
	char *password = "";
	int x, y, w, h, n, c;
	while ((c = fz_getopt(argc, argv, "x:y:w:h:n:")) != -1) {
		switch (c)
		{
		case 'p': password = fz_optarg; break;
		case 'x': x = atoi(fz_optarg); break;
		case 'y': y = atoi(fz_optarg); break;
		case 'w': w = atoi(fz_optarg); break;
		case 'h': h = atoi(fz_optarg); break;
		case 'n': n = atoi(fz_optarg); break;
		default: break;
		}
	}
	if (argc - fz_optind < 2) {
		// argument not enough!
		return 0;
	}
	infile = argv[fz_optind++];
	imgfile = argv[fz_optind++];
	if (argc - fz_optind > 0 &&
		(strstr(argv[fz_optind], ".pdf") || strstr(argv[fz_optind], ".PDF"))) {
		outfile = argv[fz_optind++];
	}

	fz_context *ctx = fz_new_context(NULL, NULL, FZ_STORE_DEFAULT);
	int retcode = 0;
	fz_try(ctx) {
		// char *errlogfile = new_unique_string(ctx, "./err_", ".log");
		 char *errlogfile = "errs.log";
		stderr_tofile(errlogfile);
		// fz_free(ctx, errlogfile); errlogfile = NULL;
		outfile = new_unique_string(ctx, "./", ".pdf");
		int code = pdf_page_add_image_file(ctx, infile, imgfile, outfile, n, x, y, w, h);
		fz_free(ctx, outfile);
		fz_drop_context(ctx);
	}
	fz_catch(ctx) {
		retcode = 1;
	}
	stderr_restore();
	return retcode;
} 
