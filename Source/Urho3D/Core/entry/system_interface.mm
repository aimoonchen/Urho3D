//
//  system_interface.m
//  Urho3D
//
//  Created by simonchen on 2021/5/22.
//

#import <Foundation/Foundation.h>
#include <sys/stat.h>
#include <sys/types.h>

// Urho3D: added variables
char* resource_dir = 0;
char* documents_dir = 0;

// Urho3D: added function
void SDL_IOS_LogMessage(const char *message)
{
#ifdef _DEBUG
    NSLog(@"%@", [NSString stringWithUTF8String: message]);
#endif
}

// Urho3D: added function
const char* SDL_IOS_GetResourceDir()
{
    if (!resource_dir)
    {
        const char *temp = [[[NSBundle mainBundle] resourcePath] UTF8String];
        resource_dir = (char*)malloc(strlen(temp) + 1);
        strcpy(resource_dir, temp);
    }

    return resource_dir;
}

// Urho3D: added function
const char* SDL_IOS_GetDocumentsDir()
{
    if (!documents_dir)
    {
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *basePath = ([paths count] > 0) ? [paths objectAtIndex:0] : nil;

        const char *temp = [basePath UTF8String];
        documents_dir = (char*)malloc(strlen(temp) + 1);
        strcpy(documents_dir, temp);
    }

    return documents_dir;
}

// Urho3D: added function
#if TARGET_OS_TV
unsigned SDL_TVOS_GetActiveProcessorCount()
{
    return [NSProcessInfo class] ? (unsigned)[[NSProcessInfo processInfo] activeProcessorCount] : 1;
}
#endif
char *
SDL_GetBasePath(void)
{ @autoreleasepool
{
    NSBundle *bundle = [NSBundle mainBundle];
    const char* baseType = [[[bundle infoDictionary] objectForKey:@"SDL_FILESYSTEM_BASE_DIR_TYPE"] UTF8String];
    const char *base = NULL;
    char *retval = NULL;

    if (baseType == NULL) {
        baseType = "resource";
    }
    if (strcasecmp(baseType, "bundle")==0) {
        base = [[bundle bundlePath] fileSystemRepresentation];
    } else if (strcasecmp(baseType, "parent")==0) {
        base = [[[bundle bundlePath] stringByDeletingLastPathComponent] fileSystemRepresentation];
    } else {
        /* this returns the exedir for non-bundled  and the resourceDir for bundled apps */
        base = [[bundle resourcePath] fileSystemRepresentation];
    }

    if (base) {
        const size_t len = strlen(base) + 2;
        retval = (char *) malloc(len);
        if (retval == NULL) {
            //SDL_OutOfMemory();
            SDL_IOS_LogMessage("out of memory");
        } else {
            snprintf(retval, len, "%s/", base);
        }
    }

    return retval;
}}

char *
SDL_GetPrefPath(const char *org, const char *app)
{ @autoreleasepool
{
    if (!app) {
        //SDL_InvalidParamError("app");
        SDL_IOS_LogMessage("Parameter is invalid : app");
        return NULL;
    }
    if (!org) {
        org = "";
    }

    char *retval = NULL;
#if !TARGET_OS_TV
    NSArray *array = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
#else
    /* tvOS does not have persistent local storage!
     * The only place on-device where we can store data is
     * a cache directory that the OS can empty at any time.
     *
     * It's therefore very likely that save data will be erased
     * between sessions. If you want your app's save data to
     * actually stick around, you'll need to use iCloud storage.
     */

    static SDL_bool shown = SDL_FALSE;
    if (!shown)
    {
        shown = SDL_TRUE;
        SDL_LogCritical(SDL_LOG_CATEGORY_SYSTEM, "tvOS does not have persistent local storage! Use iCloud storage if you want your data to persist between sessions.\n");
    }

    NSArray *array = NSSearchPathForDirectoriesInDomains(NSCachesDirectory, NSUserDomainMask, YES);
#endif /* !TARGET_OS_TV */

    if ([array count] > 0) {  /* we only want the first item in the list. */
        NSString *str = [array objectAtIndex:0];
        const char *base = [str fileSystemRepresentation];
        if (base) {
            const size_t len = strlen(base) + strlen(org) + strlen(app) + 4;
            retval = (char *) malloc(len);
            if (retval == NULL) {
                //SDL_OutOfMemory();
                SDL_IOS_LogMessage("out of memory");
            } else {
                char *ptr;
                if (*org) {
                    snprintf(retval, len, "%s/%s/%s/", base, org, app);
                } else {
                    snprintf(retval, len, "%s/%s/", base, app);
                }
                for (ptr = retval+1; *ptr; ptr++) {
                    if (*ptr == '/') {
                        *ptr = '\0';
                        mkdir(retval, 0700);
                        *ptr = '/';
                    }
                }
                mkdir(retval, 0700);
            }
        }
    }

    return retval;
}}
