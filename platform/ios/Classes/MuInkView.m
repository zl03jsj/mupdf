#include "mupdf/_z/z_algorithm.h"
#include "common.h"
#import  "MuInkView.h"

@implementation MuInkView
{
	CGSize pageSize;
	NSMutableArray *curves;
	UIColor *color;
	z_point lastpoint;
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
	CGPoint p = [rec locationInView:self];
	p.x /= scale.width;
	p.y /= scale.height;
	UInt64 ms = [[NSDate date] timeIntervalSince1970]*1000;
	// line begin with is thin!!!
	z_point point = {p.x, p.y, ms};
	NSMutableArray *curve = nil;
	if (rec.state == UIGestureRecognizerStateBegan) {
		lastpoint = point;
		point.l = 0x20;
		[curves addObject:[NSMutableArray array]];
		curve = [curves lastObject];
		[curve addObject:[NSValue valueWithBytes:&point objCType:@encode(z_point)]];
		return;
	}
	curve = [curves lastObject];
	int count = (int)[curve count];
	int step = count>4 ? 0x40: 0x20;
	int w = z_get_point_width(lastpoint, point, step);
	z_point m = z_point_center(lastpoint, point, step);
	
	if( 1==count ) {
		m.l = w +
		[curve addObject:[NSValue valueWithBytes:&point objCType:@encode(z_point)]];
		lastpoint = point;
	}
	else {
		
	}
	
	
	
	
	
	
	
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
