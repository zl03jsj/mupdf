#import <UIKit/UIKit.h>

#undef ABS
#undef MIN
#undef MAX

#include "mupdf/fitz.h"

#import "MuOutlineController.h"
#import "MuDocRef.h"
#import "MuDialogCreator.h"
#import "MuUpdater.h"
#import "MuFileselectViewController.h"
#import "NTKOPswCheckViewController.h"

enum
{
	BARMODE_MAIN,
	BARMODE_SEARCH,
	BARMODE_MORE,
	BARMODE_ANNOTATION,
	BARMODE_HIGHLIGHT,
	BARMODE_UNDERLINE,
	BARMODE_STRIKE,
	BARMODE_INK,
	BARMODE_Handsign,
	BARMODE_SIGN,
	BARMODE_DELETE
};

typedef enum Signstep_s {
	SIGN_STEP_NOT_SIGINING,
	SIGN_STEP_CHOOSE_IMAGEFILE,
	SIGN_STEP_SVR_LOGIN,
	SIGN_STEP_OPEN_ESP,
	SIGN_STEP_OPEN_PFX,
	SIGN_STEP_DRAW_SIGN_IMAGE,
	SIGN_STEP_HAND_DRAW_APPEARANCE,
	SIGN_STEP_GET_SIGN_POSITION,
	SIGN_STEP_GET_SIGN_DEVICE,
	SIGN_SETP_ADD_SIGN,
	SIGN_STEP_SAVE_SIGNATURE
} Signstep;

@interface MuDocumentController : UIViewController <UIScrollViewDelegate, UIGestureRecognizerDelegate, UISearchBarDelegate, MuDialogCreator, MuUpdater, MuFileSelectViewDelegate, NTKOPswCheckViewDelegate>
- (instancetype) initWithFilename: (NSString*)nsfilename path:(NSString *)path document:(MuDocRef *)aDoc;
- (void) createPageView: (int)number;
- (void) gotoPage: (int)number animated: (BOOL)animated;
- (void) onShowOutline: (id)sender;
- (void) onShowSearch: (id)sender;
- (void) onCancel: (id)sender;
- (void) resetSearch;
- (void) showSearchResults: (int)count forPage: (int)number;
- (void) onSlide: (id)sender;
- (void) onTap: (UITapGestureRecognizer*)sender;
- (void) showNavigationBar;
- (void) hideNavigationBar;
@end
