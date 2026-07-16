#import "OakTextView.h"
#import <oak/debug.h>

@class OakDocument;

@interface OakDocumentView : NSView
@property (nonatomic, readonly) OakTextView* textView;
@property (nonatomic) OakDocument* document;
@property (nonatomic) BOOL hideStatusBar;
@property (nonatomic, readonly) NSView* scopeBarView;
@property (nonatomic) BOOL showsScopeBar;
- (IBAction)toggleLineNumbers:(id)sender;
- (IBAction)takeTabSizeFrom:(id)sender;
- (IBAction)showTabSizeSelectorPanel:(id)sender;

- (void)addAuxiliaryView:(NSView*)aView atEdge:(NSRectEdge)anEdge;
- (void)removeAuxiliaryView:(NSView*)aView;

- (IBAction)showSymbolChooser:(id)sender;
@end
