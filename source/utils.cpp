#include "content_redirection/redirection.h"
#include <coreinit/debug.h>
#include <coreinit/dynload.h>
#include <sys/iosupport.h>

static OSDynLoad_Module sModuleHandle = nullptr;

static ContentRedirectionApiErrorType (*sCRAddFSLayer)(CRLayerHandle *, const char *, const char *, FSLayerType) = nullptr;
static ContentRedirectionApiErrorType (*sCRRemoveFSLayer)(CRLayerHandle)                                         = nullptr;
static ContentRedirectionApiErrorType (*sCRSetActive)(CRLayerHandle)                                             = nullptr;
static ContentRedirectionApiErrorType (*sCRGetVersion)(ContentRedirectionVersion *)                              = nullptr;
static int (*sCRAddDevice)(const devoptab_t *, int *)                                                            = nullptr;
static int (*sCRRemoveDevice)(const char *)                                                                      = nullptr;

static ContentRedirectionVersion sContentRedirectionVersion = CONTENT_REDIRECTION_MODULE_VERSION_ERROR;

const char *ContentRedirection_GetStatusStr(ContentRedirectionStatus status) {
    switch (status) {
        case CONTENT_REDIRECTION_RESULT_SUCCESS:
            return "CONTENT_REDIRECTION_RESULT_SUCCESS";
        case CONTENT_REDIRECTION_RESULT_MODULE_NOT_FOUND:
            return "CONTENT_REDIRECTION_RESULT_MODULE_NOT_FOUND";
        case CONTENT_REDIRECTION_RESULT_MODULE_MISSING_EXPORT:
            return "CONTENT_REDIRECTION_RESULT_MODULE_MISSING_EXPORT";
        case CONTENT_REDIRECTION_RESULT_UNSUPPORTED_VERSION:
            return "CONTENT_REDIRECTION_RESULT_UNSUPPORTED_VERSION";
        case CONTENT_REDIRECTION_RESULT_INVALID_ARGUMENT:
            return "CONTENT_REDIRECTION_RESULT_INVALID_ARGUMENT";
        case CONTENT_REDIRECTION_RESULT_NO_MEMORY:
            return "CONTENT_REDIRECTION_RESULT_NO_MEMORY";
        case CONTENT_REDIRECTION_RESULT_UNKNOWN_FS_LAYER_TYPE:
            return "CONTENT_REDIRECTION_RESULT_UNKNOWN_FS_LAYER_TYPE";
        case CONTENT_REDIRECTION_RESULT_LAYER_NOT_FOUND:
            return "CONTENT_REDIRECTION_RESULT_LAYER_NOT_FOUND";
        case CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED:
            return "CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED";
        case CONTENT_REDIRECTION_RESULT_UNKNOWN_ERROR:
            return "CONTENT_REDIRECTION_RESULT_UNKNOWN_ERROR";
    }
    return "CONTENT_REDIRECTION_RESULT_UNKNOWN_ERROR";
}

ContentRedirectionStatus ContentRedirection_InitLibrary() {
    if (OSDynLoad_Acquire("homebrew_content_redirection", &sModuleHandle) != OS_DYNLOAD_OK) {
        OSReport("ContentRedirection_Init: OSDynLoad_Acquire failed.\n");
        return CONTENT_REDIRECTION_RESULT_MODULE_NOT_FOUND;
    }

    if (OSDynLoad_FindExport(sModuleHandle, FALSE, "CRGetVersion", (void **) &sCRGetVersion) != OS_DYNLOAD_OK) {
        OSReport("ContentRedirection_Init: CRGetVersion failed.\n");
        return CONTENT_REDIRECTION_RESULT_MODULE_MISSING_EXPORT;
    }
    auto res = ContentRedirection_GetVersion(&sContentRedirectionVersion);
    if (res != CONTENT_REDIRECTION_RESULT_SUCCESS) {
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

ContentRedirectionStatus ContentRedirection_DeInitLibrary() {
    return CONTENT_REDIRECTION_RESULT_SUCCESS;
}

ContentRedirectionApiErrorType GetVersion(ContentRedirectionVersion *);
ContentRedirectionStatus ContentRedirection_GetVersion(ContentRedirectionVersion *outVariable) {
    if (sCRGetVersion == nullptr) {
        return CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED;
    }

    auto res = reinterpret_cast<decltype(&GetVersion)>(sCRGetVersion)(outVariable);
    if (res == CONTENT_REDIRECTION_API_ERROR_NONE) {
        return CONTENT_REDIRECTION_RESULT_SUCCESS;
    }
    return res == CONTENT_REDIRECTION_API_ERROR_INVALID_ARG ? CONTENT_REDIRECTION_RESULT_INVALID_ARGUMENT : CONTENT_REDIRECTION_RESULT_UNKNOWN_ERROR;
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
            return CONTENT_REDIRECTION_RESULT_INVALID_ARGUMENT;
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
        return CONTENT_REDIRECTION_RESULT_INVALID_ARGUMENT;
    }

    *resultOut = reinterpret_cast<decltype(&AddDevice)>(sCRAddDevice)(device);
    return CONTENT_REDIRECTION_RESULT_SUCCESS;
}

ContentRedirectionStatus ContentRedirection_RemoveDevice(const char *name, int *resultOut) {
    if (sCRAddFSLayer == nullptr) {
        return CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED;
    }
    if (resultOut == nullptr) {
        return CONTENT_REDIRECTION_RESULT_INVALID_ARGUMENT;
    }
    *resultOut = reinterpret_cast<decltype(&RemoveDevice)>(sCRRemoveDevice)(name);
    return CONTENT_REDIRECTION_RESULT_SUCCESS;
}
