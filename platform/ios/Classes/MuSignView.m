//
//  MuSignView.m
//  MxPDF
//
//  Created by 曾亮 on 2016/11/22.
//  Copyright © 2016年 Artifex Software, Inc. All rights reserved.
//

#include "common.h"
#import "MuSignView.h"

@implementation MuSignView
{
	CGSize _pagesize;
	CGPoint _startpoint;
	CGPoint _endpoint;
	UIColor *_color;
	// for move signrect
	BOOL _moveModeon;
	CGPoint _lasttouchpoint;
	UIImage *_image;
}

-(CGRect) getSignrect {
	return CGRectMake(fz_min(_startpoint.x,_endpoint.x),
					  fz_min(_startpoint.y,_endpoint.y),
					  fz_abs(_startpoint.x-_endpoint.x),
					  fz_abs(_startpoint.y-_endpoint.y) );
}

-(CGRect) getMoveRect {
	CGRect moverect = self.signrect;
	moverect.origin.x += moverect.size.width /4;
	moverect.origin.y += moverect.size.height/4;
	moverect.size.width = moverect.size.width /2;
	moverect.size.height= moverect.size.height/2;
	return moverect;
}

-(instancetype) initWithPageSize:(CGSize)pagesize
{
	self = [super initWithFrame:CGRectMake(0, 0, 100, 100)];
	if (self) {
		[self setOpaque:NO];
		_pagesize = pagesize;
		_color = [[UIColor colorWithRed:1.0 green:0.0 blue:0.0 alpha:0.8] retain];
		_moveModeon = false;

		UIPanGestureRecognizer *rec = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(onDrag:)];
		[self addGestureRecognizer:rec];
		[rec release];
		
		// add single Finger tap event
		UITapGestureRecognizer *singleFingerTap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(handleSingleTap:)];
		[self addGestureRecognizer:singleFingerTap];
		[singleFingerTap release];
	}
	return self;
}

//The event handling method
- (void)handleSingleTap:(UITapGestureRecognizer *)rec {
	CGRect rect = self.signrect;
	if( CGRectIsEmpty(rect) ) return;

	CGSize scale = fitPageToScreen(_pagesize, self.bounds.size);
	CGPoint point = [rec locationInView:self];
	point.x /= scale.width;
	point.y /= scale.height;
	if( !CGRectContainsPoint(rect, point) ){
		_startpoint = _endpoint = CGPointZero;
		[self setNeedsDisplay];
	}
	else {
	}
}

-(void) onDrag:(UIPanGestureRecognizer *)rec
{
	NSLog(@"onDrag");
	CGSize scale = fitPageToScreen(_pagesize, self.bounds.size);
	CGPoint point = [rec locationInView:self];
	point.x /= scale.width;
	point.y /= scale.height;
	
	if (rec.state == UIGestureRecognizerStateBegan) {
		CGRect rect = [self getMoveRect]; //self.signrect;
		if( !CGRectIsEmpty(rect) && CGRectContainsPoint(rect, point)) {
			_moveModeon = YES;
			_lasttouchpoint = point;
		}
		else{
			_startpoint = _endpoint = point;
			_moveModeon = NO;
		}
	}
	else{
		if(YES==_moveModeon) {
			float x = point.x - _lasttouchpoint.x;
			float y = point.y - _lasttouchpoint.y;
			_startpoint.x += x;
			_startpoint.y += y;
			_endpoint.x += x;
			_endpoint.y += y;
			_lasttouchpoint = point;
		}
		else {
			_endpoint = point;
		}
	}
	[self setNeedsDisplay];
}

- (void)drawRect:(CGRect)rect
{
	if(CGPointEqualToPoint(_startpoint, _endpoint))
		return;
	
	CGSize scale = fitPageToScreen(_pagesize, self.bounds.size);
	CGContextRef context = UIGraphicsGetCurrentContext();
	CGContextScaleCTM(context, scale.width, scale.height);

	CGContextSetLineCap(context,  kCGLineCapRound);
	CGContextSetLineJoin(context, kCGLineJoinRound);
	CGContextSetLineWidth(context, 2.0f);
	[_color setStroke];
	
	CGFloat lengths[] = {2,4};
	CGContextSetLineDash(context, 0, lengths, 2.0f);
	
	CGContextAddRect(context, self.signrect);
	CGContextStrokePath(context);

	CGRect moverect = [self getMoveRect];
	CGContextBeginPath(UIGraphicsGetCurrentContext());
	CGContextMoveToPoint(context, moverect.origin.x,
						 moverect.origin.y+(moverect.size.height/2));
	CGContextAddLineToPoint(context,moverect.origin.x+moverect.size.width,
							moverect.origin.y+(moverect.size.height/2) );
	CGContextMoveToPoint(context, moverect.origin.x+(moverect.size.width/2),
						 moverect.origin.y);
	CGContextAddLineToPoint(context,moverect.origin.x+(moverect.size.width/2),
							moverect.origin.y+moverect.size.height );
	CGContextStrokePath(context);
}

-(void) dealloc {
	[super dealloc];
	[_imagefile release];
	[_color release];
}

@end
