#import <UIKit/UIKit.h>
#import "MuTapResult.h"
#import "MuSignView.h"
#import "MuHanddrawView.h"

@protocol NTKOTableDs;

@protocol MuPageView
@property (NS_NONATOMIC_IOSONLY, readonly) int number;
-(void) willRotate;
-(void) showLinks;
-(void) hideLinks;
-(void) showSearchResults: (int)count;
-(void) clearSearchResults;
-(void) resetZoomAnimated: (BOOL)animated;
-(void) setScale:(float)scale;
-(MuTapResult *) handleTap:(CGPoint)pt;
-(void) textSelectModeOn;
-(void) textSelectModeOff;
-(void) deselectAnnotation;
-(void) deleteSelectedAnnotation;
-(void) inkModeOn;
-(void) inkModeOff;

-(void) saveSelectionAsMarkup:(int)type;
-(void) saveInk;
// -(void) saveContentStream;
-(void) update;

// add by zl[2016/11/16 17:30]
// for add signature
@optional
-(void) imageViewModeOn:(id<NTKOTableDs>)ds;
-(void) imageViewModeOff;
@property (nonatomic,readonly, retain) MuSignView* signView;

-(void) handsignModeOn;
-(void) handsignModeOff;
@property (nonatomic,readonly,retain) MuHanddrawView* darwView;

- (void)addsign:(z_pdf_sign_appearance *)app signdevice:(z_device *)device;
@end
