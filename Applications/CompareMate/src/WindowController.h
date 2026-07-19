@interface WindowController : NSWindowController <NSWindowRestoration>
- (instancetype)initWithLeftPath:(NSString*)leftPath rightPath:(NSString*)rightPath;
- (IBAction)nextChange:(id)sender;
- (IBAction)previousChange:(id)sender;
- (IBAction)copyChangeToLeft:(id)sender;
- (IBAction)copyChangeToRight:(id)sender;
@end
