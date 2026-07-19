#import "WindowController.h"
#import <OakTextView/src/OakDocumentView.h>
#import <document/src/OakDocument.h>

static NSString* const LeftPathRestorationKey = @"CompareMate.leftPath";
static NSString* const RightPathRestorationKey = @"CompareMate.rightPath";
static NSString* const DividerPositionRestorationKey = @"CompareMate.dividerPosition";

@interface WindowController () <NSWindowDelegate>
@property (nonatomic) NSWindowController* retainedSelf;
@property (nonatomic) OakDocumentView* leftDocumentView;
@property (nonatomic) OakDocumentView* rightDocumentView;
@property (nonatomic) NSSplitViewController* splitViewController;
@property (nonatomic, copy) NSString* leftPath;
@property (nonatomic, copy) NSString* rightPath;
@end

@implementation WindowController
+ (void)initialize
{
	NSWindow.allowsAutomaticWindowTabbing = NO;
}

- (instancetype)init
{
	return [self initWithLeftPath:nil rightPath:nil];
}

- (instancetype)initWithLeftPath:(NSString*)leftPath rightPath:(NSString*)rightPath
{
	NSRect const contentRect = NSMakeRect(0, 0, 1200, 760);
	NSWindowStyleMask const styleMask = NSWindowStyleMaskTitled|NSWindowStyleMaskResizable|NSWindowStyleMaskClosable|NSWindowStyleMaskMiniaturizable;
	if(self = [self initWithWindow:[[NSWindow alloc] initWithContentRect:contentRect styleMask:styleMask backing:NSBackingStoreBuffered defer:NO]])
	{
		_retainedSelf = self;
		_leftPath = [leftPath copy];
		_rightPath = [rightPath copy];

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
		window.title = leftPath && rightPath ? [NSString stringWithFormat:@"%@ ↔ %@", leftPath.lastPathComponent, rightPath.lastPathComponent] : @"CompareMate";
		window.delegate = self;
		window.minSize = NSMakeSize(720, 400);
		window.contentViewController = self.splitViewController;
		window.initialFirstResponder = self.leftDocumentView.textView;
		window.identifier = [NSString stringWithFormat:@"CompareMate.Comparison.%@", NSUUID.UUID.UUIDString];
		[window setFrameAutosaveName:window.identifier];
		window.restorationClass = WindowController.class;
		window.restorable = YES;

		[window layoutIfNeeded];
		[self.splitViewController.splitView setPosition:NSWidth(self.splitViewController.splitView.bounds) / 2 ofDividerAtIndex:0];
		[window center];
		[NSNotificationCenter.defaultCenter addObserver:self selector:@selector(splitViewDidResizeSubviews:) name:NSSplitViewDidResizeSubviewsNotification object:self.splitViewController.splitView];

		if(leftPath)
			[self loadDocumentAtPath:leftPath intoDocumentView:self.leftDocumentView sideName:@"Left"];
		if(rightPath)
			[self loadDocumentAtPath:rightPath intoDocumentView:self.rightDocumentView sideName:@"Right"];

		[self invalidateRestorableState];
	}
	return self;
}

+ (void)restoreWindowWithIdentifier:(NSUserInterfaceItemIdentifier)identifier state:(NSCoder*)state completionHandler:(void (^)(NSWindow*, NSError*))completionHandler
{
	NSString* leftPath = [state decodeObjectOfClass:NSString.class forKey:LeftPathRestorationKey];
	NSString* rightPath = [state decodeObjectOfClass:NSString.class forKey:RightPathRestorationKey];
	BOOL leftExists = leftPath.length && [NSFileManager.defaultManager fileExistsAtPath:leftPath];
	BOOL rightExists = rightPath.length && [NSFileManager.defaultManager fileExistsAtPath:rightPath];
	if(!leftExists || !rightExists)
	{
		completionHandler(nil, nil);
		return;
	}

	WindowController* windowController = [[WindowController alloc] initWithLeftPath:leftPath rightPath:rightPath];
	windowController.window.identifier = identifier;
	[windowController.window setFrameAutosaveName:identifier];

	completionHandler(windowController.window, nil);
}

- (void)encodeRestorableStateWithCoder:(NSCoder*)coder
{
	[super encodeRestorableStateWithCoder:coder];
	[coder encodeObject:self.leftPath forKey:LeftPathRestorationKey];
	[coder encodeObject:self.rightPath forKey:RightPathRestorationKey];
	[coder encodeDouble:NSMaxX(self.leftDocumentView.frame) forKey:DividerPositionRestorationKey];
}

- (void)restoreStateWithCoder:(NSCoder*)coder
{
	[super restoreStateWithCoder:coder];
	if([coder containsValueForKey:DividerPositionRestorationKey])
	{
		CGFloat dividerPosition = [coder decodeDoubleForKey:DividerPositionRestorationKey];
		[self.window layoutIfNeeded];
		[self.splitViewController.splitView setPosition:dividerPosition ofDividerAtIndex:0];
	}
}

- (void)splitViewDidResizeSubviews:(NSNotification*)notification
{
	[self invalidateRestorableState];
}

- (void)loadDocumentAtPath:(NSString*)path intoDocumentView:(OakDocumentView*)documentView sideName:(NSString*)sideName
{
	OakDocument* document = [OakDocument documentWithPath:path];
	[document loadModalForWindow:self.window completionHandler:^(OakDocumentIOResult result, NSString* errorMessage, oak::uuid_t const& filterUUID) {
		if(result == OakDocumentIOResultSuccess)
		{
			documentView.document = document;
			[document close];
		}
		else
		{
			NSAlert* alert = [[NSAlert alloc] init];
			alert.alertStyle = NSAlertStyleCritical;
			alert.messageText = [NSString stringWithFormat:@"Couldn’t open the %@ file", sideName];
			alert.informativeText = errorMessage.length ? errorMessage : path;
			[alert beginSheetModalForWindow:self.window completionHandler:nil];
		}
	}];
}

- (void)windowWillClose:(NSNotification*)aNotification
{
	_retainedSelf = nil;
}
@end
