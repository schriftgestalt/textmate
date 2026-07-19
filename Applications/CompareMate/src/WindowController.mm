#import "WindowController.h"
#import <OakTextView/src/OakDocumentView.h>
#import <OakTextView/src/GutterView.h>
#import <document/src/OakDocument.h>

static NSString* const LeftPathRestorationKey = @"CompareMate.leftPath";
static NSString* const RightPathRestorationKey = @"CompareMate.rightPath";
static NSString* const DividerPositionRestorationKey = @"CompareMate.dividerPosition";

@interface DiffCharacterRange : NSObject
@property (nonatomic) NSUInteger line;
@property (nonatomic) NSRange byteColumns;
@end

@implementation DiffCharacterRange
@end

@interface DiffHighlightView : NSObject
@property (nonatomic, weak) OakTextView* textView;
@property (nonatomic, weak) GutterView* gutterView;
@property (nonatomic, copy) NSIndexSet* highlightedLines;
@property (nonatomic, copy) NSIndexSet* markerPositions;
@property (nonatomic, copy) NSIndexSet* activeHighlightedLines;
@property (nonatomic, copy) NSIndexSet* activeMarkerPositions;
@property (nonatomic, copy) NSArray<DiffCharacterRange*>* characterRanges;
@property (nonatomic, copy) NSArray<DiffCharacterRange*>* characterMarkers;
@property (nonatomic) NSUInteger lineCount;
@property (nonatomic) NSColor* highlightColor;
@property (nonatomic) NSColor* markerColor;
@property (nonatomic) NSColor* activeColor;
@property (nonatomic) NSColor* characterColor;
- (instancetype)initWithTextView:(OakTextView*)textView color:(NSColor*)color;
- (void)drawBackgroundInRect:(NSRect)dirtyRect;
- (void)drawForegroundInRect:(NSRect)dirtyRect;
- (void)drawGutterBackgroundInRect:(NSRect)dirtyRect;
@end

@implementation DiffHighlightView
- (instancetype)initWithTextView:(OakTextView*)textView color:(NSColor*)color
{
	if(self = [super init])
	{
		_textView = textView;
		_highlightedLines = NSIndexSet.indexSet;
		_markerPositions = NSIndexSet.indexSet;
		_activeHighlightedLines = NSIndexSet.indexSet;
		_activeMarkerPositions = NSIndexSet.indexSet;
		_characterRanges = @[];
		_characterMarkers = @[];
		_highlightColor = color;
		_markerColor = [color colorWithAlphaComponent:0.85];
		_activeColor = [NSColor.controlAccentColor colorWithAlphaComponent:0.38];
		_characterColor = [color colorWithAlphaComponent:0.52];
	}
	return self;
}

- (void)setHighlightedLines:(NSIndexSet*)highlightedLines
{
	if([_highlightedLines isEqualToIndexSet:highlightedLines])
		return;
	_highlightedLines = [highlightedLines copy];
	self.textView.needsDisplay = YES;
	self.gutterView.needsDisplay = YES;
}

- (void)setMarkerPositions:(NSIndexSet*)markerPositions
{
	if([_markerPositions isEqualToIndexSet:markerPositions])
		return;
	_markerPositions = [markerPositions copy];
	self.textView.needsDisplay = YES;
	self.gutterView.needsDisplay = YES;
}

- (void)setActiveHighlightedLines:(NSIndexSet*)activeHighlightedLines
{
	if([_activeHighlightedLines isEqualToIndexSet:activeHighlightedLines])
		return;
	_activeHighlightedLines = [activeHighlightedLines copy];
	self.textView.needsDisplay = YES;
	self.gutterView.needsDisplay = YES;
}

- (void)setActiveMarkerPositions:(NSIndexSet*)activeMarkerPositions
{
	if([_activeMarkerPositions isEqualToIndexSet:activeMarkerPositions])
		return;
	_activeMarkerPositions = [activeMarkerPositions copy];
	self.textView.needsDisplay = YES;
	self.gutterView.needsDisplay = YES;
}

- (void)setCharacterRanges:(NSArray<DiffCharacterRange*>*)characterRanges
{
	_characterRanges = [characterRanges copy];
	self.textView.needsDisplay = YES;
}

- (void)setCharacterMarkers:(NSArray<DiffCharacterRange*>*)characterMarkers
{
	_characterMarkers = [characterMarkers copy];
	self.textView.needsDisplay = YES;
}

- (void)setLineCount:(NSUInteger)lineCount
{
	if(_lineCount == lineCount)
		return;
	_lineCount = lineCount;
	self.textView.needsDisplay = YES;
	self.gutterView.needsDisplay = YES;
}

- (void)drawBackgroundInRect:(NSRect)dirtyRect
{
	if(!self.textView)
		return;

	[self.highlightColor setFill];
	NSRect const bounds = self.textView.bounds;
	[self.highlightedLines enumerateIndexesUsingBlock:^(NSUInteger line, BOOL* stop) {
		GVLineRecord const firstFragment = [self.textView lineFragmentForLine:line column:0];
		if(firstFragment.lineNumber != line)
			return;

		CGFloat bottom = 0;
		GVLineRecord const nextLine = [self.textView lineFragmentForLine:line + 1 column:0];
		if(nextLine.lineNumber == line + 1)
			bottom = nextLine.firstY;
		else
		{
			GVLineRecord const lastFragment = [self.textView lineFragmentForLine:line column:NSUIntegerMax];
			bottom = lastFragment.lastY;
		}

		NSRect lineRect = NSMakeRect(NSMinX(bounds), firstFragment.firstY, NSWidth(bounds), MAX(1, bottom - firstFragment.firstY));
		lineRect = NSIntersectionRect(lineRect, dirtyRect);
		if(!NSIsEmptyRect(lineRect))
			NSRectFillUsingOperation(lineRect, NSCompositingOperationSourceOver);
	}];

	[self.activeColor setFill];
	[self.activeHighlightedLines enumerateIndexesUsingBlock:^(NSUInteger line, BOOL* stop) {
		GVLineRecord const firstFragment = [self.textView lineFragmentForLine:line column:0];
		if(firstFragment.lineNumber != line)
			return;

		CGFloat bottom = 0;
		GVLineRecord const nextLine = [self.textView lineFragmentForLine:line + 1 column:0];
		if(nextLine.lineNumber == line + 1)
			bottom = nextLine.firstY;
		else
			bottom = [self.textView lineFragmentForLine:line column:NSUIntegerMax].lastY;

		NSRect lineRect = NSIntersectionRect(NSMakeRect(NSMinX(bounds), firstFragment.firstY, NSWidth(bounds), MAX(1, bottom - firstFragment.firstY)), dirtyRect);
		if(!NSIsEmptyRect(lineRect))
			NSRectFillUsingOperation(lineRect, NSCompositingOperationSourceOver);
	}];

	[self.characterColor setFill];
	for(DiffCharacterRange* range in self.characterRanges)
	{
		NSRect const layoutRect = [self.textView rectForLine:range.line byteColumnRange:range.byteColumns];
		NSRect const characterRect = NSIntersectionRect(layoutRect, dirtyRect);
		if(NSIsEmptyRect(characterRect))
			continue;

		NSRectFillUsingOperation(characterRect, NSCompositingOperationSourceOver);
		NSRect const underlineRect = NSIntersectionRect(NSMakeRect(NSMinX(layoutRect), NSMaxY(layoutRect) - 2, NSWidth(layoutRect), 2), dirtyRect);
		if(!NSIsEmptyRect(underlineRect))
			NSRectFillUsingOperation(underlineRect, NSCompositingOperationSourceOver);
	}
}

- (void)drawForegroundInRect:(NSRect)dirtyRect
{
	if(!self.textView)
		return;

	[self.markerColor setFill];
	NSRect const bounds = self.textView.bounds;
	[self.markerPositions enumerateIndexesUsingBlock:^(NSUInteger position, BOOL* stop) {
		CGFloat y = 0;
		if(position < self.lineCount)
		{
			GVLineRecord const line = [self.textView lineFragmentForLine:position column:0];
			if(line.lineNumber != position)
				return;
			y = line.firstY;
		}
		else if(position == self.lineCount && self.lineCount != 0)
		{
			GVLineRecord const lastLine = [self.textView lineFragmentForLine:self.lineCount - 1 column:NSUIntegerMax];
			if(lastLine.lineNumber != self.lineCount - 1)
				return;
			y = lastLine.lastY;
		}
		else
			return;

		NSRect markerRect = NSIntersectionRect(NSMakeRect(NSMinX(bounds), y - 1, NSWidth(bounds), 2), dirtyRect);
		if(!NSIsEmptyRect(markerRect))
			NSRectFillUsingOperation(markerRect, NSCompositingOperationSourceOver);
	}];

	[self.activeColor setFill];
	[self.activeMarkerPositions enumerateIndexesUsingBlock:^(NSUInteger position, BOOL* stop) {
		CGFloat y = 0;
		if(position < self.lineCount)
			y = [self.textView lineFragmentForLine:position column:0].firstY;
		else if(position == self.lineCount && self.lineCount != 0)
			y = [self.textView lineFragmentForLine:self.lineCount - 1 column:NSUIntegerMax].lastY;
		else
			return;

		NSRect markerRect = NSIntersectionRect(NSMakeRect(NSMinX(bounds), y - 2, NSWidth(bounds), 4), dirtyRect);
		if(!NSIsEmptyRect(markerRect))
			NSRectFillUsingOperation(markerRect, NSCompositingOperationSourceOver);
	}];

	[self.markerColor setFill];
	for(DiffCharacterRange* marker in self.characterMarkers)
	{
		NSRect const caretRect = [self.textView caretRectForLine:marker.line byteColumn:marker.byteColumns.location];
		NSRect const markerRect = NSIntersectionRect(NSMakeRect(NSMinX(caretRect) - 1, NSMinY(caretRect) + 1, 2, MAX(2, NSHeight(caretRect) - 2)), dirtyRect);
		if(!NSIsEmptyRect(markerRect))
			NSRectFillUsingOperation(markerRect, NSCompositingOperationSourceOver);
	}
}

- (void)drawGutterBackgroundInRect:(NSRect)dirtyRect
{
	if(!self.textView || !self.gutterView)
		return;

	NSRect const bounds = self.gutterView.bounds;
	void (^drawLines)(NSIndexSet*, NSColor*) = ^(NSIndexSet* lines, NSColor* color) {
		[color setFill];
		[lines enumerateIndexesUsingBlock:^(NSUInteger line, BOOL* stop) {
			GVLineRecord const firstFragment = [self.textView lineFragmentForLine:line column:0];
			if(firstFragment.lineNumber != line)
				return;

			GVLineRecord const nextLine = [self.textView lineFragmentForLine:line + 1 column:0];
			CGFloat const bottom = nextLine.lineNumber == line + 1 ? nextLine.firstY : [self.textView lineFragmentForLine:line column:NSUIntegerMax].lastY;
			NSRect const lineRect = NSIntersectionRect(NSMakeRect(NSMinX(bounds), firstFragment.firstY, NSWidth(bounds), MAX(1, bottom - firstFragment.firstY)), dirtyRect);
			if(!NSIsEmptyRect(lineRect))
				NSRectFillUsingOperation(lineRect, NSCompositingOperationSourceOver);
		}];
	};
	drawLines(self.highlightedLines, self.highlightColor);
	drawLines(self.activeHighlightedLines, self.activeColor);

	void (^drawMarkers)(NSIndexSet*, CGFloat, NSColor*) = ^(NSIndexSet* positions, CGFloat thickness, NSColor* color) {
		[color setFill];
		[positions enumerateIndexesUsingBlock:^(NSUInteger position, BOOL* stop) {
			CGFloat y = 0;
			if(position < self.lineCount)
				y = [self.textView lineFragmentForLine:position column:0].firstY;
			else if(position == self.lineCount && self.lineCount != 0)
				y = [self.textView lineFragmentForLine:self.lineCount - 1 column:NSUIntegerMax].lastY;
			else
				return;

			NSRect const markerRect = NSIntersectionRect(NSMakeRect(NSMinX(bounds), y - thickness / 2, NSWidth(bounds), thickness), dirtyRect);
			if(!NSIsEmptyRect(markerRect))
				NSRectFillUsingOperation(markerRect, NSCompositingOperationSourceOver);
		}];
	};
	drawMarkers(self.markerPositions, 2, self.markerColor);
	drawMarkers(self.activeMarkerPositions, 4, self.activeColor);
}
@end

@interface DiffScrollTransition : NSObject
@property (nonatomic) NSUInteger sourceBoundary;
@property (nonatomic) NSUInteger targetStart;
@property (nonatomic) NSUInteger targetEnd;
@end

@implementation DiffScrollTransition
@end

@interface DiffHunk : NSObject
@property (nonatomic) NSRange leftLines;
@property (nonatomic) NSRange rightLines;
@end

@implementation DiffHunk
@end

@interface DiffCharacterToken : NSObject
@property (nonatomic) NSString* string;
@property (nonatomic) NSRange byteRange;
@end

@implementation DiffCharacterToken
@end

static NSArray<DiffCharacterToken*>* CharacterTokensForLine (NSString* line)
{
	NSMutableArray<DiffCharacterToken*>* tokens = [NSMutableArray array];
	__block NSUInteger byteOffset = 0;
	[line enumerateSubstringsInRange:NSMakeRange(0, line.length) options:NSStringEnumerationByComposedCharacterSequences usingBlock:^(NSString* substring, NSRange substringRange, NSRange enclosingRange, BOOL* stop) {
		NSUInteger const byteLength = [substring lengthOfBytesUsingEncoding:NSUTF8StringEncoding];
		DiffCharacterToken* token = [[DiffCharacterToken alloc] init];
		token.string = substring;
		token.byteRange = NSMakeRange(byteOffset, byteLength);
		[tokens addObject:token];
		byteOffset += byteLength;
	}];
	return tokens;
}

static void AppendCharacterRange (NSArray<DiffCharacterToken*>* tokens, NSRange tokenRange, NSUInteger line, NSMutableArray<DiffCharacterRange*>* ranges)
{
	if(tokenRange.length == 0)
		return;

	DiffCharacterToken* first = tokens[tokenRange.location];
	DiffCharacterToken* last = tokens[NSMaxRange(tokenRange) - 1];
	DiffCharacterRange* range = [[DiffCharacterRange alloc] init];
	range.line = line;
	range.byteColumns = NSMakeRange(first.byteRange.location, NSMaxRange(last.byteRange) - first.byteRange.location);
	[ranges addObject:range];
}

static NSUInteger ByteColumnForTokenBoundary (NSArray<DiffCharacterToken*>* tokens, NSUInteger tokenIndex)
{
	if(tokenIndex < tokens.count)
		return tokens[tokenIndex].byteRange.location;
	return tokens.count ? NSMaxRange(tokens.lastObject.byteRange) : 0;
}

static void AppendCharacterMarker (NSArray<DiffCharacterToken*>* tokens, NSUInteger tokenIndex, NSUInteger line, NSMutableArray<DiffCharacterRange*>* markers)
{
	DiffCharacterRange* marker = [[DiffCharacterRange alloc] init];
	marker.line = line;
	marker.byteColumns = NSMakeRange(ByteColumnForTokenBoundary(tokens, tokenIndex), 0);
	[markers addObject:marker];
}

static void AppendCharacterDifferences (NSString* leftLine, NSString* rightLine, NSUInteger leftLineNumber, NSUInteger rightLineNumber, NSMutableArray<DiffCharacterRange*>* leftRanges, NSMutableArray<DiffCharacterRange*>* rightRanges, NSMutableArray<DiffCharacterRange*>* leftMarkers, NSMutableArray<DiffCharacterRange*>* rightMarkers)
{
	NSArray<DiffCharacterToken*>* leftTokens = CharacterTokensForLine(leftLine);
	NSArray<DiffCharacterToken*>* rightTokens = CharacterTokensForLine(rightLine);
	if(leftTokens.count + rightTokens.count > 8192)
		return;

	NSArray<NSString*>* leftCharacters = [leftTokens valueForKey:@"string"];
	NSArray<NSString*>* rightCharacters = [rightTokens valueForKey:@"string"];
	NSOrderedCollectionDifference<NSString*>* difference = [rightCharacters differenceFromArray:leftCharacters];
	NSMutableIndexSet* removedCharacters = [NSMutableIndexSet indexSet];
	NSMutableIndexSet* insertedCharacters = [NSMutableIndexSet indexSet];
	for(NSOrderedCollectionChange<NSString*>* removal in difference.removals)
		[removedCharacters addIndex:removal.index];
	for(NSOrderedCollectionChange<NSString*>* insertion in difference.insertions)
		[insertedCharacters addIndex:insertion.index];

	NSUInteger leftIndex = 0;
	NSUInteger rightIndex = 0;
	while(leftIndex < leftTokens.count || rightIndex < rightTokens.count)
	{
		BOOL const leftChanged = leftIndex < leftTokens.count && [removedCharacters containsIndex:leftIndex];
		BOOL const rightChanged = rightIndex < rightTokens.count && [insertedCharacters containsIndex:rightIndex];
		if(leftIndex < leftTokens.count && rightIndex < rightTokens.count && !leftChanged && !rightChanged)
		{
			++leftIndex;
			++rightIndex;
			continue;
		}

		NSUInteger const leftStart = leftIndex;
		NSUInteger const rightStart = rightIndex;
		while(leftIndex < leftTokens.count && [removedCharacters containsIndex:leftIndex])
			++leftIndex;
		while(rightIndex < rightTokens.count && [insertedCharacters containsIndex:rightIndex])
			++rightIndex;
		NSUInteger const removedCount = leftIndex - leftStart;
		NSUInteger const insertedCount = rightIndex - rightStart;
		AppendCharacterRange(leftTokens, NSMakeRange(leftStart, removedCount), leftLineNumber, leftRanges);
		AppendCharacterRange(rightTokens, NSMakeRange(rightStart, insertedCount), rightLineNumber, rightRanges);
		if(insertedCount > removedCount)
			AppendCharacterMarker(leftTokens, leftIndex, leftLineNumber, leftMarkers);
		else if(removedCount > insertedCount)
			AppendCharacterMarker(rightTokens, rightIndex, rightLineNumber, rightMarkers);

		if(leftStart == leftIndex && rightStart == rightIndex)
			break;
	}
}

@interface WindowController () <NSWindowDelegate>
@property (nonatomic) NSWindowController* retainedSelf;
@property (nonatomic) OakDocumentView* leftDocumentView;
@property (nonatomic) OakDocumentView* rightDocumentView;
@property (nonatomic) DiffHighlightView* leftDiffHighlightView;
@property (nonatomic) DiffHighlightView* rightDiffHighlightView;
@property (nonatomic) NSSplitViewController* splitViewController;
@property (nonatomic, copy) NSString* leftPath;
@property (nonatomic, copy) NSString* rightPath;
@property (nonatomic) BOOL leftDocumentLoaded;
@property (nonatomic) BOOL rightDocumentLoaded;
@property (nonatomic) NSUInteger diffGeneration;
@property (nonatomic, copy) NSArray<NSNumber*>* leftToRightLineMap;
@property (nonatomic, copy) NSArray<NSNumber*>* rightToLeftLineMap;
@property (nonatomic, copy) NSArray<DiffScrollTransition*>* leftToRightScrollTransitions;
@property (nonatomic, copy) NSArray<DiffScrollTransition*>* rightToLeftScrollTransitions;
@property (nonatomic, copy) NSArray<DiffHunk*>* diffHunks;
@property (nonatomic) NSInteger activeDiffHunkIndex;
@property (nonatomic, weak) NSClipView* leftClipView;
@property (nonatomic, weak) NSClipView* rightClipView;
@property (nonatomic, weak) NSClipView* lastScrolledClipView;
@property (nonatomic) BOOL synchronizingScroll;
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
		_activeDiffHunkIndex = -1;

		self.leftDocumentView = [[OakDocumentView alloc] initWithFrame:NSZeroRect];
		self.leftDocumentView.document = [OakDocument documentWithString:@"" fileType:@"text.plain" customName:@"Left"];
		self.leftDocumentView.textView.softWrap = NO;
		self.leftDocumentView.textView.scrollPastEnd = YES;

		self.rightDocumentView = [[OakDocumentView alloc] initWithFrame:NSZeroRect];
		self.rightDocumentView.document = [OakDocument documentWithString:@"" fileType:@"text.plain" customName:@"Right"];
		self.rightDocumentView.textView.softWrap = NO;
		self.rightDocumentView.textView.scrollPastEnd = YES;

		self.leftDiffHighlightView = [[DiffHighlightView alloc] initWithTextView:self.leftDocumentView.textView color:[NSColor.systemRedColor colorWithAlphaComponent:0.16]];
		self.leftDiffHighlightView.gutterView = self.leftDocumentView.gutterView;
		__weak DiffHighlightView* weakLeftDiffHighlightView = self.leftDiffHighlightView;
		self.leftDocumentView.textView.decorationDrawingBlock = ^(NSRect dirtyRect, OTVDecorationLayer layer) {
			if(layer == OTVDecorationLayerBackground)
				[weakLeftDiffHighlightView drawBackgroundInRect:dirtyRect];
			else
				[weakLeftDiffHighlightView drawForegroundInRect:dirtyRect];
		};
		self.leftDocumentView.gutterView.backgroundDecorationDrawingBlock = ^(NSRect dirtyRect) {
			[weakLeftDiffHighlightView drawGutterBackgroundInRect:dirtyRect];
		};
		self.rightDiffHighlightView = [[DiffHighlightView alloc] initWithTextView:self.rightDocumentView.textView color:[NSColor.systemGreenColor colorWithAlphaComponent:0.16]];
		self.rightDiffHighlightView.gutterView = self.rightDocumentView.gutterView;
		__weak DiffHighlightView* weakRightDiffHighlightView = self.rightDiffHighlightView;
		self.rightDocumentView.textView.decorationDrawingBlock = ^(NSRect dirtyRect, OTVDecorationLayer layer) {
			if(layer == OTVDecorationLayerBackground)
				[weakRightDiffHighlightView drawBackgroundInRect:dirtyRect];
			else
				[weakRightDiffHighlightView drawForegroundInRect:dirtyRect];
		};
		self.rightDocumentView.gutterView.backgroundDecorationDrawingBlock = ^(NSRect dirtyRect) {
			[weakRightDiffHighlightView drawGutterBackgroundInRect:dirtyRect];
		};

		self.leftClipView = self.leftDocumentView.textView.enclosingScrollView.contentView;
		self.rightClipView = self.rightDocumentView.textView.enclosingScrollView.contentView;
		self.leftClipView.postsBoundsChangedNotifications = YES;
		self.rightClipView.postsBoundsChangedNotifications = YES;
		[NSNotificationCenter.defaultCenter addObserver:self selector:@selector(scrollBoundsDidChange:) name:NSViewBoundsDidChangeNotification object:self.leftClipView];
		[NSNotificationCenter.defaultCenter addObserver:self selector:@selector(scrollBoundsDidChange:) name:NSViewBoundsDidChangeNotification object:self.rightClipView];

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

- (CGFloat)bottomForLine:(NSUInteger)line lineCount:(NSUInteger)lineCount textView:(OakTextView*)textView
{
	if(line + 1 < lineCount)
	{
		GVLineRecord const nextLine = [textView lineFragmentForLine:line + 1 column:0];
		if(nextLine.lineNumber == line + 1)
			return nextLine.firstY;
	}

	GVLineRecord const lastFragment = [textView lineFragmentForLine:line column:NSUIntegerMax];
	return lastFragment.lastY;
}

- (CGFloat)yPositionForLineBoundary:(NSUInteger)position lineCount:(NSUInteger)lineCount textView:(OakTextView*)textView
{
	if(position < lineCount)
	{
		GVLineRecord const line = [textView lineFragmentForLine:position column:0];
		return line.firstY;
	}
	if(position == lineCount && lineCount != 0)
		return [self bottomForLine:lineCount - 1 lineCount:lineCount textView:textView];
	return 0;
}

- (void)scrollBoundsDidChange:(NSNotification*)notification
{
	if(self.synchronizingScroll)
		return;

	NSClipView* sourceClipView = notification.object;
	if(sourceClipView != self.leftClipView && sourceClipView != self.rightClipView)
		return;

	self.lastScrolledClipView = sourceClipView;
	[self synchronizeScrollFromClipView:sourceClipView];
}

- (void)synchronizeScrollFromClipView:(NSClipView*)sourceClipView
{
	if(!sourceClipView || self.synchronizingScroll)
		return;

	BOOL const sourceIsLeft = sourceClipView == self.leftClipView;
	NSClipView* targetClipView = sourceIsLeft ? self.rightClipView : self.leftClipView;
	OakTextView* sourceTextView = sourceIsLeft ? self.leftDocumentView.textView : self.rightDocumentView.textView;
	OakTextView* targetTextView = sourceIsLeft ? self.rightDocumentView.textView : self.leftDocumentView.textView;
	NSArray<NSNumber*>* lineMap = sourceIsLeft ? self.leftToRightLineMap : self.rightToLeftLineMap;
	NSArray<DiffScrollTransition*>* scrollTransitions = sourceIsLeft ? self.leftToRightScrollTransitions : self.rightToLeftScrollTransitions;
	NSUInteger const targetLineCount = (sourceIsLeft ? self.rightToLeftLineMap : self.leftToRightLineMap).count;
	if(!targetClipView || lineMap.count == 0 || targetLineCount == 0)
		return;

	NSRect const sourceBounds = sourceClipView.bounds;
	CGFloat const sourceAnchorY = NSMidY(sourceBounds);
	GVLineRecord const sourceLine = [sourceTextView lineRecordForPosition:sourceAnchorY];
	if(sourceLine.lineNumber >= lineMap.count)
		return;

	NSInteger const encodedTarget = lineMap[sourceLine.lineNumber].integerValue;
	BOOL const mapsToGap = encodedTarget < 0;
	NSUInteger const targetPosition = mapsToGap ? (NSUInteger)(-encodedTarget - 1) : (NSUInteger)encodedTarget;
	CGFloat targetY = [self yPositionForLineBoundary:targetPosition lineCount:targetLineCount textView:targetTextView];
	if(!mapsToGap && targetPosition < targetLineCount)
	{
		CGFloat const sourceLineHeight = sourceLine.lastY - sourceLine.firstY;
		CGFloat const fraction = sourceLineHeight > 0 ? std::clamp((sourceAnchorY - sourceLine.firstY) / sourceLineHeight, (CGFloat)0, (CGFloat)1) : 0;
		CGFloat const targetBottom = [self bottomForLine:targetPosition lineCount:targetLineCount textView:targetTextView];
		targetY += fraction * MAX(0, targetBottom - targetY);
	}

	CGFloat const transitionLength = MAX(NSHeight(sourceBounds), 4 * MAX((CGFloat)1, sourceLine.lastY - sourceLine.firstY));
	for(DiffScrollTransition* transition in scrollTransitions)
	{
		CGFloat const sourceBoundaryY = [self yPositionForLineBoundary:transition.sourceBoundary lineCount:lineMap.count textView:sourceTextView];
		CGFloat const targetStartY = [self yPositionForLineBoundary:transition.targetStart lineCount:targetLineCount textView:targetTextView];
		CGFloat const targetEndY = [self yPositionForLineBoundary:transition.targetEnd lineCount:targetLineCount textView:targetTextView];
		CGFloat const insertedHeight = targetEndY - targetStartY;
		CGFloat progress = std::clamp((sourceAnchorY - (sourceBoundaryY - transitionLength / 2)) / transitionLength, (CGFloat)0, (CGFloat)1);
		progress = progress * progress * (3 - 2 * progress);
		CGFloat const discreteProgress = sourceAnchorY < sourceBoundaryY ? 0 : 1;
		targetY += insertedHeight * (progress - discreteProgress);
	}

	NSRect targetBounds = targetClipView.bounds;
	targetBounds.origin = NSMakePoint(NSMinX(sourceBounds), targetY - NSHeight(targetBounds) / 2);
	targetBounds = [targetClipView constrainBoundsRect:targetBounds];
	if(fabs(NSMinX(targetBounds) - NSMinX(targetClipView.bounds)) < 0.25 && fabs(NSMinY(targetBounds) - NSMinY(targetClipView.bounds)) < 0.25)
		return;

	self.synchronizingScroll = YES;
	[targetClipView scrollToPoint:targetBounds.origin];
	[targetClipView.enclosingScrollView reflectScrolledClipView:targetClipView];
	self.synchronizingScroll = NO;
}

- (void)updateActiveChangeHighlights
{
	if(self.activeDiffHunkIndex < 0 || self.activeDiffHunkIndex >= (NSInteger)self.diffHunks.count)
	{
		self.leftDiffHighlightView.activeHighlightedLines = NSIndexSet.indexSet;
		self.leftDiffHighlightView.activeMarkerPositions = NSIndexSet.indexSet;
		self.rightDiffHighlightView.activeHighlightedLines = NSIndexSet.indexSet;
		self.rightDiffHighlightView.activeMarkerPositions = NSIndexSet.indexSet;
		return;
	}

	DiffHunk* hunk = self.diffHunks[self.activeDiffHunkIndex];
	self.leftDiffHighlightView.activeHighlightedLines = hunk.leftLines.length ? [NSIndexSet indexSetWithIndexesInRange:hunk.leftLines] : NSIndexSet.indexSet;
	self.leftDiffHighlightView.activeMarkerPositions = hunk.leftLines.length ? NSIndexSet.indexSet : [NSIndexSet indexSetWithIndex:hunk.leftLines.location];
	self.rightDiffHighlightView.activeHighlightedLines = hunk.rightLines.length ? [NSIndexSet indexSetWithIndexesInRange:hunk.rightLines] : NSIndexSet.indexSet;
	self.rightDiffHighlightView.activeMarkerPositions = hunk.rightLines.length ? NSIndexSet.indexSet : [NSIndexSet indexSetWithIndex:hunk.rightLines.location];
}

- (CGFloat)centerYForLineRange:(NSRange)lineRange lineCount:(NSUInteger)lineCount textView:(OakTextView*)textView
{
	CGFloat const startY = [self yPositionForLineBoundary:lineRange.location lineCount:lineCount textView:textView];
	if(lineRange.length == 0)
		return startY;
	CGFloat const endY = [self yPositionForLineBoundary:NSMaxRange(lineRange) lineCount:lineCount textView:textView];
	return (startY + endY) / 2;
}

- (void)centerActiveChange
{
	if(self.activeDiffHunkIndex < 0 || self.activeDiffHunkIndex >= (NSInteger)self.diffHunks.count)
		return;

	DiffHunk* hunk = self.diffHunks[self.activeDiffHunkIndex];
	CGFloat const leftY = [self centerYForLineRange:hunk.leftLines lineCount:self.leftToRightLineMap.count textView:self.leftDocumentView.textView];
	CGFloat const rightY = [self centerYForLineRange:hunk.rightLines lineCount:self.rightToLeftLineMap.count textView:self.rightDocumentView.textView];

	self.synchronizingScroll = YES;
	NSArray<NSDictionary*>* scrollRequests = @[
		@{ @"clipView": self.leftClipView, @"y": @(leftY) },
		@{ @"clipView": self.rightClipView, @"y": @(rightY) },
	];
	for(NSDictionary* request in scrollRequests)
	{
		NSClipView* clipView = request[@"clipView"];
		NSRect bounds = clipView.bounds;
		bounds.origin.y = [request[@"y"] doubleValue] - NSHeight(bounds) / 2;
		bounds = [clipView constrainBoundsRect:bounds];
		[clipView scrollToPoint:bounds.origin];
		[clipView.enclosingScrollView reflectScrolledClipView:clipView];
	}
	self.synchronizingScroll = NO;
	self.lastScrolledClipView = self.leftClipView;
}

- (void)selectChangeWithOffset:(NSInteger)offset
{
	if(self.diffHunks.count == 0)
	{
		NSBeep();
		return;
	}

	if(self.activeDiffHunkIndex < 0)
		self.activeDiffHunkIndex = offset < 0 ? self.diffHunks.count - 1 : 0;
	else
	{
		NSInteger const hunkCount = (NSInteger)self.diffHunks.count;
		self.activeDiffHunkIndex = (self.activeDiffHunkIndex + offset + hunkCount) % hunkCount;
	}

	[self updateActiveChangeHighlights];
	[self centerActiveChange];
}

- (IBAction)nextChange:(id)sender
{
	[self selectChangeWithOffset:1];
}

- (IBAction)previousChange:(id)sender
{
	[self selectChangeWithOffset:-1];
}

- (void)copyActiveChangeToLeft:(BOOL)copyToLeft
{
	if(self.activeDiffHunkIndex < 0 || self.activeDiffHunkIndex >= (NSInteger)self.diffHunks.count)
	{
		NSBeep();
		return;
	}

	DiffHunk* hunk = self.diffHunks[self.activeDiffHunkIndex];
	OakDocument* sourceDocument = copyToLeft ? self.rightDocumentView.document : self.leftDocumentView.document;
	OakDocument* targetDocument = copyToLeft ? self.leftDocumentView.document : self.rightDocumentView.document;
	NSRange const sourceRange = copyToLeft ? hunk.rightLines : hunk.leftLines;
	NSRange const targetRange = copyToLeft ? hunk.leftLines : hunk.rightLines;
	NSArray<NSString*>* sourceLines = [sourceDocument.content ?: @"" componentsSeparatedByString:@"\n"];
	NSMutableArray<NSString*>* targetLines = [[targetDocument.content ?: @"" componentsSeparatedByString:@"\n"] mutableCopy];
	if(NSMaxRange(sourceRange) > sourceLines.count || NSMaxRange(targetRange) > targetLines.count)
	{
		NSBeep();
		return;
	}

	NSArray<NSString*>* replacementLines = [sourceLines subarrayWithRange:sourceRange];
	[targetLines replaceObjectsInRange:targetRange withObjectsFromArray:replacementLines];
	NSString* replacementContent = [targetLines componentsJoinedByString:@"\n"];
	NSString* currentTargetContent = targetDocument.content ?: @"";
	std::string const oldContent(currentTargetContent.UTF8String);
	std::multimap<std::pair<size_t, size_t>, std::string> replacements;
	replacements.emplace(std::make_pair(0, oldContent.size()), std::string(replacementContent.UTF8String));
	if(![targetDocument performReplacements:replacements checksum:0])
		NSBeep();
}

- (IBAction)copyChangeToLeft:(id)sender
{
	[self copyActiveChangeToLeft:YES];
}

- (IBAction)copyChangeToRight:(id)sender
{
	[self copyActiveChangeToLeft:NO];
}

- (void)scheduleDiffUpdate
{
	if(!self.leftDocumentLoaded || !self.rightDocumentLoaded)
		return;

	[NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(updateDiff) object:nil];
	[self performSelector:@selector(updateDiff) withObject:nil afterDelay:0.12];
}

- (void)updateDiff
{
	NSString* leftContent = self.leftDocumentView.document.content ?: @"";
	NSString* rightContent = self.rightDocumentView.document.content ?: @"";
	NSUInteger const generation = ++self.diffGeneration;
	__weak WindowController* weakSelf = self;

	dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
		NSArray<NSString*>* leftLines = [leftContent componentsSeparatedByString:@"\n"];
		NSArray<NSString*>* rightLines = [rightContent componentsSeparatedByString:@"\n"];
		NSOrderedCollectionDifference<NSString*>* difference = [rightLines differenceFromArray:leftLines];
		NSMutableIndexSet* removedLines = [NSMutableIndexSet indexSet];
		NSMutableIndexSet* insertedLines = [NSMutableIndexSet indexSet];
		NSMutableIndexSet* leftMarkerPositions = [NSMutableIndexSet indexSet];
		NSMutableIndexSet* rightMarkerPositions = [NSMutableIndexSet indexSet];
		NSMutableArray<DiffCharacterRange*>* leftCharacterRanges = [NSMutableArray array];
		NSMutableArray<DiffCharacterRange*>* rightCharacterRanges = [NSMutableArray array];
		NSMutableArray<DiffCharacterRange*>* leftCharacterMarkers = [NSMutableArray array];
		NSMutableArray<DiffCharacterRange*>* rightCharacterMarkers = [NSMutableArray array];
		NSMutableArray<NSNumber*>* leftToRightLineMap = [NSMutableArray arrayWithCapacity:leftLines.count];
		NSMutableArray<NSNumber*>* rightToLeftLineMap = [NSMutableArray arrayWithCapacity:rightLines.count];
		NSMutableArray<DiffScrollTransition*>* leftToRightScrollTransitions = [NSMutableArray array];
		NSMutableArray<DiffScrollTransition*>* rightToLeftScrollTransitions = [NSMutableArray array];
		NSMutableArray<DiffHunk*>* diffHunks = [NSMutableArray array];
		for(NSUInteger i = 0; i < leftLines.count; ++i)
			[leftToRightLineMap addObject:@0];
		for(NSUInteger i = 0; i < rightLines.count; ++i)
			[rightToLeftLineMap addObject:@0];

		for(NSOrderedCollectionChange<NSString*>* removal in difference.removals)
			[removedLines addIndex:removal.index];
		for(NSOrderedCollectionChange<NSString*>* insertion in difference.insertions)
			[insertedLines addIndex:insertion.index];

		NSUInteger leftIndex = 0;
		NSUInteger rightIndex = 0;
		while(leftIndex < leftLines.count || rightIndex < rightLines.count)
		{
			BOOL const leftChanged = leftIndex < leftLines.count && [removedLines containsIndex:leftIndex];
			BOOL const rightChanged = rightIndex < rightLines.count && [insertedLines containsIndex:rightIndex];
			if(leftIndex < leftLines.count && rightIndex < rightLines.count && !leftChanged && !rightChanged)
			{
				leftToRightLineMap[leftIndex] = @(rightIndex);
				rightToLeftLineMap[rightIndex] = @(leftIndex);
				++leftIndex;
				++rightIndex;
				continue;
			}

			NSUInteger const leftHunkStart = leftIndex;
			NSUInteger const rightHunkStart = rightIndex;
			while(leftIndex < leftLines.count && [removedLines containsIndex:leftIndex])
				++leftIndex;
			while(rightIndex < rightLines.count && [insertedLines containsIndex:rightIndex])
				++rightIndex;

			NSUInteger const removedCount = leftIndex - leftHunkStart;
			NSUInteger const insertedCount = rightIndex - rightHunkStart;
			DiffHunk* hunk = [[DiffHunk alloc] init];
			hunk.leftLines = NSMakeRange(leftHunkStart, removedCount);
			hunk.rightLines = NSMakeRange(rightHunkStart, insertedCount);
			[diffHunks addObject:hunk];

			NSUInteger const pairedCount = MIN(removedCount, insertedCount);
			for(NSUInteger i = 0; i < pairedCount; ++i)
			{
				leftToRightLineMap[leftHunkStart + i] = @(rightHunkStart + i);
				rightToLeftLineMap[rightHunkStart + i] = @(leftHunkStart + i);
				AppendCharacterDifferences(leftLines[leftHunkStart + i], rightLines[rightHunkStart + i], leftHunkStart + i, rightHunkStart + i, leftCharacterRanges, rightCharacterRanges, leftCharacterMarkers, rightCharacterMarkers);
			}
			for(NSUInteger i = pairedCount; i < removedCount; ++i)
				leftToRightLineMap[leftHunkStart + i] = @(-((NSInteger)rightIndex) - 1);
			for(NSUInteger i = pairedCount; i < insertedCount; ++i)
				rightToLeftLineMap[rightHunkStart + i] = @(-((NSInteger)leftIndex) - 1);

			if(insertedCount > removedCount)
			{
				[leftMarkerPositions addIndex:leftIndex];
				DiffScrollTransition* transition = [[DiffScrollTransition alloc] init];
				transition.sourceBoundary = leftIndex;
				transition.targetStart = rightHunkStart + pairedCount;
				transition.targetEnd = rightIndex;
				[leftToRightScrollTransitions addObject:transition];
			}
			else if(removedCount > insertedCount)
			{
				[rightMarkerPositions addIndex:rightIndex];
				DiffScrollTransition* transition = [[DiffScrollTransition alloc] init];
				transition.sourceBoundary = rightIndex;
				transition.targetStart = leftHunkStart + pairedCount;
				transition.targetEnd = leftIndex;
				[rightToLeftScrollTransitions addObject:transition];
			}

			if(removedCount == 0 && insertedCount == 0)
				break;
		}

		dispatch_async(dispatch_get_main_queue(), ^{
			WindowController* strongSelf = weakSelf;
			if(!strongSelf || strongSelf.diffGeneration != generation)
				return;
			strongSelf.leftDiffHighlightView.highlightedLines = removedLines;
			strongSelf.leftDiffHighlightView.markerPositions = leftMarkerPositions;
			strongSelf.leftDiffHighlightView.characterRanges = leftCharacterRanges;
			strongSelf.leftDiffHighlightView.characterMarkers = leftCharacterMarkers;
			strongSelf.leftDiffHighlightView.lineCount = leftLines.count;
			strongSelf.rightDiffHighlightView.highlightedLines = insertedLines;
			strongSelf.rightDiffHighlightView.markerPositions = rightMarkerPositions;
			strongSelf.rightDiffHighlightView.characterRanges = rightCharacterRanges;
			strongSelf.rightDiffHighlightView.characterMarkers = rightCharacterMarkers;
			strongSelf.rightDiffHighlightView.lineCount = rightLines.count;
			strongSelf.leftToRightLineMap = leftToRightLineMap;
			strongSelf.rightToLeftLineMap = rightToLeftLineMap;
			strongSelf.leftToRightScrollTransitions = leftToRightScrollTransitions;
			strongSelf.rightToLeftScrollTransitions = rightToLeftScrollTransitions;
			strongSelf.diffHunks = diffHunks;
			if(diffHunks.count == 0)
				strongSelf.activeDiffHunkIndex = -1;
			else if(strongSelf.activeDiffHunkIndex >= (NSInteger)diffHunks.count)
				strongSelf.activeDiffHunkIndex = diffHunks.count - 1;
			[strongSelf updateActiveChangeHighlights];
			[strongSelf synchronizeScrollFromClipView:strongSelf.lastScrolledClipView ?: strongSelf.leftClipView];
		});
	});
}

- (void)documentContentDidChange:(NSNotification*)notification
{
	if(notification.object == self.leftDocumentView.document || notification.object == self.rightDocumentView.document)
	{
		++self.diffGeneration;
		self.diffHunks = @[];
		self.activeDiffHunkIndex = -1;
		self.leftDiffHighlightView.characterRanges = @[];
		self.rightDiffHighlightView.characterRanges = @[];
		self.leftDiffHighlightView.characterMarkers = @[];
		self.rightDiffHighlightView.characterMarkers = @[];
		[self updateActiveChangeHighlights];
		[self scheduleDiffUpdate];
	}
}

- (void)loadDocumentAtPath:(NSString*)path intoDocumentView:(OakDocumentView*)documentView sideName:(NSString*)sideName
{
	OakDocument* document = [OakDocument documentWithPath:path];
	[document loadModalForWindow:self.window completionHandler:^(OakDocumentIOResult result, NSString* errorMessage, oak::uuid_t const& filterUUID) {
		if(result == OakDocumentIOResultSuccess)
		{
			documentView.document = document;
			[NSNotificationCenter.defaultCenter addObserver:self selector:@selector(documentContentDidChange:) name:OakDocumentContentDidChangeNotification object:document];
			if(documentView == self.leftDocumentView)
				self.leftDocumentLoaded = YES;
			else if(documentView == self.rightDocumentView)
				self.rightDocumentLoaded = YES;
			[self scheduleDiffUpdate];
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
	[NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(updateDiff) object:nil];
	++self.diffGeneration;
	[NSNotificationCenter.defaultCenter removeObserver:self];
	_retainedSelf = nil;
}
@end
