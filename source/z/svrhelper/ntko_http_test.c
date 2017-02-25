/*
 * =====================================================================================
 *
 *       Filename:  ntko_http_test.c *
 *    Description:  *
 *        Version:  1.0
 *        Created:  2017/02/13 10时30分26秒
 *       Revision:  none
 *       Compiler:  gcc *
 *         Author:  YOUR NAME (), 
 *   Organization:  *
 * =====================================================================================
 */
#include <stdlib.h>
#include "mupdf/z/ntko_svr.h"

static
void clear_stdin_cache() { char c; while ((c=getchar())!=EOF&&c!='\n'); }

static 
char *readString(fz_context *ctx, const char *key) {
    char readstr[256];
    memset(readstr, 0, sizeof(readstr));
    printf("%8s: ", key); 

    // read chars till meet '\n', and skip '\n'
    char e = 0;
    scanf ("%[^\n]%c", readstr, &e);
    while(e!='\n') scanf("%c", &e);

    int len = strlen(readstr);
    char *tmp = null;
    if(len) {
        fz_try(ctx)
            tmp = fz_strdup(ctx, readstr);
        fz_catch(ctx)
            fz_warn(ctx, "%s", fz_caught_message(ctx));
    }
    return tmp;
}

static 
bool readbool(fz_context *ctx, bool def) 
{
    char b = 'n';
    bool bvalue = false;

    char e = 0;
    scanf("%c%*[^\n]%c", &b, &e); 
    while(e!='\n') scanf("%c", &e);

    bvalue = b=='y' ? true : b=='n' ? false : def;

    return bvalue;
}

static 
int readint(fz_context *ctx, int def)
{
    char istr[16] = {0};

    char e = 0;
    scanf("%[0-9]s%*[^\n]%c", istr, &e); 
    while(e!='\n') scanf("%c", &e);

    int i;
    if(istr[0]>47 && istr[0]<59)
        i = atoi(istr);
    else i =def;

    return i; 
}

static
void display_esp_infolist(fz_context *ctx, z_list *esplist)
{

    ntko_server_espinfo *espinfo = null;
    z_list_node *n = esplist->first; 
    printf("-----------------\n");
    printf("esp list:\n");

    printf("%-8s%-16s%-16s%-24s\n", "number", "signname", "signuser", "url");
    int i = 0;
    while(n) {
        espinfo = n->data;
        printf("%-8d%-16s%-16s%-24s\n", ++i,
                espinfo->signname, espinfo->signuser,
                espinfo->filename);
        n = n->n;
    }
    printf("-----------------\n");
}

static 
void display_user_rights(fz_context *ctx, ntko_user_rights *rights)
{
   
    printf("-----------------\n");
    printf("user rights:\n");
    printf("-----------------\n");
    printf("%-24s%-8s\n", "right", "permit");
	printf("%-24s%-8s\n", "handsign", rights->permit_handsign?"yes":"no");
	printf("%-24s%-8s\n", "keyboradSign", rights->permit_keyboradSign?"yes":"no");
	printf("%-24s%-8s\n", "normalSign", rights->permit_normalSign?"yes":"no");
	printf("%-24s%-8s\n", "ekeySign", rights->permit_ekeySign?"yes":"no");
	printf("%-24s%-8s\n", "barCode", rights->permit_barCode?"yes":"no");
	printf("%-24s%-8s\n", "fingerPrint", rights->permit_fingerPrint?"yes":"no");
	printf("%-24s%-8s\n", "multiPage_sign", rights->permit_multiPage_sign?"yes":"no");
	printf("%-24s%-8s\n", "multiCrossPage_sign", rights->permit_multiCrossPage_sign?"yes":"no");
	printf("%-24s%-8s\n", "doubleCrossPage_sign", rights->permit_doubleCrossPage_sign?"yes":"no");
	printf("%-24s%-8s\n", "sign_need_ekey", rights->multiPage_sign_need_ekey?"yes":"no");
	printf("%-24s%-8s\n", "sign_need_ekey", rights->multiCrossPage_sign_need_ekey?"yes":"no");
}

static
void display_options(fz_context *ctx, ntko_sign_options *options)
{
    printf("-----------------\n");
    printf("sign options:\n");
    printf("-----------------\n"); 
    printf("%-24s%-8s\n", "check_doc_modification", options->check_doc_modification?"yes":"no") ; 
    printf("%-24s%-8s\n", "add_comment", options->add_comment?"yes":"no") ;
    printf("%-24s%-8s\n", "use_certificate", options->use_certificate?"yes":"no") ;
    printf("%-24s%-8d\n", " printmode",  options->printmode) ;
    printf("%-24s%-8s\n", "show_option_ui", options->show_option_ui?"yes":"no") ;
    printf("%-24s%-8s\n", "lock_sign", options->lock_sign?"yes":"no") ;
    printf("%-24s%-8s\n", "saved", options->saved?"yes":"no") ;
    printf("%-24s%-8s\n", "check_font_change", options->check_font_change?"yes":"no") ;
    printf("%-24s%-8s\n", "sign_add_hand_draw", options->sign_add_hand_draw?"yes":"no") ;
    printf("%-24s%-8s\n", "sign_add_date", options->sign_add_date?"yes":"no") ;
    printf("%-24s%-8s\n", "csp_issure_name", options->csp_issure_name) ;
}

static
bool list_server_signs(fz_context *ctx, ntko_server_info *svrinfo, ntko_user_rights *rights, ntko_sign_options *options, ntko_http_response_status *status)
{
    z_list *signlist = null;
    bool isok = false;
    fz_try(ctx) {
        signlist = ntko_http_get_esplist(ctx, svrinfo, rights, options, status);
        if(status->code==0) {
            display_esp_infolist(ctx, signlist); 
            display_user_rights(ctx, rights);
            display_options(ctx, options); 
            isok = true;
        }
    }
    fz_always(ctx) {
        if(signlist) z_list_free(ctx, signlist); 
    }
    fz_catch(ctx) {
        if(signlist) z_list_free(ctx, signlist); 
        fz_warn(ctx, "%s", fz_caught_message(ctx));
        return false;
    }
    return isok;
}

static 
void print_title(char *title)  {
    printf("-----------------\n");
    printf("%s:\n", title);
    printf("-----------------\n"); 
}

typedef bool(*doop_func)(fz_context *ctx, ntko_server_info *svrinfo, ntko_user_rights *rights, ntko_sign_options *options, ntko_http_response_status *status);
typedef bool(*opname_check_func)(const char *opname);

typedef struct ntko_op_s 
{
    const char *opname;
    doop_func doop; 
    opname_check_func opname_check;
} ntko_op;

static
bool op_check_listesp(const char *opname) {
    if(0==memcmp(opname, "le", 2) || 0==memcmp(opname, "listesp", 2))
        return true;
    return false;
}

static
bool op_check_getsvrtime(const char *opname) {
    if(0==memcmp(opname, "time", 5) || 0==memcmp(opname, "getsvrtime", 2))
        return true;
    return false; 
}

static
bool op_exit(fz_context *ctx, ntko_server_info *svrinfo, ntko_user_rights *rights, ntko_sign_options *options, ntko_http_response_status *status)
{
    return false;
}

static
bool getsvrtime(fz_context *ctx, ntko_server_info *svrinfo, ntko_user_rights *rights, ntko_sign_options *options, ntko_http_response_status *status) {
    char *svrtime = null;
    fz_try(ctx) {
        svrtime = ntko_http_get_svrtime(ctx, svrinfo, status);
    }
    fz_catch(ctx) {
        fz_warn(ctx, "get servertime err:%s", fz_caught_message(ctx));
    }

    if(svrtime) {
        print_title("server time:");
        printf("%s\n", svrtime);
        fz_free(ctx, svrtime);
    }
    return true;
}

ntko_op ops[] =  {
    {"listesp", list_server_signs, op_check_listesp},
    {"getsvrtime", getsvrtime, op_check_getsvrtime},
    {"exit", op_exit, null}
};

static
bool dologin(fz_context *ctx, ntko_server_info *svrinfo, ntko_http_response_status *status, ntko_user_rights *rights)
{
    char *username = null, *password = null;

    bool isok = false;
    fz_try(ctx) {
        printf("login:----------------------\n");
        username = readString(ctx, "username");
        password = readString(ctx, "password");
        if(svrinfo->settingurl) {
            printf("sign server url:%s,to reset or not, input [y/n]:", 
                    svrinfo->settingurl);
            bool reset = readbool(ctx, reset);
            if(reset) {
                fz_free(ctx, svrinfo->settingurl);
                svrinfo->settingurl = null;
            }
        } 
        if(!svrinfo->settingurl)
            svrinfo->settingurl = readString(ctx, "sign server"); 
        if(!svrinfo->rooturl) 
            svrinfo->settingurl = fz_strdup(ctx, svrinfo->settingurl);
        isok = ntko_http_login(ctx, username, password,status, svrinfo, rights); 
    }
    fz_always(ctx) {
        if(username) fz_free(ctx, username);
        if(password) fz_free(ctx, password);
    }
    fz_catch(ctx) {
    }
    return isok; 
}


static
void on_error(fz_context *ctx, ntko_http_response_status *status) {
    printf("on_error:-------------\n");
    fz_try(ctx) {
        printf("http code:%d\n", status->code);
        if(status->data) {
            unsigned char *data = null;
            int size = 0; 
            size = fz_buffer_get_data(ctx, status->data, &data);
            if(data)
                printf("http response buffer:\n%s\n", data);

        }
    }
    fz_always(ctx){
    }
    fz_catch(ctx)
        fz_warn(ctx, "%s", fz_caught_message(ctx));
}

int main(int argc, char **argv) 
{
    ntko_server_info svrinfo;
    ntko_user_rights rights;
    ntko_sign_options options;
    ntko_http_response_status status;

    memset(&svrinfo, 0, sizeof(ntko_server_info));
    memset(&rights, 0, sizeof(rights));
    memset(&options, 0, sizeof(options));
    memset(&status, 0, sizeof(ntko_http_response_status));

    fz_context *ctx = fz_new_context(null, null, FZ_STORE_DEFAULT);
    int i = 0;

    bool loginok = false;

    fz_try(ctx) {
tag_login:
        while(!loginok && (i++)<3) {
            loginok = dologin(ctx, &svrinfo, &status, &rights); 
            if(!loginok)
                printf("login error:%s,left [%d] chance.\n", 
                        status.fail_reason, 3-i);
        } 
tag_after_login:
        if(loginok) {
            int i, count;
            char opname[64];
            memset(opname, 0, sizeof(opname));
            print_title("operations");
            count = sizeof(ops)/sizeof(ops[0]);
            for(i=0; i<count; i++)
                printf("%-8d%-16s\n", i, ops[i].opname);
            
            int sel = readint(ctx, -1);
            if(sel>=count) sel=-1;
            if(sel==-1) {
                printf("operation not defined! reselect\n");
                goto tag_after_login; 
            }

            ntko_op *selop = &ops[sel];
            bool isok = selop->doop(ctx,&svrinfo,&rights,&options,&status); 
            if(isok) goto tag_after_login; 
            else {
                if(status.code!=0 && status.fail_reason) {
                    printf("op faild:%s\n", status.fail_reason);
                    if(status.code==1) {
                        loginok = false;
                        goto tag_login;
                    }
                }
                printf("exit\n");
                goto tag_exit; 
            }
        } 
tag_exit:
        while(0);
    }
    fz_always(ctx) {

    }
    fz_catch(ctx) {
        fz_warn(ctx, "%s", fz_caught_message(ctx));
        on_error(ctx, &status);
    }


    fz_drop_context(ctx); 
}
