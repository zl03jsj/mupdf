#import <UIKit/UIKit.h>

#undef ABS
#undef MIN
#undef MAX

#include "mupdf/fitz.h"

#import "MuHitView.h"
#import "MuPageView.h"
#import "MuDocRef.h"
#import "MuDialogCreator.h"
#import "MuTextSelectView.h"
#import "MuInkView.h"
#import "MuSignView.h"
#import "MuHanddrawView.h"
#import "MuAnnotSelectView.h"
#import "MuUpdater.h"
#import "NTKOPswCheckViewController.h"

@interface MuPageViewNormal : UIScrollView <UIScrollViewDelegate,MuPageView>
- (instancetype) initWithFrame: (CGRect)frame dialogCreator:(id<MuDialogCreator>)dia updater:(id<MuUpdater>)upd document: (MuDocRef *)aDoc page: (int)aNumber;
- (void) displayImage: (UIImage*)image;
- (void) resizeImage;
- (void) loadPage;
- (void) loadTile;
@end
