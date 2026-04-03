#pragma once

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CONTENT_REDIRECTION_DEVICE_MAGIC   0x43524456 // "CRDV"
#define CONTENT_REDIRECTION_DEVICE_VERSION 1

typedef struct {
    uint32_t dev;
    uint32_t ino;
    uint32_t mode;
    uint32_t nlink;
    uint32_t uid;
    uint32_t gid;
    uint32_t rdev;
    int64_t size;
    int64_t atime;
    int64_t mtime;
    int64_t ctime;
    int64_t blksize;
    int64_t blocks;
} CR_Stat;

typedef struct {
    uint64_t bsize;
    uint64_t frsize;
    uint64_t blocks;
    uint64_t bfree;
    uint64_t bavail;
    uint64_t files;
    uint64_t ffree;
    uint64_t favail;
    uint64_t fsid;
    uint64_t flag;
    uint64_t namemax;
} CR_Statvfs;

typedef struct {
    int64_t tv_sec;
    int64_t tv_usec;
} CR_Timeval;

/**
 * @brief ABI-safe representation of a devoptab_t device.
 * * This structure bridges native devoptab implementations across the
 * module boundary. It removes newlib-specific types (like struct _reent
 * and native struct stat) in favor of standard-sized equivalents.
 *
 * * @note ERROR HANDLING: On failure, every function must return the
 * NEGATIVE standard errno value (e.g., `return -ENOENT;`).
 */
typedef struct ContentRedirectionDeviceABI {
    uint32_t magic;   /**< Identifier magic number (CONTENT_REDIRECTION_DEVICE_MAGIC) */
    uint32_t version; /**< ABI version (CONTENT_REDIRECTION_DEVICE_VERSION) */
    const char *name; /**< Name of the registered device (e.g., "romfs") */
    int structSize;   /**< Size of the internal file struct */
    int dirStateSize; /**< Size of the internal dir struct */
    void *deviceData; /**< Context data passed into each function call */

    // --- File Operations ---
    /**
     * @brief Opens a file.
     * @return 0 or a positive identifier on success, negative errno on failure.
     */
    int (*open)(void *deviceData, void *fileStruct, const char *path, int flags, uint32_t mode);

    /**
     * @brief Closes an open file.
     * @return 0 on success, negative errno on failure.
     */
    int (*close)(void *deviceData, void *fd);

    /**
     * @brief Writes data to an open file.
     * @return Number of bytes written on success, negative errno on failure.
     */
    ssize_t (*write)(void *deviceData, void *fd, const char *ptr, size_t len);

    /**
     * @brief Reads data from an open file.
     * @return Number of bytes read on success, 0 on EOF, negative errno on failure.
     */
    ssize_t (*read)(void *deviceData, void *fd, char *ptr, size_t len);

    /**
     * @brief Repositions the file offset.
     * @return The new offset from the beginning of the file, negative errno on failure.
     */
    int64_t (*seek)(void *deviceData, void *fd, int64_t pos, int dir);

    /**
     * @brief Retrieves information about an open file descriptor.
     * @return 0 on success, negative errno on failure.
     */
    int (*fstat)(void *deviceData, void *fd, CR_Stat *st);

    /**
     * @brief Retrieves information about a file by its path.
     * @return 0 on success, negative errno on failure.
     */
    int (*stat)(void *deviceData, const char *file, CR_Stat *st);

    /**
     * @brief Creates a hard link to an existing file.
     * @return 0 on success, negative errno on failure.
     */
    int (*link)(void *deviceData, const char *existing, const char *newLink);

    /**
     * @brief Deletes a name and possibly the file it refers to.
     * @return 0 on success, negative errno on failure.
     */
    int (*unlink)(void *deviceData, const char *name);

    /**
     * @brief Changes the current working directory.
     * @return 0 on success, negative errno on failure.
     */
    int (*chdir)(void *deviceData, const char *name);

    /**
     * @brief Renames a file or directory.
     * @return 0 on success, negative errno on failure.
     */
    int (*rename)(void *deviceData, const char *oldName, const char *newName);

    /**
     * @brief Creates a new directory.
     * @return 0 on success, negative errno on failure.
     */
    int (*mkdir)(void *deviceData, const char *path, uint32_t mode);

    // --- Directory Operations ---

    /**
     * @brief Opens a directory stream.
     * @return 0 on success, negative errno on failure.
     */
    int (*diropen)(void *deviceData, void *dirStruct, const char *path);

    /**
     * @brief Resets a directory stream to the beginning.
     * @return 0 on success, negative errno on failure.
     */
    int (*dirreset)(void *deviceData, void *dirStruct);

    /**
     * @brief Reads the next entry from a directory stream.
     * @return 0 on success, negative errno on failure (e.g., -ENOENT for end of stream).
     */
    int (*dirnext)(void *deviceData, void *dirStruct, char *filename, CR_Stat *filestat);

    /**
     * @brief Closes a directory stream.
     * @return 0 on success, negative errno on failure.
     */
    int (*dirclose)(void *deviceData, void *dirStruct);

    // --- Advanced / VFS Operations ---
    /**
     * @brief Retrieves filesystem statistics.
     * @return 0 on success, negative errno on failure.
     */
    int (*statvfs)(void *deviceData, const char *path, CR_Statvfs *buf);

    /**
     * @brief Truncates an open file to a specified length.
     * @return 0 on success, negative errno on failure.
     */
    int (*ftruncate)(void *deviceData, void *fd, int64_t len);

    /**
     * @brief Flushes file modifications to physical storage.
     * @return 0 on success, negative errno on failure.
     */
    int (*fsync)(void *deviceData, void *fd);

    /**
     * @brief Changes the permissions of a file by path.
     * @return 0 on success, negative errno on failure.
     */
    int (*chmod)(void *deviceData, const char *path, uint32_t mode);

    /**
     * @brief Changes the permissions of an open file.
     * @return 0 on success, negative errno on failure.
     */
    int (*fchmod)(void *deviceData, void *fd, uint32_t mode);

    /**
     * @brief Removes a directory.
     * @return 0 on success, negative errno on failure.
     */
    int (*rmdir)(void *deviceData, const char *name);

    /**
     * @brief Retrieves information about a file, not following symlinks.
     * @return 0 on success, negative errno on failure.
     */
    int (*lstat)(void *deviceData, const char *file, CR_Stat *st);

    /**
     * @brief Changes the access and modification times of a file.
     * @return 0 on success, negative errno on failure.
     */
    int (*utimes)(void *deviceData, const char *filename, const CR_Timeval times[2]);

    /**
     * @brief Retrieves configuration values for an open file descriptor.
     * @return The requested configuration value, negative errno on failure.
     */
    int64_t (*fpathconf)(void *deviceData, void *fd, int name);

    /**
     * @brief Retrieves configuration values for a file by path.
     * @return The requested configuration value, negative errno on failure.
     */
    int64_t (*pathconf)(void *deviceData, const char *path, int name);

    /**
     * @brief Creates a symbolic link.
     * @return 0 on success, negative errno on failure.
     */
    int (*symlink)(void *deviceData, const char *target, const char *linkpath);

    /**
     * @brief Reads the value of a symbolic link.
     * @return Number of bytes placed in buf on success, negative errno on failure.
     */
    ssize_t (*readlink)(void *deviceData, const char *path, char *buf, size_t bufsiz);
} ContentRedirectionDeviceABI;

#ifdef __cplusplus
} // extern "C"
#endif