#import "Private.h"
#import <OakAppKit/src/NSMenuItem Additions.h>
#import <OakFoundation/src/NSString Additions.h>
#import <ns/src/ns.h>
#import <oak/debug.h>

@interface NSObject (HasSelection)
- (BOOL)hasSelection;
- (scope::context_t)scopeContext;
@end

@implementation BundleMenuDelegate
+ (instancetype)sharedInstance
{
	static BundleMenuDelegate* sharedInstance = [self new];
	return sharedInstance;
}

- (BOOL)menuHasKeyEquivalent:(NSMenu*)aMenu forEvent:(NSEvent*)theEvent target:(id*)aTarget action:(SEL*)anAction
{
	return NO;
}

- (void)menuNeedsUpdate:(NSMenu*)aMenu
{
	[aMenu removeAllItems];

	scope::context_t scope = "";
	if(id textView = [NSApp targetForAction:@selector(scopeContext)])
		scope = [textView scopeContext];

	bundles::item_ptr umbrellaItem = bundles::lookup(to_s(aMenu.title));
	if(!umbrellaItem)
		return;

	for(auto const& item : umbrellaItem->menu())
	{
		switch(item->kind())
		{
			case bundles::kItemTypeMenu:
			{
				NSMenuItem* menuItem = [aMenu addItemWithTitle:[NSString stringWithCxxString:item->name()] action:NULL keyEquivalent:@""];

				menuItem.submenu = [[NSMenu alloc] initWithTitle:[NSString stringWithCxxString:item->uuid()]];
				menuItem.submenu.delegate = BundleMenuDelegate.sharedInstance;
			}
			break;

			case bundles::kItemTypeMenuItemSeparator:
				[aMenu addItem:[NSMenuItem separatorItem]];
			break;

			case bundles::kItemTypeProxy:
			{
				auto const items = bundles::items_for_proxy(item, scope);
				OakAddBundlesToMenu(items, true, aMenu, @selector(performBundleItemWithUUIDStringFrom:));

				if(items.empty())
				{
					NSMenuItem* menuItem = [aMenu addItemWithTitle:[NSString stringWithCxxString:item->name()] action:@selector(nop:) keyEquivalent:@""];
					[menuItem setInactiveKeyEquivalentCxxString:key_equivalent(item)];
					[menuItem setTabTriggerCxxString:item->value_for_field(bundles::kFieldTabTrigger)];
				}
			}
			break;

			default:
			{
				NSMenuItem* menuItem = [aMenu addItemWithTitle:[NSString stringWithCxxString:item->name()] action:@selector(performBundleItemWithUUIDStringFrom:) keyEquivalent:@""];
				[menuItem setInactiveKeyEquivalentCxxString:key_equivalent(item)];
				[menuItem setTabTriggerCxxString:item->value_for_field(bundles::kFieldTabTrigger)];
				[menuItem setRepresentedObject:[NSString stringWithCxxString:item->uuid()]];
			}
			break;
		}
	}
}
@end
