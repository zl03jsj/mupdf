#import <UIKit/UIKit.h>

@interface MuInkView : UIView

@property(readonly) NSArray *curves;
@property(retain) UIColor *color;

/****************** retain means!!!!!
@property(retain) UIColor *color;
-(void)color{
	[color retain];
	return [color autorelease];
}
-(void)setColor:(UIColor*) newcolor{
	if( color = newcolor ){
		[color rlease];
		color = [newcolor retain];
	}
}
*************************/
- (id) initWithPageSize:(CGSize)pageSize;


@end
