#import "Keys.h"
#import <OakTabBarView/src/OakTabBarView.h>
#import <BundlesManager/src/BundlesManager.h>

static NSArray* default_environment ()
{
	return @[
		@{ @"enabled": @NO, @"name": @"PATH",            @"value": @"$PATH:/opt/local/bin:/usr/local/bin:/usr/texbin" },
		@{ @"enabled": @NO, @"name": @"TM_C_POINTER",    @"value": @"* "                               },
		@{ @"enabled": @NO, @"name": @"TM_CXX_FLAGS",    @"value": @"-framework Carbon -liconv -include vector -include string -include map -include cstdio -funsigned-char -Wall -Wwrite-strings -Wformat=2 -Winit-self -Wmissing-include-dirs -Wno-parentheses -Wno-sign-compare -Wno-switch" },
		@{ @"enabled": @NO, @"name": @"TM_FULLNAME",     @"value": @"Scrooge McDuck"                   },
		@{ @"enabled": @NO, @"name": @"TM_ORGANIZATION", @"value": @"The Billionaires Club"            },
		@{ @"enabled": @NO, @"name": @"TM_XHTML",        @"value": @" /"                               },
		@{ @"enabled": @NO, @"name": @"TM_GIT",          @"value": @"/opt/local/bin/git"               },
		@{ @"enabled": @NO, @"name": @"TM_HG",           @"value": @"/opt/local/bin/hg"                },
		@{ @"enabled": @NO, @"name": @"TM_MAKE_FLAGS",   @"value": @"rj8"                              },
	];
}

static NSDictionary* default_settings ()
{
	return @{
		kUserDefaultsHTMLOutputPlacementKey:     @"window",
		kUserDefaultsFileBrowserPlacementKey:    @"right",
		kUserDefaultsShowFileExtensionsKey:      @NO,
		kUserDefaultsEnvironmentVariablesKey:    default_environment(),
		kUserDefaultsDisableBundleUpdatesKey:    @NO,
		kUserDefaultsDisableRMateServerKey:      @NO,
		kUserDefaultsRMateServerListenKey:       kRMateServerListenLocalhost,
		kUserDefaultsRMateServerPortKey:         @"52698",
		kUserDefaultsLicenseOwnerKey:            NSFullUserName(),
		kUserDefaultsLineNumbersKey:             @YES,
		kUserDefaultsCrashReportsContactInfoKey: NSFullUserName() ?: @"Anonymous",
	};
}

static bool register_defaults ()
{
	[NSUserDefaults.standardUserDefaults registerDefaults:default_settings()];
	return true;
}

void RegisterDefaults ()
{
	static bool __attribute__ ((unused)) dummy = register_defaults();
}

// =========
// = Files =
// =========

NSString* const kUserDefaultsDisableSessionRestoreKey            = @"disableSessionRestore";
NSString* const kUserDefaultsDisableNewDocumentAtStartupKey      = @"disableNewDocumentAtStartup";
NSString* const kUserDefaultsDisableNewDocumentAtReactivationKey = @"disableNewDocumentAtReactivation";
NSString* const kUserDefaultsShowFavoritesInsteadOfUntitledKey   = @"showFavoritesInsteadOfUntitled";

// ============
// = Projects =
// ============

NSString* const kUserDefaultsFoldersOnTopKey                   = @"foldersOnTop";
NSString* const kUserDefaultsShowFileExtensionsKey             = @"showFileExtensions";
NSString* const kUserDefaultsInitialFileBrowserURLKey          = @"initialFileBrowserURL";
NSString* const kUserDefaultsFileBrowserPlacementKey           = @"fileBrowserPlacement";
NSString* const kUserDefaultsFileBrowserSingleClickToOpenKey   = @"fileBrowserSingleClickToOpen";
NSString* const kUserDefaultsFileBrowserOpenAnimationDisabled  = @"fileBrowserOpenAnimationDisabled";
NSString* const kUserDefaultsFileBrowserStyleKey               = @"fileBrowserStyle";
NSString* const kUserDefaultsHTMLOutputPlacementKey            = @"htmlOutputPlacement";
NSString* const kUserDefaultsDisableFileBrowserWindowResizeKey = @"disableFileBrowserWindowResize";
NSString* const kUserDefaultsAutoRevealFileKey                 = @"autoRevealFile";
NSString* const kUserDefaultsDisableTabReorderingKey           = @"disableTabReordering";
NSString* const kUserDefaultsDisableTabAutoCloseKey            = @"disableTabAutoClose";
NSString* const kUserDefaultsDisableTabBarCollapsingKey        = @"disableTabBarCollapsing";
NSString* const kUserDefaultsAllowExpandingLinksKey            = @"allowExpandingLinks";
NSString* const kUserDefaultsAllowExpandingPackagesKey         = @"allowExpandingPackages";

// ===========
// = Bundles =
// ===========

// =============
// = Variables =
// =============

NSString* const kUserDefaultsEnvironmentVariablesKey    = @"environmentVariables";

// ============
// = Terminal =
// ============

NSString* const kUserDefaultsMateInstallPathKey         = @"mateInstallPath";
NSString* const kUserDefaultsMateInstallVersionKey      = @"mateInstallVersion";

NSString* const kUserDefaultsDisableRMateServerKey      = @"rmateServerDisabled";
NSString* const kUserDefaultsRMateServerListenKey       = @"rmateServerListen"; // localhost (default), remote
NSString* const kUserDefaultsRMateServerPortKey         = @"rmateServerPort";

NSString* const kRMateServerListenLocalhost             = @"localhost";
NSString* const kRMateServerListenRemote                = @"remote";

// ================
// = Registration =
// ================

NSString* const kUserDefaultsLicenseOwnerKey            = @"licenseOwnerName";

// ==============
// = Appearance =
// ==============

NSString* const kUserDefaultsDisableAntiAliasKey        = @"disableAntiAlias";
NSString* const kUserDefaultsLineNumbersKey             = @"lineNumbers";

// =========
// = Other =
// =========

NSString* const kUserDefaultsFolderSearchFollowLinksKey = @"folderSearchFollowLinks";
NSString* const kUserDefaultsDisableCrashReportingKey   = @"DisableCrashReports";
NSString* const kUserDefaultsCrashReportsContactInfoKey = @"CrashReportsContactInfo";
