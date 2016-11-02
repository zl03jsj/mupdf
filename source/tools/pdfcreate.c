/*
 * PDF creation tool: Tool for creating pdf content.
 *
 * Simple test bed to work with adding content and creating PDFs
 */

#include "mupdf/pdf.h"

static void usage(void)
{
	fprintf(stderr,
		"usage: mutool create [-o output.pdf] [-O options] page.txt [page2.txt ...]\n"
		"\t-o\tname of PDF file to create\n"
		"\t-O\tcomma separated list of output options\n"
		"\tpage.txt\tcontent stream with annotations for creating resources\n\n"
		"Content stream special commands:\n"
		"\t%%%%MediaBox LLX LLY URX URY\n"
		"\t%%%%Rotate Angle\n"
		"\t%%%%Font Name Filename (or base 14 font name)\n"
		"\t%%%%Image Name Filename\n\n"
		);
	fprintf(stderr, "%s\n", fz_pdf_write_options_usage);
	exit(1);
}

static fz_context *ctx = NULL;
static pdf_document *doc = NULL;

//#define PDF_ADD_CJKFONT

static void add_font_res(pdf_obj *resources, char *name, char *path)
{
	const char *data;
	int size, index = 0;
	fz_font *font;
	pdf_obj *subres, *ref;
	// data = fz_lookup_base14_font(ctx, path, &size);
    // "\xBF\xAC\xCC\xE5_GB2312", "simkai,regular",
    // data = fz_lookup_noto_font(ctx, UCDN_SCRIPT_HAN, 0, &size);
#if defined(PDF_ADD_CJKFONT)
    data = fz_lookup_cjk_font(ctx, FZ_ADOBE_GB_1, 0, 1, &size, &index);
#else
    data = fz_lookup_base14_font(ctx, path, &size);
#endif
    if (data) {
        font = fz_new_font_from_memory(ctx, name, data, size, index, 0);
        font->ft_substitute = 1;
        font->ft_stretch = 0; 
    }
    else
        font = fz_new_font_from_file(ctx, name, path, 0, 0);

	subres = pdf_dict_get(ctx, resources, PDF_NAME_Font);
	if (!subres)
	{
		subres = pdf_new_dict(ctx, doc, 10);
		pdf_dict_put_drop(ctx, resources, PDF_NAME_Font, subres);
	}
#if defined(PDF_ADD_CJKFONT)
    ref = pdf_add_cid_font(ctx, doc, font);
#else
	ref = pdf_add_simple_font(ctx, doc, font);
#endif
	pdf_dict_puts(ctx, subres, name, ref);
	pdf_drop_obj(ctx, ref);
	fz_drop_font(ctx, font);
}

static void add_image_res(pdf_obj *resources, char *name, char *path)
{
	fz_image *image;
	pdf_obj *subres, *ref;

	image = fz_new_image_from_file(ctx, path);

	subres = pdf_dict_get(ctx, resources, PDF_NAME_XObject);
	if (!subres)
	{
		subres = pdf_new_dict(ctx, doc, 10);
		pdf_dict_put_drop(ctx, resources, PDF_NAME_XObject, subres);
	}
	ref = pdf_add_image(ctx, doc, image, 0);
	pdf_dict_puts(ctx, subres, name, ref);
	pdf_drop_obj(ctx, ref);

	fz_drop_image(ctx, image);
}

/*
The input is a raw content stream, with commands embedded in comments:

%%MediaBox LLX LLY URX URY
%%Rotate Angle
%%Font Name Filename (or base 14 font name)
%%Image Name Filename
*/
static void create_page(char *input)
{
	fz_rect mediabox = { 0, 0, 595, 842 };
	int rotate = 0;

	char line[4096];
	char *s, *p, *t, *n;
	fz_stream *stm;

	fz_buffer *contents;
	pdf_obj *resources;
	pdf_obj *page;

	resources = pdf_new_dict(ctx, doc, 2);
	contents = fz_new_buffer(ctx, 1024);

	stm = fz_open_file(ctx, input);
	while (fz_read_line(ctx, stm, line, sizeof line))
	{
		if (line[0] == '%' && line[1] == '%')
		{
			p = line;
			s = fz_strsep(&p, " ");
			if (!strcmp(s, "%%MediaBox"))
			{
				mediabox.x0 = fz_atoi(fz_strsep(&p, " "));
				mediabox.y0 = fz_atoi(fz_strsep(&p, " "));
				mediabox.x1 = fz_atoi(fz_strsep(&p, " "));
				mediabox.y1 = fz_atoi(fz_strsep(&p, " "));
			}
			else if (!strcmp(s, "%%Rotate"))
			{
				rotate = fz_atoi(fz_strsep(&p, " "));
			}
			else if (!strcmp(s, "%%Font"))
			{
				s = fz_strsep(&p, " ");
				add_font_res(resources, s, p);
			}
			else if (!strcmp(s, "%%Image"))
			{
				s = fz_strsep(&p, " ");
				add_image_res(resources, s, p);
			}
            else if(!strcmp(s, "%%Text")){
				s = fz_strsep(&p, " ");
                if(s && p) {
                    n = fz_strdup(ctx, s);
                    t = fz_strdup(ctx, p);
                }
            }
		}
		else
		{
			fz_write_buffer(ctx, contents, line, strlen(line));
			fz_write_buffer_byte(ctx, contents, '\n');
		}
	}
	fz_drop_stream(ctx, stm);

	page = pdf_add_page(ctx, doc, &mediabox, rotate, resources, contents);

    const char *fmt = "BT\n/%s 24 Tf\n100 500 Td\n(%s) Tj\nET";
    if(t && n) {
        fz_buffer *tb = fz_new_buffer(ctx, 128);
        fz_buffer_printf(ctx, tb, fmt, n, t);
        pdf_update_stream(ctx, doc, 
                pdf_dict_get(ctx, page, PDF_OBJ_ENUM_NAME_Contents),
                tb, 1);
        fz_drop_buffer(ctx, tb);
        fz_free(ctx, t);
        fz_free(ctx, n); 
    }

	pdf_insert_page(ctx, doc, -1, page);
	pdf_drop_obj(ctx, page);

	fz_drop_buffer(ctx, contents);
	pdf_drop_obj(ctx, resources);
}

int pdfcreate_main(int argc, char **argv)
{
	pdf_write_options opts = { 0 };
	char *output = "out.pdf";
	char *flags = "z";
	int i, c;

	while ((c = fz_getopt(argc, argv, "o:O:")) != -1)
	{
		switch (c)
		{
		case 'o': output = fz_optarg; break;
		case 'O': flags = fz_optarg; break;
		default: usage(); break;
		}
	}

	if (fz_optind == argc)
		usage();

	ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
	if (!ctx)
	{
		fprintf(stderr, "cannot initialise context\n");
		exit(1);
	}
	pdf_parse_write_options(ctx, &opts, flags);
	doc = pdf_create_document(ctx);

	for (i = fz_optind; i < argc; ++i)
		create_page(argv[i]);

	pdf_save_document(ctx, doc, output, &opts);

	pdf_drop_document(ctx, doc);

	fz_flush_warnings(ctx);
	fz_drop_context(ctx);
	return 0;
}
