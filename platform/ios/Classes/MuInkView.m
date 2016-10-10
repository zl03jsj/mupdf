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
	UIGraphicsBeginImageContextWithOptions(self.bounds.size, self.opaque, 0.0);
	_imageContext = UIGraphicsGetCurrentContext();
	// CGContextSetStrokeColorWithColor(_imageContext, [color CGColor]);
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
		color = [[UIColor colorWithRed:1.0 green:0.0 blue:0.0 alpha:1.0] retain];
		curves = [[NSMutableArray array] retain];
		UIPanGestureRecognizer *rec = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(onDrag:)];
		[self addGestureRecognizer:rec];
		[rec release];
		
		_imageContext = nil;
		_image = nil;
	}
	return self;
}

@synthesize curves;
@synthesize color = color;

-(void)dealloc
{
	UIGraphicsEndImageContext();
	if( nil!=_image ) [_image release];
	// if( nil!=_imageContext) [_image release];
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
			z_insertLastPoint(curve, point);
		}
		else { // UIGestureRecognizerStateChanged
			if( (ms - lastms) < 35 ){
				return;
			}
			if( 3>distanceBetweenPoints(point, lastpoint) ){
				return;
			}
			lastwidth = z_insertPoint(curve, lastpoint, lastms, lastwidth, point, ms);
		}
		if ( nil==_imageContext ){
			[self initImageContext];
		}
		[self drawCurrent:curve fromIndex:lastIndex];
		// [self setNeedsDisplayInRect:r];
		[self setNeedsDisplay];
	}
	lastpoint = point;
	lastms = ms;
}

// returns the rect need be redraw!!
- (CGRect)drawCurrent : (NSMutableArray*)points fromIndex:(int)index{
	float max_width = 7.0f;
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
	rect = CGRectInset(rect, -max_width, -max_width);
	_image = UIGraphicsGetImageFromCurrentImageContext();
	[_image retain];
	return rect;
}

- (void)drawRect:(CGRect)rect
{
	if( nil==_image ) return;
	// use CGImageCreateWithImageInRect get selected area!!
	[_image drawInRect:rect blendMode:kCGBlendModeCopy alpha:0.7];
	[_image release]; _image = nil;
	/*
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
