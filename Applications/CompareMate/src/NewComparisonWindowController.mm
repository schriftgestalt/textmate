#import "NewComparisonWindowController.h"

@interface FileDropView : NSView
@property (nonatomic, copy) void (^fileDropHandler)(NSString* path);
@property (nonatomic) NSImageView* imageView;
@property (nonatomic) BOOL receivingDrag;
- (void)setFilePath:(NSString*)path;
@end

@implementation FileDropView
- (instancetype)init
{
	if(self = [super initWithFrame:NSZeroRect])
	{
		self.wantsLayer = YES;
		self.layer.cornerRadius = 7;
		self.layer.borderWidth = 1;
		[self registerForDraggedTypes:@[ NSPasteboardTypeFileURL ]];

		_imageView = [[NSImageView alloc] initWithFrame:NSZeroRect];
		_imageView.imageScaling = NSImageScaleProportionallyUpOrDown;
		_imageView.translatesAutoresizingMaskIntoConstraints = NO;
		_imageView.image = [NSImage imageWithSystemSymbolName:@"doc" accessibilityDescription:@"File"];
		[self addSubview:_imageView];

		[NSLayoutConstraint activateConstraints:@[
			[_imageView.leadingAnchor constraintEqualToAnchor:self.leadingAnchor constant:10],
			[_imageView.trailingAnchor constraintEqualToAnchor:self.trailingAnchor constant:-10],
			[_imageView.topAnchor constraintEqualToAnchor:self.topAnchor constant:10],
			[_imageView.bottomAnchor constraintEqualToAnchor:self.bottomAnchor constant:-10],
		]];

		[self updateAppearance];
	}
	return self;
}

- (NSView*)hitTest:(NSPoint)point
{
	return NSPointInRect(point, self.bounds) ? self : nil;
}

- (void)viewDidChangeEffectiveAppearance
{
	[super viewDidChangeEffectiveAppearance];
	[self updateAppearance];
}

- (void)updateAppearance
{
	self.layer.backgroundColor = (self.receivingDrag ? NSColor.selectedContentBackgroundColor : NSColor.controlBackgroundColor).CGColor;
	self.layer.borderColor = (self.receivingDrag ? NSColor.keyboardFocusIndicatorColor : NSColor.separatorColor).CGColor;
}

- (NSString*)filePathFromDraggingInfo:(id<NSDraggingInfo>)sender
{
	NSArray<NSURL*>* URLs = [sender.draggingPasteboard readObjectsForClasses:@[ NSURL.class ] options:@{ NSPasteboardURLReadingFileURLsOnlyKey: @YES }];
	NSURL* URL = URLs.firstObject;
	NSNumber* isRegularFile = nil;
	[URL getResourceValue:&isRegularFile forKey:NSURLIsRegularFileKey error:nil];
	return isRegularFile.boolValue ? URL.path : nil;
}

- (NSDragOperation)draggingEntered:(id<NSDraggingInfo>)sender
{
	self.receivingDrag = [self filePathFromDraggingInfo:sender] != nil;
	[self updateAppearance];
	return self.receivingDrag ? NSDragOperationCopy : NSDragOperationNone;
}

- (void)draggingExited:(id<NSDraggingInfo>)sender
{
	self.receivingDrag = NO;
	[self updateAppearance];
}

- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender
{
	NSString* path = [self filePathFromDraggingInfo:sender];
	self.receivingDrag = NO;
	[self updateAppearance];
	if(path && self.fileDropHandler)
		self.fileDropHandler(path);
	return path != nil;
}

- (void)setFilePath:(NSString*)path
{
	BOOL isDirectory = NO;
	if(path.length && [NSFileManager.defaultManager fileExistsAtPath:path isDirectory:&isDirectory] && !isDirectory)
	{
		self.imageView.image = [NSWorkspace.sharedWorkspace iconForFile:path];
		self.toolTip = path;
		self.accessibilityLabel = [NSString stringWithFormat:@"Selected file: %@", path.lastPathComponent];
	}
	else
	{
		self.imageView.image = [NSImage imageWithSystemSymbolName:@"doc" accessibilityDescription:@"File"];
		self.toolTip = @"Drop a file here";
		self.accessibilityLabel = @"File drop target";
	}
}
@end

@interface NewComparisonWindowController () <NSComboBoxDelegate>
@property (nonatomic, copy) NewComparisonHandler completionHandler;
@property (nonatomic) NSComboBox* leftPathField;
@property (nonatomic) NSComboBox* rightPathField;
@property (nonatomic) FileDropView* leftDropView;
@property (nonatomic) FileDropView* rightDropView;
@property (nonatomic) NSButton* compareButton;
@end

@implementation NewComparisonWindowController
static NSString* const LeftFileHistoryKey = @"CompareMateLeftFileHistory";
static NSString* const RightFileHistoryKey = @"CompareMateRightFileHistory";

- (instancetype)initWithCompletionHandler:(NewComparisonHandler)completionHandler
{
	NSRect const contentRect = NSMakeRect(0, 0, 920, 218);
	NSWindowStyleMask const styleMask = NSWindowStyleMaskTitled|NSWindowStyleMaskClosable;
	NSWindow* window = [[NSWindow alloc] initWithContentRect:contentRect styleMask:styleMask backing:NSBackingStoreBuffered defer:NO];
	if(self = [super initWithWindow:window])
	{
		_completionHandler = [completionHandler copy];
		window.title = @"New Comparison";
		window.releasedWhenClosed = NO;
		window.restorable = NO;

		NSView* contentView = [[NSView alloc] initWithFrame:contentRect];
		window.contentView = contentView;

		NSButton* leftButton = [NSButton buttonWithTitle:@"Left…" target:self action:@selector(chooseLeftFile:)];
		NSButton* rightButton = [NSButton buttonWithTitle:@"Right…" target:self action:@selector(chooseRightFile:)];
		leftButton.translatesAutoresizingMaskIntoConstraints = NO;
		rightButton.translatesAutoresizingMaskIntoConstraints = NO;

		_leftPathField = [[NSComboBox alloc] initWithFrame:NSZeroRect];
		_leftPathField.placeholderString = @"Choose the left file";
		_leftPathField.delegate = self;
		_leftPathField.lineBreakMode = NSLineBreakByTruncatingMiddle;
		_leftPathField.numberOfVisibleItems = 10;
		_leftPathField.completes = YES;
		[_leftPathField addItemsWithObjectValues:[NSUserDefaults.standardUserDefaults stringArrayForKey:LeftFileHistoryKey] ?: @[ ]];
		_leftPathField.translatesAutoresizingMaskIntoConstraints = NO;

		_rightPathField = [[NSComboBox alloc] initWithFrame:NSZeroRect];
		_rightPathField.placeholderString = @"Choose the right file";
		_rightPathField.delegate = self;
		_rightPathField.lineBreakMode = NSLineBreakByTruncatingMiddle;
		_rightPathField.numberOfVisibleItems = 10;
		_rightPathField.completes = YES;
		[_rightPathField addItemsWithObjectValues:[NSUserDefaults.standardUserDefaults stringArrayForKey:RightFileHistoryKey] ?: @[ ]];
		_rightPathField.translatesAutoresizingMaskIntoConstraints = NO;

		NSTextField* leftLabel = [NSTextField labelWithString:@"Left"];
		NSTextField* rightLabel = [NSTextField labelWithString:@"Right"];
		leftLabel.alignment = NSTextAlignmentCenter;
		rightLabel.alignment = NSTextAlignmentCenter;
		leftLabel.translatesAutoresizingMaskIntoConstraints = NO;
		rightLabel.translatesAutoresizingMaskIntoConstraints = NO;

		_leftDropView = [[FileDropView alloc] init];
		_rightDropView = [[FileDropView alloc] init];
		_leftDropView.translatesAutoresizingMaskIntoConstraints = NO;
		_rightDropView.translatesAutoresizingMaskIntoConstraints = NO;

		__weak NewComparisonWindowController* weakSelf = self;
		_leftDropView.fileDropHandler = ^(NSString* path) {
			[weakSelf setPath:path forLeftSide:YES];
		};
		_rightDropView.fileDropHandler = ^(NSString* path) {
			[weakSelf setPath:path forLeftSide:NO];
		};

		NSButton* cancelButton = [NSButton buttonWithTitle:@"Cancel" target:self action:@selector(cancel:)];
		_compareButton = [NSButton buttonWithTitle:@"Compare" target:self action:@selector(compare:)];
		cancelButton.translatesAutoresizingMaskIntoConstraints = NO;
		_compareButton.translatesAutoresizingMaskIntoConstraints = NO;
		_compareButton.keyEquivalent = @"\r";

		for(NSView* view in @[ leftButton, rightButton, _leftPathField, _rightPathField, leftLabel, rightLabel, _leftDropView, _rightDropView, cancelButton, _compareButton ])
			[contentView addSubview:view];

		[NSLayoutConstraint activateConstraints:@[
			[leftButton.leadingAnchor constraintEqualToAnchor:contentView.leadingAnchor constant:24],
			[leftButton.topAnchor constraintEqualToAnchor:contentView.topAnchor constant:55],
			[leftButton.widthAnchor constraintEqualToConstant:100],
			[rightButton.leadingAnchor constraintEqualToAnchor:leftButton.leadingAnchor],
			[rightButton.topAnchor constraintEqualToAnchor:leftButton.bottomAnchor constant:12],
			[rightButton.widthAnchor constraintEqualToAnchor:leftButton.widthAnchor],

			[_leftPathField.leadingAnchor constraintEqualToAnchor:leftButton.trailingAnchor constant:12],
			[_leftPathField.centerYAnchor constraintEqualToAnchor:leftButton.centerYAnchor],
			[_leftPathField.trailingAnchor constraintEqualToAnchor:_leftDropView.leadingAnchor constant:-20],
			[_rightPathField.leadingAnchor constraintEqualToAnchor:rightButton.trailingAnchor constant:12],
			[_rightPathField.centerYAnchor constraintEqualToAnchor:rightButton.centerYAnchor],
			[_rightPathField.trailingAnchor constraintEqualToAnchor:_leftDropView.leadingAnchor constant:-20],

			[leftLabel.centerXAnchor constraintEqualToAnchor:_leftDropView.centerXAnchor],
			[leftLabel.topAnchor constraintEqualToAnchor:contentView.topAnchor constant:18],
			[leftLabel.widthAnchor constraintEqualToAnchor:_leftDropView.widthAnchor],
			[_leftDropView.topAnchor constraintEqualToAnchor:leftLabel.bottomAnchor constant:7],
			[_leftDropView.widthAnchor constraintEqualToConstant:76],
			[_leftDropView.heightAnchor constraintEqualToConstant:76],

			[rightLabel.centerXAnchor constraintEqualToAnchor:_rightDropView.centerXAnchor],
			[rightLabel.topAnchor constraintEqualToAnchor:leftLabel.topAnchor],
			[rightLabel.widthAnchor constraintEqualToAnchor:_rightDropView.widthAnchor],
			[_rightDropView.leadingAnchor constraintEqualToAnchor:_leftDropView.trailingAnchor constant:16],
			[_rightDropView.topAnchor constraintEqualToAnchor:_leftDropView.topAnchor],
			[_rightDropView.widthAnchor constraintEqualToAnchor:_leftDropView.widthAnchor],
			[_rightDropView.heightAnchor constraintEqualToAnchor:_leftDropView.heightAnchor],
			[_rightDropView.trailingAnchor constraintEqualToAnchor:contentView.trailingAnchor constant:-24],

			[_compareButton.trailingAnchor constraintEqualToAnchor:contentView.trailingAnchor constant:-24],
			[_compareButton.bottomAnchor constraintEqualToAnchor:contentView.bottomAnchor constant:-20],
			[_compareButton.widthAnchor constraintEqualToConstant:104],
			[cancelButton.trailingAnchor constraintEqualToAnchor:_compareButton.leadingAnchor constant:-12],
			[cancelButton.centerYAnchor constraintEqualToAnchor:_compareButton.centerYAnchor],
			[cancelButton.widthAnchor constraintEqualToConstant:104],
		]];

		[self updateState];
		[window center];
		window.initialFirstResponder = _leftPathField;
	}
	return self;
}

- (NSString*)normalizedPathFromField:(NSComboBox*)field
{
	return field.stringValue.stringByExpandingTildeInPath.stringByStandardizingPath;
}

- (BOOL)isRegularFileAtPath:(NSString*)path
{
	BOOL isDirectory = NO;
	return path.length && [NSFileManager.defaultManager fileExistsAtPath:path isDirectory:&isDirectory] && !isDirectory;
}

- (void)updateState
{
	NSString* leftPath = [self normalizedPathFromField:self.leftPathField];
	NSString* rightPath = [self normalizedPathFromField:self.rightPathField];
	[self.leftDropView setFilePath:leftPath];
	[self.rightDropView setFilePath:rightPath];
	self.compareButton.enabled = [self isRegularFileAtPath:leftPath] && [self isRegularFileAtPath:rightPath];
}

- (void)setPath:(NSString*)path forLeftSide:(BOOL)leftSide
{
	NSComboBox* field = leftSide ? self.leftPathField : self.rightPathField;
	field.stringValue = path.stringByStandardizingPath;
	[self rememberPath:field.stringValue forLeftSide:leftSide];
	[self updateState];
	[self.window makeFirstResponder:(leftSide ? self.rightPathField : self.leftPathField)];
}

- (void)rememberPath:(NSString*)path forLeftSide:(BOOL)leftSide
{
	NSString* normalizedPath = path.stringByExpandingTildeInPath.stringByStandardizingPath;
	if(![self isRegularFileAtPath:normalizedPath])
		return;

	NSString* historyKey = leftSide ? LeftFileHistoryKey : RightFileHistoryKey;
	NSMutableArray<NSString*>* history = [[NSUserDefaults.standardUserDefaults stringArrayForKey:historyKey] mutableCopy] ?: [NSMutableArray array];
	[history removeObject:normalizedPath];
	[history insertObject:normalizedPath atIndex:0];
	if(history.count > 10)
		[history removeObjectsInRange:NSMakeRange(10, history.count - 10)];
	[NSUserDefaults.standardUserDefaults setObject:history forKey:historyKey];

	NSComboBox* comboBox = leftSide ? self.leftPathField : self.rightPathField;
	[comboBox removeAllItems];
	[comboBox addItemsWithObjectValues:history];
}

- (void)chooseFileForLeftSide:(BOOL)leftSide
{
	NSOpenPanel* panel = [NSOpenPanel openPanel];
	panel.canChooseFiles = YES;
	panel.canChooseDirectories = NO;
	panel.allowsMultipleSelection = NO;
	panel.prompt = leftSide ? @"Choose Left" : @"Choose Right";

	NSString* currentPath = [self normalizedPathFromField:(leftSide ? self.leftPathField : self.rightPathField)];
	if(currentPath.length)
		panel.directoryURL = [NSURL fileURLWithPath:currentPath.stringByDeletingLastPathComponent isDirectory:YES];

	[panel beginSheetModalForWindow:self.window completionHandler:^(NSModalResponse response) {
		if(response == NSModalResponseOK)
			[self setPath:panel.URL.path forLeftSide:leftSide];
	}];
}

- (IBAction)chooseLeftFile:(id)sender
{
	[self chooseFileForLeftSide:YES];
}

- (IBAction)chooseRightFile:(id)sender
{
	[self chooseFileForLeftSide:NO];
}

- (IBAction)cancel:(id)sender
{
	[self close];
}

- (IBAction)compare:(id)sender
{
	[self updateState];
	if(!self.compareButton.enabled)
	{
		NSBeep();
		return;
	}

	if(self.completionHandler)
	{
		NSString* leftPath = [self normalizedPathFromField:self.leftPathField];
		NSString* rightPath = [self normalizedPathFromField:self.rightPathField];
		[self rememberPath:leftPath forLeftSide:YES];
		[self rememberPath:rightPath forLeftSide:NO];
		self.completionHandler(leftPath, rightPath);
	}
	[self close];
}

- (void)controlTextDidChange:(NSNotification*)notification
{
	[self updateState];
}

- (void)comboBoxSelectionDidChange:(NSNotification*)notification
{
	NSComboBox* comboBox = notification.object;
	if(NSString* selectedPath = comboBox.objectValueOfSelectedItem)
		comboBox.stringValue = selectedPath;
	[self updateState];
}
@end
