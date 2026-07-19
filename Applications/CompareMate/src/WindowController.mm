#import "WindowController.h"
#import <OakTextView/src/OakDocumentView.h>
#import <document/src/OakDocument.h>

@interface WindowController () <NSWindowDelegate>
@property (nonatomic) NSWindowController* retainedSelf;
@property (nonatomic) OakDocumentView* leftDocumentView;
@property (nonatomic) OakDocumentView* rightDocumentView;
@property (nonatomic) NSSplitViewController* splitViewController;
@end

@implementation WindowController
+ (void)initialize
{
	NSWindow.allowsAutomaticWindowTabbing = NO;
}

- (instancetype)init
{
	NSRect const contentRect = NSMakeRect(0, 0, 1200, 760);
	NSWindowStyleMask const styleMask = NSWindowStyleMaskTitled|NSWindowStyleMaskResizable|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable;
	if(self = [self initWithWindow:[[NSWindow alloc] initWithContentRect:contentRect styleMask:styleMask backing:NSBackingStoreBuffered defer:NO]])
	{
		_retainedSelf = self;

		self.leftDocumentView = [[OakDocumentView alloc] initWithFrame:NSZeroRect];
		self.leftDocumentView.document = [OakDocument documentWithString:@"" fileType:@"text.plain" customName:@"Left"];
		self.leftDocumentView.textView.softWrap = NO;

		self.rightDocumentView = [[OakDocumentView alloc] initWithFrame:NSZeroRect];
		self.rightDocumentView.document = [OakDocument documentWithString:@"" fileType:@"text.plain" customName:@"Right"];
		self.rightDocumentView.textView.softWrap = NO;

		NSViewController* leftViewController = [[NSViewController alloc] init];
		leftViewController.view = self.leftDocumentView;

		NSViewController* rightViewController = [[NSViewController alloc] init];
		rightViewController.view = self.rightDocumentView;

		NSSplitViewItem* leftItem = [NSSplitViewItem splitViewItemWithViewController:leftViewController];
		leftItem.minimumThickness = 200;
		leftItem.canCollapse = NO;

		NSSplitViewItem* rightItem = [NSSplitViewItem splitViewItemWithViewController:rightViewController];
		rightItem.minimumThickness = 200;
		rightItem.canCollapse = NO;

		self.splitViewController = [[NSSplitViewController alloc] init];
		self.splitViewController.splitView.vertical = YES;
		self.splitViewController.splitView.dividerStyle = NSSplitViewDividerStyleThin;
		[self.splitViewController addSplitViewItem:leftItem];
		[self.splitViewController addSplitViewItem:rightItem];

		NSWindow* window = self.window;
		window.title = @"CompareMate";
		window.delegate = self;
		window.minSize = NSMakeSize(720, 400);
		window.contentViewController = self.splitViewController;
		window.initialFirstResponder = self.leftDocumentView.textView;

		[window layoutIfNeeded];
		[self.splitViewController.splitView setPosition:NSWidth(self.splitViewController.splitView.bounds) / 2 ofDividerAtIndex:0];
		[window center];

		window.frameAutosaveName = @"CompareMate Main";
	}
	return self;
}

- (void)windowWillClose:(NSNotification*)aNotification
{
	_retainedSelf = nil;
	[NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0];
}
@end
