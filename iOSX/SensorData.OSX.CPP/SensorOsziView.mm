/**
 * SensorOsziView
 * ------------------------------------------------------------------
 * Copyright (c) Chi-Tai Dang
 *
 * @author	Chi-Tai Dang
 * @version	1.0
 * @remarks
 *
 * This file is part of the Environs framework developed at the
 * Lab for Human Centered Multimedia of the University of Augsburg.
 * http://hcm-lab.de/environs
 *
 * Environ is free software; you can redistribute it and/or modify
 * it under the terms of the Eclipse Public License v1.0.
 * A copy of the license may be obtained at:
 * http://www.eclipse.org/org/documents/epl-v10.html
 * --------------------------------------------------------------------
 */

/// Compiler flag that enables verbose debug output
#ifndef NDEBUG
//#define DEBUGVERB
//#define DEBUGVERBVerb
#endif

//#define RANDOM_GRAPH

#import "SensorOsziView.h"

#include "Environs.Native.h"
#include <cmath>

using namespace environs;

#define CLASS_NAME  "SensorOsziView"

#define TITLE_SIZE 12

@interface SensorOsziView ()
{
    NSLock * valuesLock;
    
    NSString * title;
    
    int xMax;
    int yRangeUpper;
    int xCurrent;
    bool curUpdated;
    
    int * values;
    int yMax;
    int yMidline;
    int yRangeHalfInPixel;
    int yOffset;

    float halfYRange;
    
    float yRangeMax;
    float curValue;
}

@end


@implementation SensorOsziView


- (id) initWithCoder:(NSCoder *)coder
{
    self = [super initWithCoder:coder];
    if (self != nil) {
        
        if ( valuesLock == nil ) {
            
            valuesLock = [[NSLock alloc] init];
            
            title = @"Unknown";
            
            [self CalculateOsziDimensions];
            
            //values = nil;
            values = new int[xMax];
            
            for ( int i=0; i < xMax; i++ )
#ifdef RANDOM_GRAPH
                values [i] = ((rand()%(int)halfYRange) * 2) + TITLE_SIZE;
#else
                values [i] = yMidline + 5;
#endif
            yRangeMax = 1.0f;
            
            xCurrent = 0;
            curUpdated = false;
        }
    }
    
    return self;
}


- (void) ResetY
{
    yRangeMax = 1.0f;
}


- (void) CalculateOsziDimensions
{
    [valuesLock lock];
    
    NSRect rect = [self frame];
    
    yMax = rect.size.height;
    /*
    if ( values )
        free ( values);
    values = (int *) malloc (xMax * sizeof(int));
    */
    
    yRangeUpper = yMax - TITLE_SIZE;
    
    xMax = rect.size.width;
    
    yMidline = rect.size.height / 2;
    yRangeHalfInPixel = yMidline - TITLE_SIZE;
    
    yOffset = TITLE_SIZE;
    
    halfYRange = yMidline - TITLE_SIZE;
    
    [valuesLock unlock];
}


- (void) SetTitle:(NSString *) value
{
    title = value;
}


- (void) UpdateValue:(float) value
{
    [valuesLock lock];
    
    float yValue = std::abs(value);
    if ( yValue > yRangeMax )
    {
        // Recalculate graph values
        float yRangeMaxNew = yValue * 1.6;
        
        for ( int i=0; i < xMax; i++ ) {
            int o = values [i];

            int v;
            if ( o >= yMidline )
                v = o - yMidline;
            else
                v = yMidline - o;
            
            float base = ((float)v * yRangeMax) / yRangeHalfInPixel;
            
            float newValue = (base * yRangeHalfInPixel) / yRangeMaxNew;
            if ( o >= yMidline )
                values [i] = yMidline + newValue;
            else
                values [i] = yMidline - newValue;
        }
        yRangeMax = yRangeMaxNew;
    }
    
    yValue = (yValue * yRangeHalfInPixel) / yRangeMax;
    if ( value < 0 )
        values [xCurrent] = yMidline - yValue;
    else
        values [xCurrent] = yMidline + yValue;
    
    curValue = value;
    curUpdated = true;
    
    [valuesLock unlock];
}


- (void) IncreaseTimer
{
    if ( !values )
        return;
    
    [valuesLock lock];
    
    if ( !curUpdated ) {

        if ( xCurrent == 0 ) {
            values [ xCurrent ] = values [ xMax - 1 ];
        }
        else {
            values [ xCurrent ] = values [ xCurrent - 1 ];
        }
    }
    /*if ( [title containsString:@"Z"] ) {
        int i=0;
        i++;
        NSLog (@"Z: %i:%i", xCurrent, values [ xCurrent ]);
    }
    */
    
    xCurrent++;
    if (xCurrent >= xMax)
        xCurrent = 0;
    
    curUpdated = false;
    [valuesLock unlock];
    
    dispatch_sync(dispatch_get_main_queue(), ^{
        [self setNeedsDisplay:YES];
    });
}


- (void) drawRect:(NSRect)pNSRect
{
    // Draw black background
    [[NSColor blackColor] set];
    NSRectFill ( pNSRect );
    
    NSBezierPath * line = [NSBezierPath bezierPath];
    [line moveToPoint:NSMakePoint(NSMinX([self bounds]), NSMaxY([self bounds]) / 2)];
    [line lineToPoint:NSMakePoint(NSMaxX([self bounds]), NSMaxY([self bounds]) / 2)];
    
    [line setLineWidth:1.0];
    [[NSColor blueColor] set];
    [line stroke];
    
    
    if (values) {
        [valuesLock lock];
        
        NSBezierPath * signalBefore = [NSBezierPath bezierPath];
        int y = values[0];
        
        [signalBefore moveToPoint:NSMakePoint(0, y)];
        
        for ( int i=1; i<=xCurrent; i++ ) {
            [signalBefore lineToPoint:NSMakePoint(i, values [i])];
        }
        
        [signalBefore setLineWidth:1.0];
        [[NSColor greenColor] set];
        [signalBefore stroke];
        
        NSBezierPath * signalAfter = [NSBezierPath bezierPath];
        
        
        
        for ( int i=xCurrent+2; i<xMax; i++ ) {
            if ( i == xCurrent+2 ) {
                y = values[i];
                [signalAfter moveToPoint:NSMakePoint(i, y)];
            }
            else {
                [signalAfter lineToPoint:NSMakePoint(i, values [i])];
            }
        }
        
        [signalAfter setLineWidth:1.0];
        [[NSColor grayColor] set];
        [signalAfter stroke];
        
        [valuesLock unlock];
    }
    
    NSDictionary *attributes = [NSDictionary dictionaryWithObjectsAndKeys:[NSFont fontWithName:@"Helvetica" size:TITLE_SIZE], NSFontAttributeName,[NSColor yellowColor], NSForegroundColorAttributeName, nil];
    
    // Draw title
    NSAttributedString * currentText=[[NSAttributedString alloc] initWithString:title attributes: attributes];
    
    [currentText drawAtPoint:NSMakePoint((xMax / 2) - (currentText.size.width / 2), 0)];

    
    attributes = [NSDictionary dictionaryWithObjectsAndKeys:[NSFont fontWithName:@"Helvetica" size:TITLE_SIZE - 2], NSFontAttributeName,[NSColor yellowColor], NSForegroundColorAttributeName, nil];
    
    // Draw y range values
    currentText=[[NSAttributedString alloc] initWithString:[[NSString alloc ] initWithFormat:@"-%.2f", yRangeMax] attributes: attributes];
    
    [currentText drawAtPoint:NSMakePoint(0, 0)];
    
    currentText=[[NSAttributedString alloc] initWithString:[[NSString alloc ] initWithFormat:@"%.2f", yRangeMax] attributes: attributes];
    
    [currentText drawAtPoint:NSMakePoint(0, yRangeUpper)];
    
    currentText=[[NSAttributedString alloc] initWithString:[[NSString alloc ] initWithFormat:@"%.2f", curValue] attributes: attributes];
    
    [currentText drawAtPoint:NSMakePoint(xMax - currentText.size.width, yRangeUpper)];
}


@end











