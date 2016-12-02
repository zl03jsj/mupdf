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
#import "MuFileSelectView.h"
#import "MuAnnotSelectView.h"
#import "MuUpdater.h"

typedef enum MuSignStep_s {
	MuSignStep_not_start = 0,
	MuSignStep_get_appearance,
	MuSignStep_get_appearance_image,
	MuSignStep_get_appearance_position,
	MuSignStep_get_device,
	MuSignStep_cal_signdata,
	MuSignStep_save_data
} MuSignStep;

@protocol MuSignStepDelegate
- (void)signModeIntoStep: (MuSignStep)intostep laststep:(MuSignStep)laststep;
@end

@interface MuPageViewNormal : UIScrollView <UIScrollViewDelegate,MuPageView>

@property (retain, nonatomic) id<MuSignStepDelegate>  signstepDelegate;

- (id) initWithFrame: (CGRect)frame dialogCreator:(id<MuDialogCreator>)dia updater:(id<MuUpdater>)upd document: (MuDocRef *)aDoc page: (int)aNumber;
- (void) displayImage: (UIImage*)image;
- (void) resizeImage;
- (void) loadPage;
- (void) loadTile;

- (void) doNextSignStep;
@end
