// this file is create by zl(2016/5/17)
// purpose to insert picture into pdf!!!

#include "mupdf/pdf.h"
#include "../../thirdparty/zlib/zlib.h"

static void useage(){
    printf("mutool addimg -x -y -h -w -n infile imagefile outfile\n");
}

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
		default: useage(); break;
		}
	}
	if (argc - fz_optind < 2) {
        useage();
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
		stderr_tofile("err.log");
		outfile = new_unique_string(ctx, "./", ".pdf");
		pdf_add_image_with_filename(ctx, infile, imgfile, outfile, n, x, y, w, h, NULL);
        printf("outfile=%s\n", outfile);
		fz_free(ctx, outfile);
	}
	fz_catch(ctx) {
		retcode = 1;
	}
	fz_drop_context(ctx);
	stderr_restore();
	return retcode;
} 
