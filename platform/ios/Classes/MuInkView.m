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
	InsertPoint(curves, lastpoint, lastms, lastwidth, point, ms);
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

	for (NSArray *curve in curves)
	{
		if (curve.count >= 2)
		{
			CGPoint pt = [[curve objectAtIndex:0] CGPointValue];
			CGContextBeginPath(cref);
			CGContextMoveToPoint(cref, pt.x, pt.y);
			CGPoint lpt = pt;

			for (int i = 1; i < curve.count; i++)
			{
				pt = [[curve objectAtIndex:i] CGPointValue];
				CGContextAddQuadCurveToPoint(cref, lpt.x, lpt.y, (pt.x + lpt.x)/2, (pt.y + lpt.y)/2);
				lpt = pt;
			}

			CGContextAddLineToPoint(cref, pt.x, pt.y);
			CGContextStrokePath(cref);
		}
	}
}

@end
