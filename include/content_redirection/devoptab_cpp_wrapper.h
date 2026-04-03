#pragma once

#ifdef __cplusplus

#include "defines.h"

#include <array>
#include <cctype>
#include <coreinit/debug.h>
#include <cstring>
#include <errno.h>
#include <mutex>
#include <sys/iosupport.h>
#include <sys/reent.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/time.h>
#include <utility>

namespace CR_DevoptabWrapper {
#ifndef CR_MAX_RUNTIME_DEVICES
#define CR_MAX_RUNTIME_DEVICES 4
#endif
    constexpr size_t MAX_RUNTIME_DEVICES = CR_MAX_RUNTIME_DEVICES;

    struct Backend {
        static void stat_to_cr_stat(const struct stat &src, CR_Stat *dst) {
            if (!dst) {
                return;
            }
            dst->dev     = src.st_dev;
            dst->ino     = src.st_ino;
            dst->mode    = src.st_mode;
            dst->nlink   = src.st_nlink;
            dst->uid     = src.st_uid;
            dst->gid     = src.st_gid;
            dst->rdev    = src.st_rdev;
            dst->size    = src.st_size;
            dst->atime   = src.st_atime;
            dst->mtime   = src.st_mtime;
            dst->ctime   = src.st_ctime;
            dst->blksize = src.st_blksize;
            dst->blocks  = src.st_blocks;
        }

        static void statvfs_to_cr_statvfs(const struct statvfs &src, CR_Statvfs *dst) {
            if (!dst) {
                return;
            }
            dst->bsize   = src.f_bsize;
            dst->frsize  = src.f_frsize;
            dst->blocks  = src.f_blocks;
            dst->bfree   = src.f_bfree;
            dst->bavail  = src.f_bavail;
            dst->files   = src.f_files;
            dst->ffree   = src.f_ffree;
            dst->favail  = src.f_favail;
            dst->fsid    = src.f_fsid;
            dst->flag    = src.f_flag;
            dst->namemax = src.f_namemax;
        }

        static int get_error(struct _reent *r) {
            return r->_errno != 0 ? -(r->_errno) : -EIO;
        }

        static struct _reent *get_reent(const devoptab_t *dev) {
            auto *r       = _REENT;
            r->deviceData = dev->deviceData;
            return r;
        }

        static int open(const devoptab_t *dev, void *fileStruct, const char *path, int flags, uint32_t mode) {
            if (!dev || !dev->open_r) {
                return -ENOSYS;
            }
            auto *r       = get_reent(dev);
            const int res = dev->open_r(r, fileStruct, path, flags, static_cast<int>(mode));
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }

        static int close(const devoptab_t *dev, void *fd) {
            if (!dev || !dev->close_r) {
                return -ENOSYS;
            }
            auto *r       = get_reent(dev);
            const int res = dev->close_r(r, fd);
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }

        static ssize_t write(const devoptab_t *dev, void *fd, const char *ptr, size_t len) {
            if (!dev || !dev->write_r) {
                return -ENOSYS;
            }
            auto *r           = get_reent(dev);
            const ssize_t res = dev->write_r(r, fd, ptr, len);
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }

        static ssize_t read(const devoptab_t *dev, void *fd, char *ptr, size_t len) {
            if (!dev || !dev->read_r) {
                return -ENOSYS;
            }
            auto *r           = get_reent(dev);
            const ssize_t res = dev->read_r(r, fd, ptr, len);
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }

        static int64_t seek(const devoptab_t *dev, void *fd, int64_t pos, int dir) {
            if (!dev || !dev->seek_r) {
                return -ENOSYS;
            }
            auto *r         = get_reent(dev);
            const off_t res = dev->seek_r(r, fd, static_cast<off_t>(pos), dir);
            if (res == static_cast<off_t>(-1)) {
                return get_error(r);
            }
            return static_cast<int64_t>(res);
        }

        static int fstat(const devoptab_t *dev, void *fd, CR_Stat *st) {
            if (!dev || !dev->fstat_r) {
                return -ENOSYS;
            }
            auto *r = get_reent(dev);
            struct stat local_st {};
            const int res = dev->fstat_r(r, fd, &local_st);
            if (res == -1) {
                return get_error(r);
            }
            stat_to_cr_stat(local_st, st);
            return res;
        }

        static int stat(const devoptab_t *dev, const char *file, CR_Stat *st) {
            if (!dev || !dev->stat_r) {
                return -ENOSYS;
            }
            auto *r = get_reent(dev);
            struct stat local_st {};
            const int res = dev->stat_r(r, file, &local_st);
            if (res == -1) {
                return get_error(r);
            }
            stat_to_cr_stat(local_st, st);
            return res;
        }

        static int link(const devoptab_t *dev, const char *existing, const char *newLink) {
            if (!dev || !dev->link_r) {
                return -ENOSYS;
            }
            auto *r       = get_reent(dev);
            const int res = dev->link_r(r, existing, newLink);
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }

        static int unlink(const devoptab_t *dev, const char *name) {
            if (!dev || !dev->unlink_r) {
                return -ENOSYS;
            }
            auto *r       = get_reent(dev);
            const int res = dev->unlink_r(r, name);
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }

        static int chdir(const devoptab_t *dev, const char *name) {
            if (!dev || !dev->chdir_r) {
                return -ENOSYS;
            }
            auto *r       = get_reent(dev);
            const int res = dev->chdir_r(r, name);
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }

        static int rename(const devoptab_t *dev, const char *oldName, const char *newName) {
            if (!dev || !dev->rename_r) {
                return -ENOSYS;
            }
            auto *r       = get_reent(dev);
            const int res = dev->rename_r(r, oldName, newName);
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }

        static int mkdir(const devoptab_t *dev, const char *path, uint32_t mode) {
            if (!dev || !dev->mkdir_r) {
                return -ENOSYS;
            }
            auto *r       = get_reent(dev);
            const int res = dev->mkdir_r(r, path, static_cast<int>(mode));
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }

        static int diropen(const devoptab_t *dev, int deviceId, void *dirStruct, const char *path) {
            if (!dev || !dev->diropen_r) {
                return -ENOSYS;
            }
            auto *r = get_reent(dev);
            DIR_ITER dummy{};
            dummy.device    = deviceId;
            dummy.dirStruct = dirStruct;

            if (dev->diropen_r(r, &dummy, path) == nullptr) {
                return get_error(r);
            }
            return 0;
        }

        static int dirreset(const devoptab_t *dev, int deviceId, void *dirStruct) {
            if (!dev || !dev->dirreset_r) {
                return -ENOSYS;
            }
            auto *r = get_reent(dev);
            DIR_ITER dummy{};
            dummy.device    = deviceId;
            dummy.dirStruct = dirStruct;

            const int res = dev->dirreset_r(r, &dummy);
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }

        static int dirnext(const devoptab_t *dev, int deviceId, void *dirStruct, char *filename, CR_Stat *filestat) {
            if (!dev || !dev->dirnext_r) {
                return -ENOSYS;
            }
            auto *r = get_reent(dev);
            DIR_ITER dummy{};
            dummy.device    = deviceId;
            dummy.dirStruct = dirStruct;
            struct stat local_st {};
            const int res = dev->dirnext_r(r, &dummy, filename, &local_st);
            if (res == -1) {
                return get_error(r);
            }
            stat_to_cr_stat(local_st, filestat);
            return res;
        }

        static int dirclose(const devoptab_t *dev, int deviceId, void *dirStruct) {
            if (!dev || !dev->dirclose_r) {
                return -ENOSYS;
            }
            auto *r = get_reent(dev);
            DIR_ITER dummy{};
            dummy.device    = deviceId;
            dummy.dirStruct = dirStruct;
            const int res   = dev->dirclose_r(r, &dummy);
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }

        static int statvfs(const devoptab_t *dev, const char *path, CR_Statvfs *buf) {
            if (!dev || !dev->statvfs_r) {
                return -ENOSYS;
            }
            auto *r = get_reent(dev);
            struct statvfs local_buf {};
            const int res = dev->statvfs_r(r, path, &local_buf);
            if (res == -1) {
                return get_error(r);
            }
            statvfs_to_cr_statvfs(local_buf, buf);
            return res;
        }

        static int ftruncate(const devoptab_t *dev, void *fd, int64_t len) {
            if (!dev || !dev->ftruncate_r) {
                return -ENOSYS;
            }
            auto *r       = get_reent(dev);
            const int res = dev->ftruncate_r(r, fd, static_cast<off_t>(len));
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }

        static int fsync(const devoptab_t *dev, void *fd) {
            if (!dev || !dev->fsync_r) {
                return -ENOSYS;
            }
            auto *r       = get_reent(dev);
            const int res = dev->fsync_r(r, fd);
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }

        static int chmod(const devoptab_t *dev, const char *path, uint32_t mode) {
            if (!dev || !dev->chmod_r) {
                return -ENOSYS;
            }
            auto *r       = get_reent(dev);
            const int res = dev->chmod_r(r, path, static_cast<mode_t>(mode));
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }

        static int fchmod(const devoptab_t *dev, void *fd, uint32_t mode) {
            if (!dev || !dev->fchmod_r) {
                return -ENOSYS;
            }
            auto *r       = get_reent(dev);
            const int res = dev->fchmod_r(r, fd, static_cast<mode_t>(mode));
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }

        static int rmdir(const devoptab_t *dev, const char *name) {
            if (!dev || !dev->rmdir_r) {
                return -ENOSYS;
            }
            auto *r = get_reent(dev);
            int res = dev->rmdir_r(r, name);
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }

        static int lstat(const devoptab_t *dev, const char *file, CR_Stat *st) {
            if (!dev || !dev->lstat_r) {
                return -ENOSYS;
            }
            auto *r = get_reent(dev);
            struct stat local_st {};
            const int res = dev->lstat_r(r, file, &local_st);
            if (res == -1) {
                return get_error(r);
            }
            stat_to_cr_stat(local_st, st);
            return res;
        }

        static int utimes(const devoptab_t *dev, const char *filename, const CR_Timeval times[2]) {
            if (!dev || !dev->utimes_r) {
                return -ENOSYS;
            }
            auto *r = get_reent(dev);

            int res;
            if (!times) {
                res = dev->utimes_r(r, filename, nullptr);
            } else {
                timeval local_times[2];
                local_times[0].tv_sec  = static_cast<time_t>(times[0].tv_sec);
                local_times[0].tv_usec = static_cast<suseconds_t>(times[0].tv_usec);
                local_times[1].tv_sec  = static_cast<time_t>(times[1].tv_sec);
                local_times[1].tv_usec = static_cast<suseconds_t>(times[1].tv_usec);
                res                    = dev->utimes_r(r, filename, local_times);
            }
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }

        static int64_t fpathconf(const devoptab_t *dev, void *fd, int name) {
            if (!dev || !dev->fpathconf_r) {
                return -ENOSYS;
            }
            auto *r        = get_reent(dev);
            const long res = dev->fpathconf_r(r, fd, name);
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }

        static int64_t pathconf(const devoptab_t *dev, const char *path, int name) {
            if (!dev || !dev->pathconf_r) {
                return -ENOSYS;
            }
            auto *r        = get_reent(dev);
            const long res = dev->pathconf_r(r, path, name);
            if (res == -1) {
                return get_error(r);
            }
            return static_cast<int64_t>(res);
        }

        static int symlink(const devoptab_t *dev, const char *target, const char *linkpath) {
            if (!dev || !dev->symlink_r) {
                return -ENOSYS;
            }
            auto *r       = get_reent(dev);
            const int res = dev->symlink_r(r, target, linkpath);
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }

        static ssize_t readlink(const devoptab_t *dev, const char *path, char *buf, size_t bufsiz) {
            if (!dev || !dev->readlink_r) {
                return -ENOSYS;
            }
            auto *r           = get_reent(dev);
            const ssize_t res = dev->readlink_r(r, path, buf, bufsiz);
            if (res == -1) {
                return get_error(r);
            }
            return res;
        }
    };

    template<size_t Slot>
    struct RuntimeSlot {
        inline static const devoptab_t *dev           = nullptr;
        inline static ContentRedirectionDeviceABI abi = {};
        inline static int deviceId                    = -1;

        static int open(void *fileStruct, const char *path, int flags, uint32_t mode) {
            return Backend::open(dev, fileStruct, path, flags, mode);
        }

        static int close(void *fd) {
            return Backend::close(dev, fd);
        }

        static ssize_t write(void *fd, const char *ptr, size_t len) {
            return Backend::write(dev, fd, ptr, len);
        }

        static ssize_t read(void *fd, char *ptr, size_t len) {
            return Backend::read(dev, fd, ptr, len);
        }

        static int64_t seek(void *fd, int64_t pos, int dir) {
            return Backend::seek(dev, fd, pos, dir);
        }

        static int fstat(void *fd, CR_Stat *st) {
            return Backend::fstat(dev, fd, st);
        }

        static int stat(const char *file, CR_Stat *st) {
            return Backend::stat(dev, file, st);
        }

        static int link(const char *existing, const char *newLink) {
            return Backend::link(dev, existing, newLink);
        }

        static int unlink(const char *name) {
            return Backend::unlink(dev, name);
        }
        static int chdir(const char *name) {
            return Backend::chdir(dev, name);
        }

        static int rename(const char *oldName, const char *newName) {
            return Backend::rename(dev, oldName, newName);
        }

        static int mkdir(const char *path, uint32_t mode) {
            return Backend::mkdir(dev, path, mode);
        }

        static int diropen(void *dirStruct, const char *path) {
            return Backend::diropen(dev, deviceId, dirStruct, path);
        }

        static int dirreset(void *dirStruct) {
            return Backend::dirreset(dev, deviceId, dirStruct);
        }

        static int dirnext(void *dirStruct, char *filename, CR_Stat *filestat) {
            return Backend::dirnext(dev, deviceId, dirStruct, filename, filestat);
        }

        static int dirclose(void *dirStruct) {
            return Backend::dirclose(dev, deviceId, dirStruct);
        }

        static int statvfs(const char *path, CR_Statvfs *buf) {
            return Backend::statvfs(dev, path, buf);
        }

        static int ftruncate(void *fd, int64_t len) {
            return Backend::ftruncate(dev, fd, len);
        }

        static int fsync(void *fd) {
            return Backend::fsync(dev, fd);
        }

        static int chmod(const char *path, uint32_t mode) {
            return Backend::chmod(dev, path, mode);
        }

        static int fchmod(void *fd, uint32_t mode) {
            return Backend::fchmod(dev, fd, mode);
        }

        static int rmdir(const char *name) {
            return Backend::rmdir(dev, name);
        }

        static int lstat(const char *file, CR_Stat *st) {
            return Backend::lstat(dev, file, st);
        }

        static int utimes(const char *filename, const CR_Timeval times[2]) {
            return Backend::utimes(dev, filename, times);
        }

        static int64_t fpathconf(void *fd, int name) {
            return Backend::fpathconf(dev, fd, name);
        }

        static int64_t pathconf(const char *path, int name) {
            return Backend::pathconf(dev, path, name);
        }

        static int symlink(const char *target, const char *linkpath) {
            return Backend::symlink(dev, target, linkpath);
        }

        static ssize_t readlink(const char *path, char *buf, size_t bufsiz) {
            return Backend::readlink(dev, path, buf, bufsiz);
        }

        static ContentRedirectionDeviceABI *bind(const devoptab_t *device) {
            dev              = device;
            abi.magic        = CONTENT_REDIRECTION_DEVICE_MAGIC;
            abi.version      = CONTENT_REDIRECTION_DEVICE_VERSION;
            abi.name         = dev->name;
            abi.structSize   = dev->structSize;
            abi.dirStateSize = dev->dirStateSize;
            abi.deviceData   = dev->deviceData;

            deviceId = -1;
            for (int i = 0; i < STD_MAX; i++) {
                if (devoptab_list[i] == dev) {
                    deviceId = i;
                    break;
                }
            }

            abi.open   = dev->open_r ? open : nullptr;
            abi.close  = dev->close_r ? close : nullptr;
            abi.write  = dev->write_r ? write : nullptr;
            abi.read   = dev->read_r ? read : nullptr;
            abi.seek   = dev->seek_r ? seek : nullptr;
            abi.fstat  = dev->fstat_r ? fstat : nullptr;
            abi.stat   = dev->stat_r ? stat : nullptr;
            abi.link   = dev->link_r ? link : nullptr;
            abi.unlink = dev->unlink_r ? unlink : nullptr;
            abi.chdir  = dev->chdir_r ? chdir : nullptr;
            abi.rename = dev->rename_r ? rename : nullptr;
            abi.mkdir  = dev->mkdir_r ? mkdir : nullptr;

            abi.diropen  = dev->diropen_r ? diropen : nullptr;
            abi.dirreset = dev->dirreset_r ? dirreset : nullptr;
            abi.dirnext  = dev->dirnext_r ? dirnext : nullptr;
            abi.dirclose = dev->dirclose_r ? dirclose : nullptr;

            abi.statvfs   = dev->statvfs_r ? statvfs : nullptr;
            abi.ftruncate = dev->ftruncate_r ? ftruncate : nullptr;
            abi.fsync     = dev->fsync_r ? fsync : nullptr;
            abi.chmod     = dev->chmod_r ? chmod : nullptr;
            abi.fchmod    = dev->fchmod_r ? fchmod : nullptr;
            abi.rmdir     = dev->rmdir_r ? rmdir : nullptr;
            abi.lstat     = dev->lstat_r ? lstat : nullptr;
            abi.utimes    = dev->utimes_r ? utimes : nullptr;
            abi.fpathconf = dev->fpathconf_r ? fpathconf : nullptr;
            abi.pathconf  = dev->pathconf_r ? pathconf : nullptr;
            abi.symlink   = dev->symlink_r ? symlink : nullptr;
            abi.readlink  = dev->readlink_r ? readlink : nullptr;

            return &abi;
        }
    };

    using RuntimeBinder = ContentRedirectionDeviceABI *(*) (const devoptab_t *);

    template<size_t... Is>
    constexpr auto make_runtime_binders(std::index_sequence<Is...>) {
        return std::array<RuntimeBinder, sizeof...(Is)>{RuntimeSlot<Is>::bind...};
    }

    struct GlobalState {
        static constexpr auto binders                                                   = make_runtime_binders(std::make_index_sequence<MAX_RUNTIME_DEVICES>{});
        inline static std::array<const devoptab_t *, MAX_RUNTIME_DEVICES> activeDevices = {};
        inline static std::recursive_mutex deviceMutex;
    };

} // namespace CR_DevoptabWrapper

/**
 * @brief Transparent, ABI-safe wrapper for registering devoptab_t devices.
 * Because this is inline C++, it is compiled entirely inside the calling plugin's environment.
 */
inline ContentRedirectionStatus ContentRedirection_AddDevice(const devoptab_t *device, int *resultOut) {
    if (!device || !resultOut) {
        return CONTENT_REDIRECTION_RESULT_INVALID_ARGUMENT;
    }

    using namespace CR_DevoptabWrapper;

    std::lock_guard lock(GlobalState::deviceMutex);

    for (size_t i = 0; i < MAX_RUNTIME_DEVICES; i++) {
        if (GlobalState::activeDevices[i] == nullptr || GlobalState::activeDevices[i] == device) {
            GlobalState::activeDevices[i] = device;

            const auto *abiDevice = GlobalState::binders[i](device);

            return ContentRedirection_AddDeviceABI(abiDevice, resultOut);
        }
    }
    return CONTENT_REDIRECTION_RESULT_NO_MEMORY;
}

inline ContentRedirectionStatus ContentRedirection_RemoveDevice(const char *deviceName, int *resultOut) {
    if (!deviceName || !resultOut) {
        return CONTENT_REDIRECTION_RESULT_INVALID_ARGUMENT;
    }

    using namespace CR_DevoptabWrapper;

    std::lock_guard lock(GlobalState::deviceMutex);

    auto *separator      = strchr(deviceName, ':');
    size_t deviceNameLen = (separator != nullptr) ? (separator - deviceName) : strlen(deviceName);

    for (size_t i = 0; i < MAX_RUNTIME_DEVICES; i++) {
        if (GlobalState::activeDevices[i]) {
            size_t namelen = strlen(GlobalState::activeDevices[i]->name);

            if (deviceNameLen == namelen) {
                if (strncmp(GlobalState::activeDevices[i]->name, deviceName, deviceNameLen) == 0) {
                    GlobalState::activeDevices[i] = nullptr;
                }
            }
        }
    }

    return ::ContentRedirection_RemoveDeviceABI(deviceName, resultOut);
}

#endif // __cplusplus