#include "content_redirection/redirection.h"
#include "logger.h"
#include <coreinit/debug.h>
#include <coreinit/dynload.h>

static OSDynLoad_Module sModuleHandle = nullptr;

static ContentRedirectionApiErrorType (*sCRAddFSLayerEx)(CRLayerHandle *, const char *, const char *, const char *, FSLayerTypeEx) = nullptr;
static ContentRedirectionApiErrorType (*sCRAddFSLayer)(CRLayerHandle *, const char *, const char *, FSLayerType)                   = nullptr;
static ContentRedirectionApiErrorType (*sCRRemoveFSLayer)(CRLayerHandle)                                                           = nullptr;
static ContentRedirectionApiErrorType (*sCRSetActive)(CRLayerHandle, bool)                                                         = nullptr;
static ContentRedirectionApiErrorType (*sCRGetVersion)(ContentRedirectionVersion *)                                                = nullptr;
static ContentRedirectionApiErrorType (*sCRAddDeviceABI)(const ContentRedirectionDeviceABI *, int *)                               = nullptr;
static ContentRedirectionApiErrorType (*sCRRemoveDeviceABI)(const char *, int *)                                                   = nullptr;

static ContentRedirectionVersion sContentRedirectionVersion = CONTENT_REDIRECTION_MODULE_VERSION_ERROR;

static ContentRedirectionStatus ConvertApiError(ContentRedirectionApiErrorType apiError) {
    switch (apiError) {
        case CONTENT_REDIRECTION_API_ERROR_NONE:
            return CONTENT_REDIRECTION_RESULT_SUCCESS;
        case CONTENT_REDIRECTION_API_ERROR_INVALID_ARG:
            return CONTENT_REDIRECTION_RESULT_INVALID_ARGUMENT;
        case CONTENT_REDIRECTION_API_ERROR_NO_MEMORY:
            return CONTENT_REDIRECTION_RESULT_NO_MEMORY;
        case CONTENT_REDIRECTION_API_ERROR_UNKNOWN_FS_LAYER_TYPE:
            return CONTENT_REDIRECTION_RESULT_UNKNOWN_FS_LAYER_TYPE;
        case CONTENT_REDIRECTION_API_ERROR_LAYER_NOT_FOUND:
            return CONTENT_REDIRECTION_RESULT_LAYER_NOT_FOUND;
        default:
            return CONTENT_REDIRECTION_RESULT_UNKNOWN_ERROR;
    }
}

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
        case CONTENT_REDIRECTION_RESULT_UNSUPPORTED_COMMAND:
            return "CONTENT_REDIRECTION_RESULT_UNSUPPORTED_COMMAND";
    }
    return "CONTENT_REDIRECTION_RESULT_UNKNOWN_ERROR";
}

ContentRedirectionStatus ContentRedirection_InitLibrary() {
    if (OSDynLoad_Acquire("homebrew_content_redirection", &sModuleHandle) != OS_DYNLOAD_OK) {
        DEBUG_FUNCTION_LINE_ERR("OSDynLoad_Acquire failed.");
        return CONTENT_REDIRECTION_RESULT_MODULE_NOT_FOUND;
    }

    if (OSDynLoad_FindExport(sModuleHandle, OS_DYNLOAD_EXPORT_FUNC, "CRGetVersion", (void **) &sCRGetVersion) != OS_DYNLOAD_OK) {
        DEBUG_FUNCTION_LINE_ERR("FindExport CRGetVersion failed.");
        return CONTENT_REDIRECTION_RESULT_MODULE_MISSING_EXPORT;
    }
    auto res = ContentRedirection_GetVersion(&sContentRedirectionVersion);
    if (res != CONTENT_REDIRECTION_RESULT_SUCCESS) {
        return CONTENT_REDIRECTION_RESULT_UNSUPPORTED_VERSION;
    }

    if (OSDynLoad_FindExport(sModuleHandle, OS_DYNLOAD_EXPORT_FUNC, "CRAddFSLayer", (void **) &sCRAddFSLayer) != OS_DYNLOAD_OK) {
        DEBUG_FUNCTION_LINE_ERR("FindExport CRAddFSLayer failed.");
        sCRAddFSLayer = nullptr;
    }
    if (OSDynLoad_FindExport(sModuleHandle, OS_DYNLOAD_EXPORT_FUNC, "CRAddFSLayerEx", (void **) &sCRAddFSLayerEx) != OS_DYNLOAD_OK) {
        DEBUG_FUNCTION_LINE_ERR("FindExport CRAddFSLayerEx failed.");
        sCRAddFSLayerEx = nullptr;
    }

    if (OSDynLoad_FindExport(sModuleHandle, OS_DYNLOAD_EXPORT_FUNC, "CRRemoveFSLayer", (void **) &sCRRemoveFSLayer) != OS_DYNLOAD_OK) {
        DEBUG_FUNCTION_LINE_ERR("FindExport CRRemoveFSLayer failed.");
        sCRRemoveFSLayer = nullptr;
    }

    if (OSDynLoad_FindExport(sModuleHandle, OS_DYNLOAD_EXPORT_FUNC, "CRSetActive", (void **) &sCRSetActive) != OS_DYNLOAD_OK) {
        DEBUG_FUNCTION_LINE_ERR("FindExport CRSetActive failed.");
        sCRSetActive = nullptr;
    }

    if (OSDynLoad_FindExport(sModuleHandle, OS_DYNLOAD_EXPORT_FUNC, "CRAddDeviceABI", (void **) &sCRAddDeviceABI) != OS_DYNLOAD_OK) {
        DEBUG_FUNCTION_LINE_ERR("FindExport CRAddDeviceABI failed.");
        sCRAddDeviceABI = nullptr;
    }

    if (OSDynLoad_FindExport(sModuleHandle, OS_DYNLOAD_EXPORT_FUNC, "CRRemoveDeviceABI", (void **) &sCRRemoveDeviceABI) != OS_DYNLOAD_OK) {
        DEBUG_FUNCTION_LINE_ERR("FindExport CRRemoveDeviceABI failed.");
        sCRRemoveDeviceABI = nullptr;
    }

    return CONTENT_REDIRECTION_RESULT_SUCCESS;
}

ContentRedirectionStatus ContentRedirection_DeInitLibrary() {
    return CONTENT_REDIRECTION_RESULT_SUCCESS;
}

ContentRedirectionStatus ContentRedirection_GetVersion(ContentRedirectionVersion *outVersion) {
    if (sCRGetVersion == nullptr) {
        if (OSDynLoad_Acquire("homebrew_content_redirection", &sModuleHandle) != OS_DYNLOAD_OK) {
            DEBUG_FUNCTION_LINE_WARN("OSDynLoad_Acquire failed.");
            return CONTENT_REDIRECTION_RESULT_MODULE_NOT_FOUND;
        }

        if (OSDynLoad_FindExport(sModuleHandle, OS_DYNLOAD_EXPORT_FUNC, "CRGetVersion", (void **) &sCRGetVersion) != OS_DYNLOAD_OK) {
            DEBUG_FUNCTION_LINE_WARN("FindExport CRGetVersion failed.");
            return CONTENT_REDIRECTION_RESULT_MODULE_MISSING_EXPORT;
        }
    }

    return ConvertApiError(sCRGetVersion(outVersion));
}


ContentRedirectionStatus ContentRedirection_AddFSLayer(CRLayerHandle *handlePtr, const char *layerName, const char *replacementDir, const FSLayerType layerType) {
    if (sContentRedirectionVersion == CONTENT_REDIRECTION_MODULE_VERSION_ERROR) {
        return CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED;
    }
    if (sCRAddFSLayer == nullptr || sContentRedirectionVersion < 1) {
        return CONTENT_REDIRECTION_RESULT_UNSUPPORTED_COMMAND;
    }

    return ConvertApiError(sCRAddFSLayer(handlePtr, layerName, replacementDir, layerType));
}

ContentRedirectionStatus ContentRedirection_AddFSLayerEx(CRLayerHandle *handlePtr, const char *layerName, const char *targetPath, const char *replacementDir, const FSLayerTypeEx layerType) {
    if (sContentRedirectionVersion == CONTENT_REDIRECTION_MODULE_VERSION_ERROR) {
        return CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED;
    }
    if (sCRAddFSLayerEx == nullptr || sContentRedirectionVersion < 2) {
        return CONTENT_REDIRECTION_RESULT_UNSUPPORTED_COMMAND;
    }

    return ConvertApiError(sCRAddFSLayerEx(handlePtr, layerName, targetPath, replacementDir, layerType));
}

ContentRedirectionStatus ContentRedirection_RemoveFSLayer(CRLayerHandle handlePtr) {
    if (sContentRedirectionVersion == CONTENT_REDIRECTION_MODULE_VERSION_ERROR) {
        return CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED;
    }
    if (sCRRemoveFSLayer == nullptr || sContentRedirectionVersion < 1) {
        return CONTENT_REDIRECTION_RESULT_UNSUPPORTED_COMMAND;
    }

    return ConvertApiError(sCRRemoveFSLayer(handlePtr));
}

ContentRedirectionStatus ContentRedirection_SetActive(CRLayerHandle handle, bool active) {
    if (sContentRedirectionVersion == CONTENT_REDIRECTION_MODULE_VERSION_ERROR) {
        return CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED;
    }
    if (sCRSetActive == nullptr || sContentRedirectionVersion < 1) {
        return CONTENT_REDIRECTION_RESULT_UNSUPPORTED_COMMAND;
    }

    return ConvertApiError(sCRSetActive(handle, active));
}

ContentRedirectionStatus ContentRedirection_AddDeviceABI(const ContentRedirectionDeviceABI *device, int *resultOut) {
    if (sContentRedirectionVersion == CONTENT_REDIRECTION_MODULE_VERSION_ERROR) {
        return CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED;
    }
    if (sCRAddDeviceABI == nullptr || sContentRedirectionVersion < 3) {
        return CONTENT_REDIRECTION_RESULT_UNSUPPORTED_COMMAND;
    }

    if (device == nullptr || resultOut == nullptr) {
        return CONTENT_REDIRECTION_RESULT_INVALID_ARGUMENT;
    }

    return ConvertApiError(sCRAddDeviceABI(device, resultOut));
}

ContentRedirectionStatus ContentRedirection_RemoveDeviceABI(const char *device_name, int *resultOut) {
    if (sContentRedirectionVersion == CONTENT_REDIRECTION_MODULE_VERSION_ERROR) {
        return CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED;
    }
    if (sCRRemoveDeviceABI == nullptr || sContentRedirectionVersion < 3) {
        return CONTENT_REDIRECTION_RESULT_UNSUPPORTED_COMMAND;
    }
    if (device_name == nullptr || resultOut == nullptr) {
        return CONTENT_REDIRECTION_RESULT_INVALID_ARGUMENT;
    }

    return ConvertApiError(sCRRemoveDeviceABI(device_name, resultOut));
}