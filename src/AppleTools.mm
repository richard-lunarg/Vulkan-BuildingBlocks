/*
 * Many system level features have to use Objective C Apple speciic functions. This
 * file contains a set of C callable functions that are implemented in Objective C.
 *
 *
 *  Richard S. Wright Jr.
 *  richard@lunarg.com
 *
 */

#include "AppleTools.h"



#import <Foundation/Foundation.h>
#import <QuartzCore/CAMetalLayer.h>
#ifndef APPLE_MOBILE
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
    NSView* view = (NSView*)handle;
    assert([view isKindOfClass:[NSView class]]);

    void *pLayer = view.layer;
    return pLayer;

//    UIView* view = (__bridge UIView*)handle;
//    assert([view isKindOfClass:[UIView class]]);

//    if (![view.layer isKindOfClass:[CAMetalLayer class]])
//        {
//        [view setLayer:[CAMetalLayer layer]];
//        [view setWantsLayer:YES];
//        }


//    void *pLayer = view.layer;
//    return pLayer;

}



// NSArray *docPath = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
// NSString *myPath = [docPath objectAtIndex:0];
// NSString *pPlannerLoc = [myPath stringByAppendingPathComponent:@"/SkyGeneratedFiles/planner.dat"];
