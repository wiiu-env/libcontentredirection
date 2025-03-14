#pragma once

#include <stdint.h>
#include <sys/iosupport.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum FSLayerType {
    /* Redirects the /vol/content to a given path.
     * Existing files in /vol/content will be ignored, only files of the layer (provided via the replacementDir) will be used.
     */
    FS_LAYER_TYPE_CONTENT_REPLACE,

    /* Redirects the /vol/content to a given path.
     * Merges the files of the layer (provided via the replacementDir) into the existing /vol/content (which is used as fallback).
     * All files which start with ".deleted_" will be ignored.
     *
     * If a file exists in both the layer and /vol/content, the layer has priority and will be used.
     * If a file doesn't exist in the layer but in /vol/content, the file from /vol/content will be used.
     * If a file only exists in the layer and not in /vol/content, the file from the layer will be used.
     *
     * To "hide" a file which exists in /vol/content you need to create an empty dummy file with the prefix ".deleted_" in the same directory for the layer..
     * e.g. When the OS requests "/vol/content/music/track1.wav" (which exists) and the layer has a file "[replacementDir]/music/.deleted_track1.wav", FS_STATUS_NOT_FOUND will be returned.
     *
     * If multiple layers are used, the "parent layer" will act like /vol/content and is used as a fallback.
     */
    FS_LAYER_TYPE_CONTENT_MERGE,

    /* Redirects the /vol/save to a given path.
     * Existing files in /vol/save will be ignored, only files in the layer (provided via the replacementDir) will be used.
     */
    FS_LAYER_TYPE_SAVE_REPLACE,

    /*
     * Same as FS_LAYER_TYPE_CONTENT_REPLACE, but for /vol/aoc
     */
    FS_LAYER_TYPE_AOC_REPLACE,

    /*
     * Same as FS_LAYER_TYPE_CONTENT_MERGE, but for /vol/aoc
     */
    FS_LAYER_TYPE_AOC_MERGE,

    /* Redirects the /vol/save/8XXXXXXX to a given path for the current user
     * Existing files in /vol/save/8XXXXXXX will be ignored, only files in the layer (provided via the replacementDir) will be used.
     */
    FS_LAYER_TYPE_SAVE_REPLACE_FOR_CURRENT_USER,
} FSLayerType;

typedef enum FSLayerTypeEx {
    FS_LAYER_TYPE_EX_REPLACE_DIRECTORY,
    FS_LAYER_TYPE_EX_MERGE_DIRECTORY,
    FS_LAYER_TYPE_EX_REPLACE_FILE,
} FSLayerTypeEx;

typedef enum ContentRedirectionStatus {
    CONTENT_REDIRECTION_RESULT_SUCCESS               = 0,
    CONTENT_REDIRECTION_RESULT_MODULE_NOT_FOUND      = -0x1,
    CONTENT_REDIRECTION_RESULT_MODULE_MISSING_EXPORT = -0x2,
    CONTENT_REDIRECTION_RESULT_UNSUPPORTED_VERSION   = -0x3,
    CONTENT_REDIRECTION_RESULT_INVALID_ARGUMENT      = -0x10,
    CONTENT_REDIRECTION_RESULT_NO_MEMORY             = -0x11,
    CONTENT_REDIRECTION_RESULT_UNKNOWN_FS_LAYER_TYPE = -0x12,
    CONTENT_REDIRECTION_RESULT_LAYER_NOT_FOUND       = -0x13,
    CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED     = -0x20,
    CONTENT_REDIRECTION_RESULT_UNSUPPORTED_COMMAND   = -0x21,
    CONTENT_REDIRECTION_RESULT_UNKNOWN_ERROR         = -0x1000,
} ContentRedirectionStatus;

typedef uint32_t CRLayerHandle;
typedef uint32_t ContentRedirectionVersion;

#define CONTENT_REDIRECTION_MODULE_VERSION_ERROR 0xFFFFFFFF

typedef enum ContentRedirectionApiErrorType {
    CONTENT_REDIRECTION_API_ERROR_NONE                  = 0,
    CONTENT_REDIRECTION_API_ERROR_INVALID_ARG           = -1,
    CONTENT_REDIRECTION_API_ERROR_NO_MEMORY             = -2,
    CONTENT_REDIRECTION_API_ERROR_UNKNOWN_FS_LAYER_TYPE = -3,
    CONTENT_REDIRECTION_API_ERROR_LAYER_NOT_FOUND       = -4,
} ContentRedirectionApiErrorType;

const char *ContentRedirection_GetStatusStr(ContentRedirectionStatus status);

/**
 * This function has to be called before any other function of this lib (except ContentRedirection_GetVersion) can be used.
 *
 * @return  CONTENT_REDIRECTION_RESULT_SUCCESS:                 The library has been initialized successfully. Other functions can now be used.
 *          CONTENT_REDIRECTION_RESULT_MODULE_NOT_FOUND:        The module could not be found. Make sure the module is loaded.
 *          CONTENT_REDIRECTION_RESULT_MODULE_MISSING_EXPORT:   The module is missing an expected export.
 *          CONTENT_REDIRECTION_RESULT_UNSUPPORTED_VERSION:     The version of the loaded module is not compatible with this version of the lib.
 */
ContentRedirectionStatus ContentRedirection_InitLibrary();

ContentRedirectionStatus ContentRedirection_DeInitLibrary();

/**
 * Retrieves the API Version of the loaded ContentRedirectionModule.<br>
 * <br>
 * @param outVersion pointer to the variable where the version will be stored.
 *
 * @return CONTENT_REDIRECTION_RESULT_SUCCESS:                  The API version has been store in the version ptr.<br>
 *         CONTENT_REDIRECTION_RESULT_MODULE_NOT_FOUND:         The module could not be found. Make sure the module is loaded.<br>
 *         CONTENT_REDIRECTION_RESULT_MODULE_MISSING_EXPORT:    The module is missing an expected export.<br>
 *         CONTENT_REDIRECTION_RESULT_INVALID_ARGUMENT:         Invalid version pointer.<br>
 *         CONTENT_REDIRECTION_RESULT_UNKNOWN_ERROR:            Retrieving the module version failed.
 */
ContentRedirectionStatus ContentRedirection_GetVersion(ContentRedirectionVersion *outVersion);

/**
 * Adds a FSLayers that redirects the /vol/content or /vol/save fs calls for the Game/Wii U Menu process.  <br>
 * Make sure to remove all added the layers before the application ends.  <br>
 * The replacement dir has to be valid in the ContentRedirection Module, use "ContentRedirection_AddDevice" to add a Device for the ContentRedirection Module.  <br>
 * Multiple layers can be added. Each layer is valid system wide for the Game/Wii U Menu process.  <br>
 * The layers will be processed in reverse adding order. e.g. when you add Layer1, Layer2 and then Layer3; Layer3, Layer2 and finally Layer1 will be processed.  <br>
 * An added layer is active by default.
 *
 * @param handlePtr         The handle of the layer is written to this pointer.
 * @param layerName         Name of the layer, used for debugging.
 * @param replacementDir    Path to the directory that will replace / merge into the original one.
 * @param layerType         Type of the layer, see FSLayerType for more information.  <br>
 *                          If set to false, errors of this layer will be returned to the OS.
* @return CONTENT_REDIRECTION_RESULT_SUCCESS:               The layer had been added successfully. <br>
 *                                                          The layer has to be removed before the currently running application ends. <br>
*         CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED:     "ContentRedirection_InitLibrary()" was not called. <br>
*         CONTENT_REDIRECTION_RESULT_INVALID_ARGUMENT:      "handlePtr", "layerName" or "replacementDir" is NULL <br>
*         CONTENT_REDIRECTION_API_ERROR_NO_MEMORY:          Not enough memory to create this layer. <br>
*         CONTENT_REDIRECTION_API_ERROR_UNKNOWN_LAYER_TYPE: Unknown/invalid LayerType. See FSLayerType for all supported layers. <br>
*         CONTENT_REDIRECTION_RESULT_UNKNOWN_ERROR:         Unknown error.
 */
ContentRedirectionStatus ContentRedirection_AddFSLayer(CRLayerHandle *handlePtr, const char *layerName, const char *replacementDir, FSLayerTypeEx layerType);

/**
 * Adds a FSLayer that redirects a files or directories fs calls for the Game/Wii U Menu process.  <br>
 * Make sure to remove all added the layers before the application ends.  <br>
 * The replacement dir has to be valid in the ContentRedirection Module, use "ContentRedirection_AddDevice" to add a Device for the ContentRedirection Module.  <br>
 * Multiple layers can be added. Each layer is valid system mwide for the Game/Wii U Menu process.  <br>
 * The layers will be processed in reverse adding order. e.g. when you add Layer1, Layer2 and then Layer3; Layer3, Layer2 and finally Layer1 will be processed.  <br>
 * An added layer is active by default.
 *
 * **Requires API version 2 or higher**
 *
 * @param handlePtr         The handle of the layer is written to this pointer.
 * @param layerName         Name of the layer, used for debugging.
 * @param targetPath        Path to the directory/file that should be replaced or merged.
 * @param replacementPath   Path to the directory/file that will replace / merge into the original one.
 * @param layerType         Type of the layer, see FSLayerType for more information.  <br>
 *                          If set to false, errors of this layer will be returned to the OS.
* @return CONTENT_REDIRECTION_RESULT_SUCCESS:               The layer had been added successfully. <br>
 *                                                          The layer has to be removed before the currently running application ends. <br>
*         CONTENT_REDIRECTION_RESULT_UNSUPPORTED_COMMAND:   This function requires API version 2 <br>
*         CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED:     "ContentRedirection_InitLibrary()" was not called. <br>
*         CONTENT_REDIRECTION_RESULT_INVALID_ARGUMENT:      "handlePtr", "layerName" or "replacementDir" is NULL <br>
*         CONTENT_REDIRECTION_API_ERROR_NO_MEMORY:          Not enough memory to create this layer. <br>
*         CONTENT_REDIRECTION_API_ERROR_UNKNOWN_LAYER_TYPE: Unknown/invalid LayerType. See FSLayerType for all supported layers. <br>
*         CONTENT_REDIRECTION_RESULT_UNKNOWN_ERROR:         Unknown error.
 */
ContentRedirectionStatus ContentRedirection_AddFSLayerEx(CRLayerHandle *handlePtr, const char *layerName, const char *targetPath, const char *replacementPath, FSLayerTypeEx layerType);

/**
 * Removes a previously added FS Layer.
 * @param handle    handle of the layer that will be removed
 * @return
 */
ContentRedirectionStatus ContentRedirection_RemoveFSLayer(CRLayerHandle handle);

/**
 * Set the "active" flag for a given FSLayer. <br>
 *
 * @param handle Handle of the FSLayer.
 * @param active New "active"-state of the layer
 * @return CONTENT_REDIRECTION_RESULT_SUCCESS:              The active state has been set successfully. <br>
 *         CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED:    "ContentRedirection_InitLibrary()" was not called. <br>
 *         CONTENT_REDIRECTION_RESULT_UNSUPPORTED_COMMAND:  This command is not supported by the currently loaded Module. <br>
 *         CONTENT_REDIRECTION_RESULT_LAYER_NOT_FOUND:      Invalid FSLayer handle. <br>
 *         CONTENT_REDIRECTION_RESULT_UNKNOWN_ERROR:        Unknown error.
 */
ContentRedirectionStatus ContentRedirection_SetActive(CRLayerHandle handle, bool active);

/**
 * Calls "AddDevice" for the ContentRedirection Module. <br>
 * When a device is added for the ContentRedirection Module, it can be used in FSLayers. <br>
 *
 * @param device        Device that will be added
 * @param resultOut     Will hold the result of the "AddDevice" call.
 * @return CONTENT_REDIRECTION_RESULT_SUCCESS:              AddDevice has been called, result is written to resultOut. <br>
 *                                                          See documentation of AddDevice for more information <br>
 *         CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED:    "ContentRedirection_InitLibrary()" was not called. <br>
 *         CONTENT_REDIRECTION_RESULT_UNSUPPORTED_COMMAND:  This command is not supported by the currently loaded Module. <br>
 *         CONTENT_REDIRECTION_RESULT_INVALID_ARG:          resultOut is NULL. <br>
 *         CONTENT_REDIRECTION_RESULT_UNKNOWN_ERROR:        Unknown error. <br>
 */
ContentRedirectionStatus ContentRedirection_AddDevice(const devoptab_t *device, int *resultOut);

/**
 * Calls "RemoveDevice" for the ContentRedirection Module.
 *
 * @param name      name of the device that will be added. e.g. "romfs:"
 * @param resultOut Will hold the result of the "AddDevice" call.
 * @return  CONTENT_REDIRECTION_RESULT_SUCCESS:           RemoveDevice has been called, result is written to resultOut. <br>
 *                                                        See documentation of RemoveDevice for more information <br>
 *          CONTENT_REDIRECTION_RESULT_LIB_UNINITIALIZED: "ContentRedirection_InitLibrary()" was not called. <br>
 *         CONTENT_REDIRECTION_RESULT_UNSUPPORTED_COMMAND:  This command is not supported by the currently loaded Module. <br>
 *          CONTENT_REDIRECTION_RESULT_INVALID_ARG:       resultOut is NULL. <br>
 *          CONTENT_REDIRECTION_RESULT_UNKNOWN_ERROR:     Unknown error.
 */
ContentRedirectionStatus ContentRedirection_RemoveDevice(const char *name, int *resultOut);

#ifdef __cplusplus
} // extern "C"
#endif