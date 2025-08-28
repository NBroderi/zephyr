#include "tt_zephyr/zephyr_flashfs.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <tt_sdk/plugins/flashfs.h>
#include <zephyr/fs/fs.h>

#define FILENAME_LEN (TT_ZEPHYR_FLASHFS_MOUNT_POINT_MAX_LEN + 12)
#define FILENAME_FMT "%s/tt_sdk_%03d"

static TtError zephyrGetSize(void*, TtFlashfsFile type, size_t* bytes);
static TtError zephyrFileRead(void*, TtFlashfsFile type, size_t offset, void* buf, size_t* len);
static TtError zephyrFileWrite(void*, TtFlashfsFile type, const void* buf, size_t len);
static TtError zephyrFileDelete(void*, TtFlashfsFile type);

TtFlashfs ttPluginFlashfs = {
    .context = NULL,
    .name = "zephyr",
    .init = NULL,
    .getSize = &zephyrGetSize,
    .fileRead = &zephyrFileRead,
    .fileWrite = &zephyrFileWrite,
    .fileDelete = &zephyrFileDelete,
};

static char zephyrMountPoint[TT_ZEPHYR_FLASHFS_MOUNT_POINT_MAX_LEN] = { 0 };

// helper function to check if a file exists
static TtError fileExists(TtFlashfsFile file, size_t* bytes)
{
    // convert the file type to a name
    char buf[FILENAME_LEN];
    int ret = snprintf(buf, TT_ARRAY_SIZE(buf), FILENAME_FMT, zephyrMountPoint, (int)file);
    if (ret <= 0 || ret >= TT_ARRAY_SIZE(buf)) {
        // something went wrong
        return TT_E_FAULT;
    }

    // try to get the file stats
    struct fs_dirent entry;
    ret = fs_stat(buf, &entry);
    if (ret == -ENOENT) {
        return TT_E_NOT_FOUND;
    } else if (ret != 0) {
        return TT_E_FAULT;
    }

    // the file exists, return the size if requested
    if (bytes != NULL) {
        *bytes = entry.size;
    }

    return TT_E_OK;
}

// helper function to open a file
static TtError openFile(TtFlashfsFile file, struct fs_file_t* filePtr, bool write)
{
    // convert the file type to a name
    char buf[FILENAME_LEN];
    int ret = snprintf(buf, TT_ARRAY_SIZE(buf), FILENAME_FMT, zephyrMountPoint, (int)file);
    if (ret <= 0 || ret >= TT_ARRAY_SIZE(buf)) {
        // something went wrong
        return TT_E_FAULT;
    }

    // initialize the file structure
    fs_file_t_init(filePtr);

    // Zephyr versions before 3.7.0 do not support FS_O_TRUNC
#ifdef FS_O_TRUNC
    // try to open the file (this will truncate any content in write mode)
    ret = fs_open(filePtr, buf, write ? (FS_O_WRITE | FS_O_CREATE | FS_O_TRUNC) : FS_O_READ);
    if (ret == -ENOENT) {
        return TT_E_NOT_FOUND;
    } else if (ret != 0) {
        return TT_E_FAULT;
    }
#else
    // try to open the file
    ret = fs_open(filePtr, buf, write ? (FS_O_WRITE | FS_O_CREATE) : FS_O_READ);
    if (ret == -ENOENT) {
        return TT_E_NOT_FOUND;
    } else if (ret != 0) {
        return TT_E_FAULT;
    }

    // truncate any content in write mode
    if (write) {
        ret = fs_truncate(filePtr, 0);
        if (ret != 0) {
            // something went wrong
            fs_close(filePtr); // ignore errors here
            return TT_E_FAULT;
        }
    }
#endif

    return TT_E_OK;
}

static TtError zephyrGetSize(void*, TtFlashfsFile type, size_t* bytes)
{
    // make sure the plugin was initialized
    if (zephyrMountPoint[0] == '\0') {
        return TT_E_NOT_INITIALIZED;
    }

    // check if it exists (also returns the size if requested)
    return fileExists(type, bytes);
}

static TtError zephyrFileRead(void*, TtFlashfsFile type, size_t offset, void* buf, size_t* len)
{
    // make sure the plugin was initialized
    if (zephyrMountPoint[0] == '\0') {
        return TT_E_NOT_INITIALIZED;
    }

    // try to open the file for reading
    struct fs_file_t file;
    TtError err = openFile(type, &file, false);
    if (TT_IS_ERR(err)) {
        return err;
    }

    // seek to the read offset (if needed)
    if (offset > 0) {
        const int ret = fs_seek(&file, offset, FS_SEEK_SET);
        if (ret != 0) {
            fs_close(&file);
            return (ret == -EINVAL) ? TT_E_INVALID_ARG : TT_E_FAULT;
        }
    }

    // try to read the requested amount of data
    const ssize_t ret = fs_read(&file, buf, *len);
    fs_close(&file); // we don't care about errors here (we did not change the file)
    if (ret < 0) {
        return TT_E_FAULT;
    }
    *len = (size_t)ret;

    return TT_E_OK;
}

static TtError zephyrFileWrite(void*, TtFlashfsFile type, const void* buf, size_t len)
{
    // make sure the plugin was initialized
    if (zephyrMountPoint[0] == '\0') {
        return TT_E_NOT_INITIALIZED;
    }

    // open the file for writing
    struct fs_file_t file;
    const TtError err = openFile(type, &file, false);
    if (TT_IS_ERR(err)) {
        return err;
    }

    // write the data
    ssize_t written = fs_write(&file, buf, len);
    if (written != len) {
        // either a error number or not everything was written
        if (written >= 0) {
            // partial write, get the reason why
            written = -errno;
        }

        // close the file (ignore any error) and return the error code
        fs_close(&file);
        return (written == -ENOMEM) ? TT_E_NO_MEMORY : TT_E_FAULT;
    }

    // close the file
    const int ret = fs_close(&file);
    if (ret < 0) {
        return (ret == -ENOMEM) ? TT_E_NO_MEMORY : TT_E_FAULT;
    }

    return TT_E_OK;
}

static TtError zephyrFileDelete(void*, TtFlashfsFile type)
{
    // make sure the plugin was initialized
    if (zephyrMountPoint[0] == '\0') {
        return TT_E_NOT_INITIALIZED;
    }

    // convert the file type to a name
    char buf[FILENAME_LEN];
    int ret = snprintf(buf, TT_ARRAY_SIZE(buf), FILENAME_FMT, zephyrMountPoint, (int)type);
    if (ret <= 0 || ret >= TT_ARRAY_SIZE(buf)) {
        // something went wrong
        return TT_E_FAULT;
    }

    // try to unlink the file
    ret = fs_unlink(buf);
    if (ret < 0) {
        return (ret == -ENOENT) ? TT_E_NOT_FOUND : TT_E_FAULT;
    }

    return TT_E_OK;
}

TT_NO_DISCARD TtError ttZephyrFlashfsInit(const char* mountPoint)
{
    // check the required argument
    if (mountPoint == NULL
        || strlen(mountPoint) < 2
        || strlen(mountPoint) >= TT_ZEPHYR_FLASHFS_MOUNT_POINT_MAX_LEN
        || mountPoint[0] != '/') {
        return TT_E_INVALID_ARG;
    }

    // store the mount point
    strncpy(zephyrMountPoint, mountPoint, TT_ARRAY_SIZE(zephyrMountPoint));
    zephyrMountPoint[TT_ARRAY_SIZE(zephyrMountPoint) - 1] = '\0';
    const size_t lastPos = strlen(zephyrMountPoint) - 1;
    if (zephyrMountPoint[lastPos] == '/') {
        zephyrMountPoint[lastPos] = '\0';
    }

    // make sure the mount point exists
    struct fs_dirent entry;
    const int ret = fs_stat(zephyrMountPoint, &entry);
    if (ret == -ENOENT) {
        return TT_E_NOT_FOUND;
    } else if (ret != 0) {
        return TT_E_FAULT;
    }

    // make sure the mount point is a folder
    if (entry.type != FS_DIR_ENTRY_DIR) {
        return TT_E_INVALID_ARG;
    }

    return TT_E_OK;
}
