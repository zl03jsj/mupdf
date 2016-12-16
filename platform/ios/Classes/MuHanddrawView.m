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
	UIColor *_color;
	CGPoint _lastpoint;
	int64_t _lastms;
	float   _lastwidth;
	CGContextRef _imageContext;
	UIImage *_image;
	z_fpoint_arraylist *_strokes;
	float _maxwidth;
	float _minwidth;
}

- (z_fpoint_arraylist*)getKeepStrokes {
	if(_strokes) {
		return z_keep_fpoint_arraylist(ctx, _strokes);
	}
	return NULL;
}

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
		_strokes = NULL; // z_new_fpoint_arraylist(ctx);
		UIPanGestureRecognizer *rec = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(onDrag:)];
		[self addGestureRecognizer:rec];
		[rec release];
		_imageContext = nil;
		_image = nil;
		_strokebounds = CGRectZero;
		_maxwidth = 7.0f;
		_minwidth = 1.0f;
	}
	return self;
}

-(void)dealloc
{
	UIGraphicsEndImageContext();
	if( nil!=_image ) [_image release];
	if(_strokes) z_drop_fpoint_arraylist(ctx, _strokes);
	[_color release];
	[super dealloc];
}

-(void) refreshWidth:(z_fpoint_array*)a fromindex:(int)index {
	z_fpoint *zfp;
	for(; index<a->len; index++) {
		zfp = (a->point + index);
		zfp->w = fmaxf(_maxwidth * zfp->w, _minwidth);
	}
}

-(void) onDrag:(UIPanGestureRecognizer *)rec
{
	CGSize scale = fitPageToScreen(_pagesize, self.bounds.size);
	CGPoint point = [rec locationInView:self];
	point.x /= scale.width;
	point.y /= scale.height;
	int64_t ms = [[NSDate date] timeIntervalSince1970]*1000;
	z_fpoint_array *stroke = NULL;
	UIGestureRecognizerState curState = rec.state;
	
	if(!_strokes)
		_strokes = z_new_fpoint_arraylist(ctx);
	
	if (curState == UIGestureRecognizerStateBegan) {
		stroke = z_fpoint_arraylist_append_new(ctx, _strokes, _maxwidth, _minwidth);
		_lastwidth = z_cg_insertPoint(ctx, stroke, _lastpoint, _lastms, _lastwidth, point, ms);
		_strokebounds.origin = point;
	}
	else{
		stroke = _strokes->end->a;
		int lastIndex = stroke->len - 1;
		if( curState == UIGestureRecognizerStateEnded){
			z_cg_insertLastPoint(ctx, stroke, point);
			// [self refreshWidth:stroke fromindex:0];
		}
		else { // UIGestureRecognizerStateChanged
			if( (ms-_lastms) < 35 || 4>z_cg_distance(point, _lastpoint) )
				return;
			_lastwidth = z_cg_insertPoint(ctx, stroke, _lastpoint, _lastms, _lastwidth, point, ms);
		}
		// [self refreshWidth:stroke fromindex:lastIndex+1];
#if 0
		NSLog(@"array(len=%d,  ref=%d)", stroke->len, stroke->ref);

		NSLog(@"before rect expand:rect(x=%d,y=%d,w=%d,h=%d), point(x=%d,y=%d)",(int)_strokebounds.origin.x, (int)_strokebounds.origin.y,
			  (int)_strokebounds.size.width, (int)_strokebounds.size.height, (int)point.x, (int)point.y);
#endif
		_strokebounds = z_CGRectExpandToPoint(_strokebounds, point);
#if 0
		NSLog(@"after rect expand:rect(x=%d,y=%d,w=%d,h=%d)",(int)_strokebounds.origin.x, (int)_strokebounds.origin.y,
			  (int)_strokebounds.size.width, (int)_strokebounds.size.height);
#endif
		
		if ( nil==_imageContext ) [self initImageContext];
#if 1	// update all uiview rectangle
		[self drawCurrent:stroke fromIndex:lastIndex];
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
- (CGRect)drawCurrent : (z_fpoint_array*)arr fromIndex:(int)index{
	z_fpoint *zfpoint = (arr->point+index);
	fz_point point = zfpoint->p;
	
	float w = fmaxf(_maxwidth * zfpoint->w, _minwidth);
	
	fz_rect rect = {point.x, point.y, point.x, point.y};
	
	CGContextMoveToPoint(_imageContext, point.x, point.y);
	CGContextSetLineWidth(_imageContext, w);
	index ++;
	
	for(; index<arr->len; index++) {
		zfpoint = arr->point + index;
		point = zfpoint->p;
		CGContextAddLineToPoint(_imageContext, point.x, point.y);
		CGContextStrokePath(_imageContext);
		
		w = fmaxf(_maxwidth * zfpoint->w, _minwidth);
		
		CGContextMoveToPoint(_imageContext, point.x, point.y);
		CGContextSetLineWidth(_imageContext, w);
		
		fz_include_point_in_rect(&rect, &point);
	}
//	fz_expand_rect(&rect, max_width);
	_image = UIGraphicsGetImageFromCurrentImageContext();
	[_image retain];
	
	return CGRectMake(fminf(rect.x0, rect.x1), fminf(rect.y0, rect.y1),
					  fabsf(rect.x1-rect.x0 ), fabsf(rect.y1-rect.y0 ));
}

- (void)drawRect:(CGRect)rect
{
	if( nil==_image ) return;
	[_image drawInRect:rect blendMode:kCGBlendModeCopy alpha:0.7];
	[_image release]; _image = nil;
}

@end
