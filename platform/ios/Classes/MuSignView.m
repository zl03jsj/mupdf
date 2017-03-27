//
//  MuSignView.m
//  MxPDF
//
//  Created by 曾亮 on 2016/11/22.
//  Copyright © 2016年 Artifex Software, Inc. All rights reserved.
//

#include "common.h"
#import "MuSignView.h"
#import "ntko/NTKOTableDs.h"

@implementation MuSignView
{
	CGSize _pagesize;
	CGPoint _startpoint;
	CGPoint _endpoint;
	UIColor *_color;

	BOOL _moveModeon;
	CGPoint _lasttouchpoint;
	UIImage *_image;
	BOOL _firstLayoutsubview;
}

-(CGRect) getRectOfPage {
	return CGRectMake(fz_min(_startpoint.x,_endpoint.x),
					  fz_min(_startpoint.y,_endpoint.y),
					  fz_abs(_startpoint.x-_endpoint.x),
					  fz_abs(_startpoint.y-_endpoint.y) );
}

-(CGRect) getMoveRect {
	return self.rectOfPage;
}

-(instancetype) initWithPageSize:(CGSize)pagesize
{
	self = [super initWithFrame:CGRectMake(0, 0, 100, 100)];
	if (self) {
		[self setOpaque:NO];
		_pagesize = pagesize;
		_color = [[UIColor colorWithRed:1.0 green:0.0 blue:0.0 alpha:0.8] retain];
		_moveModeon = false;
		_firstLayoutsubview = true;

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
	CGRect rect = self.rectOfPage;
	if( CGRectIsEmpty(rect) ) return;

	CGSize scale = fitPageToScreen(_pagesize, self.bounds.size);
	CGPoint point = [rec locationInView:self];
	point.x /= scale.width;
	point.y /= scale.height;
	if( !CGRectContainsPoint(rect, point) ){
//		_startpoint = _endpoint = point;
//		[self setNeedsLayout];
//		[_delegate imagePositionNotOk];
	}
	else {
	}
}

- (void) refreshImageRect {
	if(!_image || !CGRectIsEmpty(self.rectOfPage))
		return;
	CGSize size = self.bounds.size;
	CGSize imagesize = _image.size;
	
	CGSize scale = fitPageToScreen(_pagesize, size);
	size.width /= scale.width;
	size.height/= scale.height;
	
	// imagesize.width /= scale.width;
	// imagesize.height/= scale.height;
	float f = imagesize.width / imagesize.height;
	imagesize.width = size.width / 4;
	imagesize.height = imagesize.width / f;
	
	_startpoint.x = (size.width - imagesize.width) / 2;
	_startpoint.y = (size.height- imagesize.height)/ 2;
	_endpoint.x = _startpoint.x + imagesize.width;
	_endpoint.y = _startpoint.y + imagesize.height;
}

- (void)layoutSubviews {
	[super layoutSubviews];
	[self refreshImageRect];
	_firstLayoutsubview = false;
}

-(void) onDrag:(UIPanGestureRecognizer *)rec
{
	CGSize scale = fitPageToScreen(_pagesize, self.bounds.size);
	CGPoint point = [rec locationInView:self];
	point.x /= scale.width;
	point.y /= scale.height;
	
	CGRect rect = self.rectOfPage;
	if (rec.state == UIGestureRecognizerStateBegan) {
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
	
	if( _delegate ) {
		rect = self.rectOfPage;
		if (CGRectIsEmpty(rect)) [_delegate imagePositionNotOk];
		else [_delegate imagePositionOk:rect];
	}
	[self setNeedsDisplay];
}

- (void)setSignFile:(id<NTKOTableDs>)signfile {
	if([_signfile.title isEqualToString:_signfile.title])
		return;
	if(_signfile) [_signfile release];
	_signfile = [signfile retain];
	
	if(_image) [_image release];
	_image = [_signfile.image retain];
	
	if(!_firstLayoutsubview)
		[self refreshImageRect];
}

- (void)drawRect:(CGRect)rect
{
	CGRect imagerect = self.rectOfPage;
	if(CGRectIsEmpty(imagerect)) return;

	CGSize scale = fitPageToScreen(_pagesize, self.bounds.size);
	CGContextRef context = UIGraphicsGetCurrentContext();
	
	CGContextScaleCTM(context, scale.width, scale.height);

	CGContextSetLineCap(context,  kCGLineCapRound);
	CGContextSetLineJoin(context, kCGLineJoinRound);
	CGContextSetLineWidth(context, 2.0f);
	[_color setStroke];
	
	CGFloat lengths[] = {2,4};
	CGContextSetLineDash(context, 0, lengths, 2.0f);
	
	if(_image) [_image drawInRect: imagerect];

	CGContextAddRect(context, imagerect);
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
	NSLog(@"%@ was dealloced", [self class]);
	[_signfile release];
	[_image release];
	[_color release];
	[super dealloc];
}

@end
