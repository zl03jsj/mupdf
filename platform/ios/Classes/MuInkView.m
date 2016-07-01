#include "common.h"
#import  "MuInkView.h"

@implementation MuInkView
{
	CGSize pageSize;
	NSMutableArray *curves;
	UIColor *color;
	
	CGPoint lastpoint;
	int64_t lastms;
	float   lastwidth;
	
	CGContextRef _imageContext;
	UIImage *_image;
}

- (BOOL) initImageContext{
	UIGraphicsBeginImageContext(self.frame.size);
	_imageContext = UIGraphicsGetCurrentContext();
	CGContextSetStrokeColorWithColor(_imageContext, [color CGColor]);
	CGSize scale = fitPageToScreen(pageSize, self.bounds.size);
	CGContextScaleCTM(_imageContext, scale.width, scale.height);
	[color set];
	CGContextSetLineCap(_imageContext,  kCGLineCapRound);
	CGContextSetLineJoin(_imageContext, kCGLineJoinRound);
	return YES;
}

- (id) initWithPageSize:(CGSize)_pageSize
{
	self = [super initWithFrame:CGRectMake(0, 0, 100, 100)];
	if (self) {
		[self setOpaque:NO];
		pageSize = _pageSize;
		color = [[UIColor colorWithRed:1.0 green:0.0 blue:0.0 alpha:0.8] retain];
		curves = [[NSMutableArray array] retain];
		UIPanGestureRecognizer *rec = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(onDrag:)];
		[self addGestureRecognizer:rec];
		[rec release];
		
		_imageContext = nil;
	}
	return self;
}

@synthesize curves;

-(void)dealloc
{
	[curves release];
	[color release];
	[super dealloc];
}

-(void) onDrag:(UIPanGestureRecognizer *)rec
{
	CGSize scale = fitPageToScreen(pageSize, self.bounds.size);
	CGPoint point = [rec locationInView:self];
	point.x /= scale.width;
	point.y /= scale.height;
	int64_t ms = [[NSDate date] timeIntervalSince1970]*1000;
	NSMutableArray *curve = nil;
	UIGestureRecognizerState curState = rec.state;
	if (curState == UIGestureRecognizerStateBegan) {
			[curves addObject:[NSMutableArray array]];
			curve = [curves lastObject];
			lastwidth = z_insertPoint(curve, lastpoint , lastms, lastwidth, point, ms);
	}
	else{
		curve = [curves lastObject];
		int lastIndex = (int)([curve count] - 1);
		if( curState == UIGestureRecognizerStateEnded){
			printf(" UIGestureRecognizerStateEnded\n");
			z_insertLastPoint(curve, lastpoint);
		}
		else { // UIGestureRecognizerStateChanged
			if( point.x==lastpoint.x && point.y==lastpoint.y) {
				return;
			}
			lastwidth = z_insertPoint(curve, lastpoint, lastms, lastwidth, point, ms);
		}
		if ( nil==_imageContext ){
			[self initImageContext];
		}
		[self setNeedsDisplay];
		/*
		CGRect r = [self drawCurrent:curve fromIndex:lastIndex];
		[self setNeedsDisplayInRect:r];
		 */
	}
	lastpoint = point;
	lastms = ms;
}

- (CGRect)drawCurrent : (NSMutableArray*)points fromIndex:(int)index{
	float max_width = 5.0f;
	float min_width = 1.0f;
	float w = max_width * z_get_stored_Width(points, index);
	if( w<min_width ) w = min_width;
	CGPoint point = z_get_stored_CGPoint(points, index);
	
	CGRect rect =  {point, CGSizeZero};
	
	CGContextMoveToPoint(_imageContext, point.x, point.y);
	CGContextSetLineWidth(_imageContext, w);
	index ++;
	for(; index<[points count]; index++){
		point = z_get_stored_CGPoint(points, index);
		CGContextAddLineToPoint(_imageContext, point.x, point.y);
		CGContextStrokePath(_imageContext);
		
		float w = max_width * z_get_stored_Width(points, index);
		if( w<min_width ) w = min_width;
		CGContextMoveToPoint(_imageContext, point.x, point.y);
		CGContextSetLineWidth(_imageContext, w);
		
		rect = CGRectExpendTo(rect, point);
	}
	_image = UIGraphicsGetImageFromCurrentImageContext();
	return rect;
}

- (void)drawRect:(CGRect)rect
{
	// [_image drawInRect:rect];
	//*
	float max_width = 5.0f;
	float min_width = 1.0f;
	CGContextRef cref = UIGraphicsGetCurrentContext();
	CGSize scale = fitPageToScreen(pageSize, self.bounds.size);
	CGContextScaleCTM(cref, scale.width, scale.height);
	[color set];
	CGContextSetLineCap(cref,  kCGLineCapRound);
	CGContextSetLineJoin(cref, kCGLineJoinRound);
	
	for (NSArray *curve in curves) {
		if (curve.count >= 2) {
			CGPoint pt= z_get_stored_CGPoint(curve, 0);
			float w = max_width * z_get_stored_Width(curve, 0);
			if( w<min_width ) w = min_width;
			CGContextSetLineWidth(cref, w);
			CGContextMoveToPoint (cref, pt.x, pt.y);
			for( int i=1; i<curve.count; i++) {
				pt = z_get_stored_CGPoint(curve, i);
				CGContextAddLineToPoint(cref, pt.x, pt.y);
				CGContextStrokePath(cref);
				
				w = max_width * z_get_stored_Width(curve, i);
				if( w<min_width ) w = min_width;
				CGContextMoveToPoint(cref, pt.x, pt.y);
				CGContextSetLineWidth(cref, w);
			}
		}
	 }
 // */
}

@end
