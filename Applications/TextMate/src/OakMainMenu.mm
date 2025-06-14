#import "OakMainMenu.h"
#import <ns/src/ns.h>
#import <bundles/src/bundles.h>
#import <OakFoundation/src/NSString Additions.h>
#import <BundleMenu/src/BundleMenu.h>
#import <crash/src/info.h>

/*

The route of an event seems to be:

 - OakTextView performKeyEquivalent:
 - OakMainMenu performKeyEquivalent:
 - OakTextView keyDown:

Handling keys in keyDown: (and trusting NSMenu to handle menu items) leads to a
few problems:

 - If multiple menu items share key, NSMenu will pick one at random. We can
   make a better choice since we know about scope selectors.
 - If user started a multi-stroke key sequence, NSMenu will not know about it
   and may disrupt it (by firing a menu item for one of the keys involved).
 - Some “special keys” do not make it to OakTextView’s keyDown: (e.g. control
   left/right).

For this reason we:

 - Handle bundle items, “special keys”, and multi-stroke sequences in
   OakTextView performKeyEquivalent:
 - Bypass NSMenu’s performKeyEquivalent: for the bundles menu.
 - Handle bundle items in OakMainMenu performKeyEquivalent: — this is incase
   there are no windows open.

One downside is that we do not get the Bundles menu flashing when the user
picks from that menu (via a key equivalent). To achieve this, I am thinking it
might be possible to replace the Bundles menu with one that has just one item
(with the key equivalent pressed) and then call performKeyEquivalent: on this
submenu.

*/

static CGPoint MenuPosition ()
{
	NSPoint pos = [NSEvent mouseLocation];
	pos.y -= 16;

	return pos;
}

@implementation OakMainMenu
- (BOOL)performWindowMenuAction:(SEL)anAction
{
	NSMenu* windowMenu = [[self itemWithTitle:@"Window"] submenu];
	NSInteger index = [windowMenu indexOfItemWithTarget:nil andAction:anAction];
	if(!windowMenu || index == -1)
		return [NSApp sendAction:anAction to:nil from:self];

	[windowMenu update];
	if(![[windowMenu itemAtIndex:index] isEnabled])
		return NO;

	[windowMenu performActionForItemAtIndex:index];
	return YES;
}

- (BOOL)performKeyEquivalent:(NSEvent*)anEvent
{
	std::string const keyString = to_s(anEvent);
	crash_reporter_info_t info("Key equivalent ‘%s’.", keyString.c_str());

	auto const bundleItems = bundles::query(bundles::kFieldKeyEquivalent, keyString, "", bundles::kItemTypeCommand|bundles::kItemTypeGrammar|bundles::kItemTypeSnippet);
	if(!bundleItems.empty())
	{
		if(bundles::item_ptr item = OakShowMenuForBundleItems(bundleItems, nil, MenuPosition()))
			[NSApp sendAction:@selector(performBundleItemWithUUIDStringFrom:) to:nil from:@{ @"representedObject": to_ns(item->uuid()) }];
		return YES;
	}

	if([super performKeyEquivalent:anEvent])
		return YES;
	else if(@available(macos 10.13, *))
		return NO;
	else if(keyString == "~@\uF702" || keyString == "@{") // ⌥⌘⇠ or ⌘{
		return [self performWindowMenuAction:@selector(selectPreviousTab:)];
	else if(keyString == "~@\uF703" || keyString == "@}") // ⌥⌘⇢ or ⌘}
		return [self performWindowMenuAction:@selector(selectNextTab:)];
	return NO;
}
@end
