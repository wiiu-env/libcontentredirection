#include "content_redirection/redirection.h"
#include <coreinit/debug.h>
#include <coreinit/dynload.h>
#include <sys/iosupport.h>

static OSDynLoad_Module sModuleHandle = nullptr;

static ContentRedirectionStatus (*sCRAddFSLayer)(CRLayerHandle *, const char *, const char *, FSLayerType) = nullptr;
static ContentRedirectionStatus (*sCRRemoveFSLayer)(CRLayerHandle)                                         = nullptr;
static ContentRedirectionStatus (*sCRSetActive)(CRLayerHandle)                                             = nullptr;
static ContentRedirectionVersion (*sCRGetVersion)()                                                        = nullptr;
static ContentRedirectionStatus (*sCRAddDevice)(const devoptab_t *, int *)                                 = nullptr;
static ContentRedirectionStatus (*sCRRemoveDevice)(const char *)                                           = nullptr;

ContentRedirectionStatus ContentRedirection_Init() {
    if (OSDynLoad_Acquire("homebrew_content_redirection", &sModuleHandle) != OS_DYNLOAD_OK) {
        OSReport("ContentRedirection_Init: OSDynLoad_Acquire failed.\n");
        return CONTENT_REDIRECTION_RESULT_MODULE_NOT_FOUND;
    }

    if (OSDynLoad_FindExport(sModuleHandle, FALSE, "CRGetVersion", (void **) &sCRGetVersion) != OS_DYNLOAD_OK) {
        OSReport("ContentRedirection_Init: CRGetVersion failed.\n");
        return CONTENT_REDIRECTION_RESULT_MODULE_MISSING_EXPORT;
    }
    auto res = ContentRedirection_GetVersion();
    if (res != CONTENT_REDIRECT_MODULE_VERSION) {
        return CONTENT_REDIRECTION_RESULT_UNSUPPORTED_VERSION;
    }

    if (OSDynLoad_FindExport(sModuleHandle, FALSE, "CRAddFSLayer", (void **) &sCRAddFSLayer) != OS_DYNLOAD_OK) {
        OSReport("ContentRedirection_Init: CRAddFSLayer failed.\n");
        return CONTENT_REDIRECTION_RESULT_MODULE_MISSING_EXPORT;
    }

    if (OSDynLoad_FindExport(sModuleHandle, FALSE, "CRRemoveFSLayer", (void **) &sCRRemoveFSLayer) != OS_DYNLOAD_OK) {
        OSReport("ContentRedirection_Init: CRRemoveFSLayer failed.\n");
        return CONTENT_REDIRECTION_RESULT_MODULE_MISSING_EXPORT;
    }

    if (OSDynLoad_FindExport(sModuleHandle, FALSE, "CRSetActive", (void **) &sCRSetActive) != OS_DYNLOAD_OK) {
        OSReport("ContentRedirection_Init: CRSetActive failed.\n");
        return CONTENT_REDIRECTION_RESULT_MODULE_MISSING_EXPORT;
    }

    if (OSDynLoad_FindExport(sModuleHandle, FALSE, "CRAddDevice", (void **) &sCRAddDevice) != OS_DYNLOAD_OK) {
        OSReport("ContentRedirection_Init: CRAddDevice failed.\n");
        return CONTENT_REDIRECTION_RESULT_MODULE_MISSING_EXPORT;
    }

    if (OSDynLoad_FindExport(sModuleHandle, FALSE, "CRRemoveDevice", (void **) &sCRRemoveDevice) != OS_DYNLOAD_OK) {
        OSReport("ContentRedirection_Init: CRRemoveDevice failed.\n");
        return CONTENT_REDIRECTION_RESULT_MODULE_MISSING_EXPORT;
    }

    return CONTENT_REDIRECTION_RESULT_SUCCESS;
}

ContentRedirectionVersion GetVersion();
ContentRedirectionVersion ContentRedirection_GetVersion() {
    if (sCRGetVersion == nullptr) {
        return CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED;
    }

    return reinterpret_cast<decltype(&GetVersion)>(sCRGetVersion)();
}


ContentRedirectionApiErrorType AddFSLayer(CRLayerHandle *, const char *, const char *, FSLayerType);
ContentRedirectionStatus ContentRedirection_AddFSLayer(CRLayerHandle *handlePtr, const char *layerName, const char *replacementDir, FSLayerType layerType) {
    if (sCRAddFSLayer == nullptr) {
        return CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED;
    }
    auto res = reinterpret_cast<decltype(&AddFSLayer)>(sCRAddFSLayer)(handlePtr, layerName, replacementDir, layerType);
    if (res == CONTENT_REDIRECTION_API_ERROR_NONE) {
        return CONTENT_REDIRECTION_RESULT_SUCCESS;
    }
    switch (res) {
        case CONTENT_REDIRECTION_API_ERROR_INVALID_ARG:
            return CONTENT_REDIRECTION_RESULT_INVALID_ARG;
        case CONTENT_REDIRECTION_API_ERROR_NO_MEMORY:
            return CONTENT_REDIRECTION_RESULT_NO_MEMORY;
        case CONTENT_REDIRECTION_API_ERROR_UNKNOWN_FS_LAYER_TYPE:
            return CONTENT_REDIRECTION_RESULT_UNKNOWN_FS_LAYER_TYPE;
        default:
            return CONTENT_REDIRECTION_RESULT_UNKNOWN_ERROR;
    }
}

ContentRedirectionApiErrorType RemoveFSLayer(CRLayerHandle);
ContentRedirectionStatus ContentRedirection_RemoveFSLayer(CRLayerHandle handlePtr) {
    if (sCRAddFSLayer == nullptr) {
        return CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED;
    }
    auto res = reinterpret_cast<decltype(&RemoveFSLayer)>(sCRRemoveFSLayer)(handlePtr);
    if (res == CONTENT_REDIRECTION_API_ERROR_NONE) {
        return CONTENT_REDIRECTION_RESULT_SUCCESS;
    }

    if (res == CONTENT_REDIRECTION_API_ERROR_LAYER_NOT_FOUND) {
        return CONTENT_REDIRECTION_RESULT_LAYER_NOT_FOUND;
    }

    return CONTENT_REDIRECTION_RESULT_UNKNOWN_ERROR;
}

ContentRedirectionApiErrorType SetActive(CRLayerHandle, bool);
ContentRedirectionStatus ContentRedirection_SetActive(CRLayerHandle handle, bool active) {
    if (sCRAddFSLayer == nullptr) {
        return CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED;
    }
    auto res = reinterpret_cast<decltype(&SetActive)>(sCRSetActive)(handle, active);
    if (res == CONTENT_REDIRECTION_API_ERROR_NONE) {
        return CONTENT_REDIRECTION_RESULT_SUCCESS;
    }

    if (res == CONTENT_REDIRECTION_API_ERROR_LAYER_NOT_FOUND) {
        return CONTENT_REDIRECTION_RESULT_LAYER_NOT_FOUND;
    }

    return CONTENT_REDIRECTION_RESULT_UNKNOWN_ERROR;
}

ContentRedirectionStatus ContentRedirection_AddDevice(const devoptab_t *device, int *resultOut) {
    if (sCRAddFSLayer == nullptr) {
        return CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED;
    }

    if (resultOut == nullptr) {
        return CONTENT_REDIRECTION_RESULT_INVALID_ARG;
    }

    *resultOut = reinterpret_cast<decltype(&AddDevice)>(sCRAddDevice)(device);
    return CONTENT_REDIRECTION_RESULT_SUCCESS;
}

ContentRedirectionStatus ContentRedirection_RemoveDevice(const char *name, int *resultOut) {
    if (sCRAddFSLayer == nullptr) {
        return CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED;
    }
    if (resultOut == nullptr) {
        return CONTENT_REDIRECTION_RESULT_INVALID_ARG;
    }
    *resultOut = reinterpret_cast<decltype(&RemoveDevice)>(sCRRemoveDevice)(name);
    return CONTENT_REDIRECTION_RESULT_SUCCESS;
}
