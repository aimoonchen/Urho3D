#include "../Container/Str.h"

static wchar_t* WIN_UTF8ToString(const char* path)
{
    auto sourcePath = Urho3D::WString(Urho3D::String(path));
    auto length = sourcePath.Length();
    auto retval = (wchar_t*)malloc((length + 1) * sizeof(wchar_t));
    memcpy(retval, sourcePath.CString(), length * sizeof(wchar_t));
    retval[length] = '\0';
    return retval;
}

static char* WIN_StringToUTF8(wchar_t* path)
{
    Urho3D::String sourcePath;
    sourcePath.SetUTF8FromWChar(path);
    auto length = sourcePath.Length();
    auto retval = (char*)malloc(length + 1);
    memcpy(retval, sourcePath.CString(), length);
    retval[length] = '\0';
    return retval;
}

#ifdef _WIN32
#include <shlobj.h>
#include <cassert>


char* SDL_GetBasePath(void)
{
    typedef DWORD(WINAPI* GetModuleFileNameExW_t)(HANDLE, HMODULE, LPWSTR, DWORD);
    GetModuleFileNameExW_t pGetModuleFileNameExW;
    DWORD buflen = 128;
    WCHAR* path = NULL;
    auto psapi = LoadLibrary("psapi.dll");
    char* retval = NULL;
    DWORD len = 0;
    int i;

    if (!psapi) {
        //WIN_SetError("Couldn't load psapi.dll");
        return NULL;
    }

    pGetModuleFileNameExW = (GetModuleFileNameExW_t)GetProcAddress(psapi, "GetModuleFileNameExW");
    if (!pGetModuleFileNameExW) {
        //WIN_SetError("Couldn't find GetModuleFileNameExW");
        FreeLibrary(psapi);
        return NULL;
    }

    while (true) {
        void* ptr = realloc(path, buflen * sizeof(WCHAR));
        if (!ptr) {
            free(path);
            FreeLibrary(psapi);
            //SDL_OutOfMemory();
            return NULL;
        }

        path = (WCHAR*)ptr;

        len = pGetModuleFileNameExW(GetCurrentProcess(), NULL, path, buflen);
        if (len != buflen) {
            break;
        }

        /* buffer too small? Try again. */
        buflen *= 2;
    }

    FreeLibrary(psapi);

    if (len == 0) {
        free(path);
        //WIN_SetError("Couldn't locate our .exe");
        return NULL;
    }

    for (i = len - 1; i > 0; i--) {
        if (path[i] == '\\') {
            break;
        }
    }

    assert(i > 0); /* Should have been an absolute path. */
    path[i + 1] = '\0';  /* chop off filename. */
    
    retval = WIN_StringToUTF8(path);

    free(path);

    return retval;
}

char* SDL_GetPrefPath(const char* org, const char* app)
{
    /*
     * Vista and later has a new API for this, but SHGetFolderPath works there,
     *  and apparently just wraps the new API. This is the new way to do it:
     *
     *     SHGetKnownFolderPath(FOLDERID_RoamingAppData, KF_FLAG_CREATE,
     *                          NULL, &wszPath);
     */

    WCHAR path[MAX_PATH];
    char* retval = NULL;
    WCHAR* worg = NULL;
    WCHAR* wapp = NULL;
    size_t new_wpath_len = 0;
    BOOL api_result = FALSE;

    if (!app) {
        //SDL_InvalidParamError("app");
        return NULL;
    }
    if (!org) {
        org = "";
    }

    if (!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, path))) {
        //WIN_SetError("Couldn't locate our prefpath");
        return NULL;
    }

    worg = WIN_UTF8ToString(org);
    if (worg == NULL) {
        //SDL_OutOfMemory();
        return NULL;
    }

    wapp = WIN_UTF8ToString(app);
    if (wapp == NULL) {
        free(worg);
        //SDL_OutOfMemory();
        return NULL;
    }

    new_wpath_len = lstrlenW(worg) + lstrlenW(wapp) + lstrlenW(path) + 3;

    if ((new_wpath_len + 1) > MAX_PATH) {
        free(worg);
        free(wapp);
        //WIN_SetError("Path too long.");
        return NULL;
    }

    if (*worg) {
        lstrcatW(path, L"\\");
        lstrcatW(path, worg);
    }
    free(worg);

    api_result = CreateDirectoryW(path, NULL);
    if (api_result == FALSE) {
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            free(wapp);
            //WIN_SetError("Couldn't create a prefpath.");
            return NULL;
        }
    }

    lstrcatW(path, L"\\");
    lstrcatW(path, wapp);
    free(wapp);

    api_result = CreateDirectoryW(path, NULL);
    if (api_result == FALSE) {
        if (GetLastError() != ERROR_ALREADY_EXISTS) {
            //WIN_SetError("Couldn't create a prefpath.");
            return NULL;
        }
    }

    lstrcatW(path, L"\\");

    retval = WIN_StringToUTF8(path);

    return retval;
}

#elif defined(__ANDROID__)
#include <unistd.h>

#include "SDL_error.h"
#include "SDL_filesystem.h"
#include "SDL_system.h"

char* SDL_GetBasePath(void)
{
    /* The current working directory is / on Android */
    SDL_Unsupported();
    return NULL;
}

char* SDL_GetPrefPath(const char* org, const char* app)
{
    const char* path = SDL_AndroidGetInternalStoragePath();
    if (path)
    {
        size_t pathlen = SDL_strlen(path) + 2;
        char* fullpath = (char*)SDL_malloc(pathlen);
        if (!fullpath)
        {
            SDL_OutOfMemory();
            return NULL;
        }
        SDL_snprintf(fullpath, pathlen, "%s/", path);
        return fullpath;
    }
    return NULL;
}
#elif defined(__EMSCRIPTEN__)
#include <errno.h>
#include <sys/stat.h>

#include "SDL_error.h"
#include "SDL_filesystem.h"

#include <emscripten/emscripten.h>

char* SDL_GetBasePath(void)
{
    char* retval = "/";
    return SDL_strdup(retval);
}

char* SDL_GetPrefPath(const char* org, const char* app)
{
    const char* append = "/libsdl/";
    char* retval;
    size_t len = 0;

    if (!app)
    {
        SDL_InvalidParamError("app");
        return NULL;
    }
    if (!org)
    {
        org = "";
    }

    len = SDL_strlen(append) + SDL_strlen(org) + SDL_strlen(app) + 3;
    retval = (char*)SDL_malloc(len);
    if (!retval)
    {
        SDL_OutOfMemory();
        return NULL;
    }

    if (*org)
    {
        SDL_snprintf(retval, len, "%s%s/%s/", append, org, app);
    }
    else
    {
        SDL_snprintf(retval, len, "%s%s/", append, app);
    }

    if (mkdir(retval, 0700) != 0 && errno != EEXIST)
    {
        SDL_SetError("Couldn't create directory '%s': '%s'", retval, strerror(errno));
        SDL_free(retval);
        return NULL;
    }

    return retval;
}
#endif