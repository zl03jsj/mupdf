//
//  MuHanddrawView.m
//  MxPDF
//
//  Created by 曾亮 on 2016/11/23.
//  Copyright © 2016年 Artifex Software, Inc. All rights reserved.
//

#include "common.h"
#import "MuHanddrawView.h"

@implementation MuHanddrawView
{
	CGSize _pagesize;
	NSMutableArray * _curves;
	UIColor *_color;
	
	CGPoint _lastpoint;
	int64_t _lastms;
	float   _lastwidth;
	
	CGContextRef _imageContext;
	UIImage *_image;
}

@synthesize curves = _curves;

- (BOOL) initImageContext{
	UIGraphicsBeginImageContextWithOptions(self.bounds.size, self.opaque, 0.0);
	_imageContext = UIGraphicsGetCurrentContext();
	CGSize scale = fitPageToScreen(_pagesize, self.bounds.size);
	CGContextScaleCTM(_imageContext, scale.width, scale.height);
	[_color setStroke];
	CGContextSetLineCap(_imageContext,  kCGLineCapRound);
	CGContextSetLineJoin(_imageContext, kCGLineJoinRound);
	return YES;
}

- (id) initWithPageSize:(CGSize)pagesize
{
	self = [super initWithFrame:CGRectMake(0, 0, 100, 100)];
	if (self) {
		[self setOpaque:NO];
		_pagesize = pagesize;
		_color = [[UIColor colorWithRed:1.0 green:0.0 blue:0.0 alpha:1.0] retain];
		_curves = [[NSMutableArray array] retain];
		UIPanGestureRecognizer *rec = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(onDrag:)];
		[self addGestureRecognizer:rec];
		[rec release];
		
		_imageContext = nil;
		_image = nil;
	}
	return self;
}

-(void)dealloc
{
	UIGraphicsEndImageContext();
	if( nil!=_image ) [_image release];
	// if( nil!=_imageContext) [_image release];
	[_curves release];
	[_color release];
	[super dealloc];
}

-(void) onDrag:(UIPanGestureRecognizer *)rec
{
	CGSize scale = fitPageToScreen(_pagesize, self.bounds.size);
	CGPoint point = [rec locationInView:self];
	point.x /= scale.width;
	point.y /= scale.height;
	int64_t ms = [[NSDate date] timeIntervalSince1970]*1000;
	NSMutableArray *curve = nil;
	UIGestureRecognizerState curState = rec.state;
	
	if (curState == UIGestureRecognizerStateBegan) {
		[_curves addObject:[NSMutableArray array]];
		curve = [_curves lastObject];
		_lastwidth = z_IOS_insertPoint(curve, _lastpoint , _lastms, _lastwidth, point, ms);
	}
	else{
		curve = [_curves lastObject];
		int lastIndex = (int)([curve count] - 1);
		if( curState == UIGestureRecognizerStateEnded){
			z_IOS_insertLastPoint(curve, point);
		}
		else { // UIGestureRecognizerStateChanged
			if( (ms - _lastms) < 35 ){
				return;
			}
			if( 3>distanceBetweenPoints(point, _lastpoint) ){
				return;
			}
			_lastwidth = z_IOS_insertPoint(curve, _lastpoint, _lastms, _lastwidth, point, ms);
		}
		if ( nil==_imageContext ){
			[self initImageContext];
		}
		
#if 0	// update all uiview rectangle
		[self drawCurrent:curve fromIndex:lastIndex];
		[self setNeedsDisplay];
#else	// only update changed rect
		CGRect updaterect = [self drawCurrent:curve fromIndex:lastIndex];
		[self setNeedsDisplayInRect:updaterect];
#endif
	}
	_lastpoint = point;
	_lastms = ms;
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
	[_image drawInRect:rect blendMode:kCGBlendModeCopy alpha:0.7];
	[_image release]; _image = nil;
}

@end
