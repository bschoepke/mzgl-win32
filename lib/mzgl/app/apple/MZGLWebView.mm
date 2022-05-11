//
//  EventsView.m
//  mzgl macOS
//
//  Created by Marek Bereza on 03/02/2021.
//  Copyright © 2021 Marek Bereza. All rights reserved.
//

#import "MZGLWebView.h"

#include <string>
#include "EventDispatcher.h"
#include "log.h"
#include "filesystem.h"
@implementation MZGLWebView {
    EventDispatcher *eventDispatcher;
}
- (id) initWithFrame: (NSRect) frame eventDispatcher:(void*)evtDispatcherPtr andUrl: (NSString*) url {
    eventDispatcher = (EventDispatcher*)evtDispatcherPtr;
    
    ((WebViewApp*)eventDispatcher->app)->callJS = [self](const std::string &s) {
        [self evaluateJavaScript:[NSString stringWithUTF8String:s.c_str()] completionHandler:nil];
    };

    WKWebViewConfiguration *config = [[WKWebViewConfiguration alloc] init];
    [config.userContentController addScriptMessageHandler:self name:@"updateHandler"];

	self = [super initWithFrame:frame configuration:config];
	std::string customUrl = [url UTF8String];//"http://localhost:8080/";
	if(self!=nil) {
		
		[self registerForDraggedTypes:[NSArray arrayWithObject:NSFilenamesPboardType]];
		self.UIDelegate = self;
		
		if(customUrl!="" && customUrl.find("http")!=-1) {
			NSURL *url = [NSURL URLWithString:[NSString stringWithFormat:@"%s", customUrl.c_str()]];
			NSURLRequest *request = [NSURLRequest requestWithURL:url];
			[self loadRequest:request];
		} else {
			
		
			NSURL *baseURL = [[NSBundle mainBundle] resourceURL];
			baseURL = [baseURL URLByAppendingPathComponent:@"www"];
			NSURL *url = [baseURL URLByAppendingPathComponent:@"index.html"];
			
			
			if(customUrl!="") {
				NSLog(@"Got custom URL!!");
				auto p = fs::path(customUrl);
				baseURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String: p.parent_path().string().c_str()]];
				url = [baseURL URLByAppendingPathComponent:[NSString stringWithUTF8String:p.filename().string().c_str()]];
			}
			NSLog(@"%@", url);
			
			[self loadFileURL:url allowingReadAccessToURL:baseURL];
//		} else {
//
//			auto p = fs::path(customUrl);
//			NSURL *baseURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String: p.parent_path().string().c_str()]];
//			NSURL *url = [baseURL URLByAppendingPathComponent:[NSString stringWithUTF8String:p.filename().string().c_str()]];
//			[self loadFileURL:url allowingReadAccessToURL:baseURL];
//		}
		}
	}
	return self;
}
- (void)webView:(WKWebView *)webView runJavaScriptAlertPanelWithMessage:(NSString *)message initiatedByFrame:(WKFrameInfo *)frame completionHandler:(void (^)(void))completionHandler
{
 
    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:message];
    [alert setAlertStyle:NSAlertStyleWarning];

    [alert beginSheetModalForWindow:[NSApp mainWindow] completionHandler:^(NSInteger result) {
        completionHandler();
    }];
}

- (void)windowDidResize:(NSNotification *)notification {
	Log::d() << "windowDidResize";
	NSWindow *window = notification.object;
	//WIDTH = window.frame.size.width;
	//HEIGHT = window.frame.size.height;
	
	auto w = window.contentLayoutRect.size.width;
	auto h = window.contentLayoutRect.size.height;
//	auto pixelScale = [window backingScaleFactor];
	Log::d() << w << " " << h;
	
	NSRect f = self.frame;
	f.size = CGSizeMake(w, h);;
	self.frame = f;

	
	
}

- (BOOL) acceptsFirstResponder {
	return YES;
}

- (BOOL) becomeFirstResponder {
	return YES;
}

- (BOOL) isOpaque {
	return YES;
}


- (void) shutdown {
//	[super shutdown];
//	eventDispatcher->exit();
}
//
//- (void*) getApp {
//	return eventDispatcher->app.get();
//}

//
//- (BOOL)windowShouldClose:(NSWindow *)sender {
//	return YES;
//}
//
//- (void)windowDidBecomeKey:(NSNotification *)notification {
////	NSWindow *window = notification.object;
////	Graphics &g = eventDispatcher->app->g;
////	g.pixelScale = [window backingScaleFactor];
////	Log::e() << "Pixel scale being set for first time: " << g.pixelScale;
//}
//
//- (void)windowWillClose:(NSNotification *)notification {
//	[[NSApplication sharedApplication] terminate:nil];
//}
//
//- (void)windowDidResize:(NSNotification *)notification {
//	[super windowResized: notification];
//}



- (void)userContentController:(nonnull WKUserContentController *)userContentController didReceiveScriptMessage:(nonnull WKScriptMessage *)message {
    NSDictionary *dict = message.body;
    std::string key = [dict[@"key"] UTF8String];
    std::string value = [dict[@"value"] UTF8String];
    eventDispatcher->receivedJSMessage(key, value);
}


@end
