#import <UIKit/UIKit.h>

@interface MuInkView : UIView

@property(readonly) NSArray *curves;
@property(retain) UIColor *color;

- (instancetype) initWithPageSize:(CGSize)pageSize;

@end
