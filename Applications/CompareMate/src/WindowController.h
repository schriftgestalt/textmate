@interface WindowController : NSWindowController <NSWindowRestoration>
- (instancetype)initWithLeftPath:(NSString*)leftPath rightPath:(NSString*)rightPath;
@end
