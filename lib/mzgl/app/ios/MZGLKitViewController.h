//
//  MZGLES3Renderer.h
//  MZGLiOS
//
//  Created by Marek Bereza on 17/01/2018.
//  Copyright © 2018 Marek Bereza. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <GLKit/GLKit.h>
#ifdef MZGL_AU
#import <CoreAudioKit/CoreAudioKit.h>
#endif
#import "MZGLKitView.h"
class App;

class EventDispatcher;
@interface MZGLKitViewController : GLKViewController
#ifdef MZGL_AU
// NOT SURE IF WE NEED THESE, AS THERES A DIFFERENT CLASS DOING IT I THINK
<AUAudioUnitFactory,NSExtensionRequestHandling>
#endif
- (id) initWithApp: (App*) app;
- (EventDispatcher*) getEventDispatcher;
- (void) openURLWhenLoadedAndDeleteFile: (NSString*) urlToOpen;
- (MZGLKitView*) getView;
@end



