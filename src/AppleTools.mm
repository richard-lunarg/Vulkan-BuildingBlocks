/*
 * Many system level features have to use Objective C Apple speciic functions. This
 * file contains a set of C callable functions that are implemented in Objective C.

    Copyright 2022, Starstone Software Systems, Inc.
    All Rights Reserved
    Richard S. Wright Jr.
    rwright@starstonesoftware.com
 */

#include "AppleTools.h"

#import <Foundation/Foundation.h>
#import <QuartzCore/CAMetalLayer.h>

#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#include <UIKit/UIView.h>
#else
#import <AppKit/AppKit.h>
#endif

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>


int detectDarkTheme(void) {

    NSString *osxMode = (NSString *)[[NSUserDefaults standardUserDefaults] stringArrayForKey:@"AppleInterfaceStyle"];

    // This is either nil or @Dark
    if(osxMode == nil)
        return 0;

    return 1;
    }


void *makeViewMetalCompatible(void* handle)
{
#if TARGET_OS_IPHONE
    UIView* view = (__bridge UIView*)handle;
    assert([view isKindOfClass:[UIView class]]);

    void *pLayer =(__bridge void*)view.layer;
    return pLayer;
#else
    NSView* view = (__bridge NSView*)handle;
    assert([view isKindOfClass:[NSView class]]);

    void *pLayer = (__bridge void *)view.layer;
    return pLayer;
#endif
}



// NSArray *docPath = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
// NSString *myPath = [docPath objectAtIndex:0];
// NSString *pPlannerLoc = [myPath stringByAppendingPathComponent:@"/SkyGeneratedFiles/planner.dat"];
