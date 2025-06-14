#import "HOStatusBar.h"
#import <OakAppKit/src/NSImage Additions.h>
#import <OakAppKit/src/OakUIConstructionFunctions.h>

static NSButton* OakCreateImageButton (NSImage* image)
{
	NSButton* res = [NSButton new];
	[res setButtonType:NSButtonTypeMomentaryChange];
	[res setBordered:NO];
	[res setImage:image];
	[res setImagePosition:NSImageOnly];
	return res;
}

static NSTextField* OakCreateTextField ()
{
	NSTextField* res = [[NSTextField alloc] initWithFrame:NSZeroRect];
	[res setBordered:NO];
	[res setEditable:NO];
	[res setSelectable:NO];
	[res setBezeled:NO];
	[res setDrawsBackground:NO];
	[res setFont:OakStatusBarFont()];
	return res;
}

@interface HOStatusBar ()
@property (nonatomic) NSView*              topDivider;
@property (nonatomic) NSView*              divider;
@property (nonatomic) NSButton*            goBackButton;
@property (nonatomic) NSButton*            goForwardButton;
@property (nonatomic) NSTextField*         statusTextField;
@property (nonatomic) NSProgressIndicator* progressIndicator;
@property (nonatomic) NSProgressIndicator* spinner;
@property (nonatomic) NSMutableArray*      layoutConstraints;
@property (nonatomic) BOOL                 indeterminateProgress;
@end

@implementation HOStatusBar
- (id)initWithFrame:(NSRect)frame
{
	if(self = [super initWithFrame:frame])
	{
		self.wantsLayer   = YES;
		self.material     = NSVisualEffectMaterialTitlebar;
		self.blendingMode = NSVisualEffectBlendingModeWithinWindow;
		self.state        = NSVisualEffectStateFollowsWindowActiveState;

		_indeterminateProgress = YES;

		_topDivider               = OakCreateNSBoxSeparator();
		_divider                  = OakCreateNSBoxSeparator();

		_goBackButton             = OakCreateImageButton([NSImage imageNamed:NSImageNameGoLeftTemplate]);
		_goBackButton.toolTip     = @"Show the previous page";
		_goBackButton.enabled     = NO;
		_goBackButton.target      = self;
		_goBackButton.action      = @selector(goBack:);

		_goForwardButton          = OakCreateImageButton([NSImage imageNamed:NSImageNameGoRightTemplate]);
		_goForwardButton.toolTip  = @"Show the next page";
		_goForwardButton.enabled  = NO;
		_goForwardButton.target   = self;
		_goForwardButton.action   = @selector(goForward:);

		_statusTextField          = OakCreateTextField();
		[_statusTextField setContentCompressionResistancePriority:NSLayoutPriorityDefaultLow forOrientation:NSLayoutConstraintOrientationHorizontal];
		[_statusTextField.cell setLineBreakMode:NSLineBreakByTruncatingMiddle];

		_progressIndicator = [NSProgressIndicator new];
		_progressIndicator.controlSize          = NSControlSizeSmall;
		_progressIndicator.maxValue             = 1;
		_progressIndicator.indeterminate        = NO;
		_progressIndicator.displayedWhenStopped = NO;
		_progressIndicator.bezeled              = NO;

		_spinner = [NSProgressIndicator new];
		_spinner.controlSize          = NSControlSizeSmall;
		_spinner.style                = NSProgressIndicatorStyleSpinning;
		_spinner.displayedWhenStopped = NO;

		NSArray* views = @[ _topDivider, _divider, _goBackButton, _goForwardButton, _statusTextField, _spinner ];
		OakAddAutoLayoutViewsToSuperview(views, self);

		[_progressIndicator setTranslatesAutoresizingMaskIntoConstraints:NO];
	}
	return self;
}

- (void)updateConstraints
{
	if(_layoutConstraints)
		[self removeConstraints:_layoutConstraints];
	_layoutConstraints = [NSMutableArray array];

	[super updateConstraints];

	NSDictionary* views = @{
		@"topDivider": _topDivider,
		@"back":       _goBackButton,
		@"forward":    _goForwardButton,
		@"divider":    _divider,
		@"status":     _statusTextField,
		@"spinner":    _indeterminateProgress ? _spinner : _progressIndicator,
	};

	NSArray* layout = @[
		@"H:|[topDivider]|", @"V:|[topDivider(==1)]-4-[divider(==15)]-5-|", @"V:[status]-5-|"
	];

	for(NSString* str in layout)
		[_layoutConstraints addObjectsFromArray:[NSLayoutConstraint constraintsWithVisualFormat:str options:0 metrics:nil views:views]];
	[_layoutConstraints addObjectsFromArray:[NSLayoutConstraint constraintsWithVisualFormat:@"H:|-(3)-[back(==22)]-(2)-[forward(==back)]-(2)-[divider(==1)]" options:NSLayoutFormatAlignAllCenterY metrics:nil views:views]];

	if(!_indeterminateProgress)
	{
		[_layoutConstraints addObjectsFromArray:[NSLayoutConstraint constraintsWithVisualFormat:@"H:[divider]-[status(>=100)]-[spinner(>=50,<=150)]-|" options:0 metrics:nil views:views]];
		[_layoutConstraints addObjectsFromArray:[NSLayoutConstraint constraintsWithVisualFormat:@"V:[spinner]-6-|" options:0 metrics:nil views:views]];
	}
	else
	{
		[_layoutConstraints addObjectsFromArray:[NSLayoutConstraint constraintsWithVisualFormat:@"H:[divider]-[status(>=100)]-[spinner]-|" options:0 metrics:nil views:views]];
		[_layoutConstraints addObjectsFromArray:[NSLayoutConstraint constraintsWithVisualFormat:@"V:[spinner]-5-|" options:0 metrics:nil views:views]];
	}

	[self addConstraints:_layoutConstraints];
}

- (void)setIndeterminateProgress:(BOOL)newIndeterminateProgress
{
	if(_indeterminateProgress == newIndeterminateProgress)
		return;

	if(_indeterminateProgress = newIndeterminateProgress)
	{
		[_progressIndicator removeFromSuperview];
		[self addSubview:_spinner];
		if(_busy)
			[_spinner startAnimation:nil];
	}
	else
	{
		[self addSubview:_progressIndicator];
		if(_busy)
			[_spinner stopAnimation:nil];
		[_spinner removeFromSuperview];
	}
	[self setNeedsUpdateConstraints:YES];
}

- (void)setBusy:(BOOL)flag
{
	_busy = flag;
	if(_indeterminateProgress)
	{
		if(_busy)
				[_spinner startAnimation:nil];
		else	[_spinner stopAnimation:nil];
	}
}

- (void)goBack:(id)sender             { [NSApp sendAction:@selector(goBack:)    to:_delegate from:self]; }
- (void)goForward:(id)sender          { [NSApp sendAction:@selector(goForward:) to:_delegate from:self]; }

- (BOOL)canGoBack                     { return _goBackButton.isEnabled; }
- (BOOL)canGoForward                  { return _goForwardButton.isEnabled; }
- (NSString*)statusText               { return _statusTextField.stringValue; }
- (CGFloat)progress                   { return _progressIndicator.doubleValue; }

- (void)setCanGoBack:(BOOL)flag       { _goBackButton.enabled = flag; }
- (void)setCanGoForward:(BOOL)flag    { _goForwardButton.enabled = flag; }
- (void)setStatusText:(NSString*)text { _statusTextField.stringValue = text; }

- (void)setProgress:(CGFloat)value
{
	_progressIndicator.doubleValue = value;
	self.indeterminateProgress = value == 0;
}
@end
