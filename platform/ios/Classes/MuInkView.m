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
	float penWidth;
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
	if (rec.state == UIGestureRecognizerStateBegan) {
		[curves addObject:[NSMutableArray array]];
		curve = [curves lastObject];
	}
	lastwidth = z_insertPoint(curve, lastpoint, lastms, lastwidth, point, ms);
	lastpoint = point;
	lastms = ms;
	
	[self setNeedsDisplay];
}

- (void)drawRect:(CGRect)rect
{
	CGSize scale = fitPageToScreen(pageSize, self.bounds.size);
	CGContextRef cref = UIGraphicsGetCurrentContext();
	CGContextScaleCTM(cref, scale.width, scale.height);

	[color set];
	CGContextSetLineWidth(cref, 5.0);
	float max_width = 5.0f;
	for (NSArray *curve in curves)
	{
		if (curve.count >= 2) {
			CGPoint pt= z_get_stored_CGPoint(curve, 0);
			float w = max_width * z_get_stored_Width(curve, 0);
			CGContextSetLineWidth(cref, w);
			CGContextBeginPath(cref);
			CGContextMoveToPoint(cref, pt.x, pt.y);
			for( int i=1; i<curve.count; i++) {
				pt = z_get_stored_CGPoint(curve, i);
				w = max_width * z_get_stored_Width(curve, 0);
				CGContextSetLineWidth(cref, w);
				CGContextAddLineToPoint(cref, pt.x, pt.y);
			}
			CGContextStrokePath(cref);
		}
	}
}

@end
