/*             ----> DO NOT REMOVE THE FOLLOWING NOTICE <----

                  Copyright (c) 2014-2020 Tuxera US Inc.
                      All Rights Reserved Worldwide.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; use version 2 of the License.

    This program is distributed in the hope that it will be useful,
    but "AS-IS," WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
/*  Businesses and individuals that for commercial or other reasons cannot
    comply with the terms of the GPLv2 license must obtain a commercial license
    before incorporating Reliance Edge into proprietary software for
    distribution in any form.  Visit http://www.datalight.com/reliance-edge for
    more information.
*/
/** @file
    @brief Implements block device I/O.
*/

#define __MS_IO
#include <ms_rtos.h>

#include <redfs.h>
#include <redvolume.h>
#include <redbdev.h>

/** @brief Initialize a block device.

    This function is called when the file system needs access to a block
    device.

    Upon successful return, the block device should be fully initialized and
    ready to service read/write/flush/close requests.

    The behavior of calling this function on a block device which is already
    open is undefined.

    @param bVolNum  The volume number of the volume whose block device is being
                    initialized.
    @param mode     The open mode, indicating the type of access required.

    @return A negated ::REDSTATUS code indicating the operation result.

    @retval 0           Operation was successful.
    @retval -RED_EINVAL @p bVolNum is an invalid volume number; or invalid
                        sector geometry.
    @retval -RED_EIO    A disk I/O error occurred.
*/
REDSTATUS RedOsBDevOpen(
    uint8_t         bVolNum,
    BDEVOPENMODE    mode)
{
    REDSTATUS       ret;

    if((bVolNum >= REDCONF_VOLUME_COUNT) || (gaRedVolConf[bVolNum].dev == NULL))
    {
        ret = -RED_EINVAL;
    }
    else
    {
        ms_io_device_t *dev = (ms_io_device_t *)gaRedVolConf[bVolNum].dev;

        if(dev->drv->ops->ioctl(dev->ctx, NULL, MS_BLKDEV_CMD_INIT, NULL) < 0)
        {
            ret = -RED_EIO;
        }
        else
        {
            ret = 0;
        }
    }

    return ret;
}


/** @brief Uninitialize a block device.

    This function is called when the file system no longer needs access to a
    block device.  If any resource were allocated by RedOsBDevOpen() to service
    block device requests, they should be freed at this time.

    Upon successful return, the block device must be in such a state that it
    can be opened again.

    The behavior of calling this function on a block device which is already
    closed is undefined.

    @param bVolNum  The volume number of the volume whose block device is being
                    uninitialized.

    @return A negated ::REDSTATUS code indicating the operation result.

    @retval 0           Operation was successful.
    @retval -RED_EINVAL @p bVolNum is an invalid volume number.
*/
REDSTATUS RedOsBDevClose(
    uint8_t     bVolNum)
{
    REDSTATUS   ret;

    if((bVolNum >= REDCONF_VOLUME_COUNT) || (gaRedVolConf[bVolNum].dev == NULL))
    {
        ret = -RED_EINVAL;
    }
    else
    {
//        ms_io_device_t *dev = (ms_io_device_t *)gaRedVolConf[bVolNum].dev;
//
//        if(dev->drv->ops->ioctl(dev->ctx, NULL, MS_IO_BLKDEV_CMD_CLOSE, NULL) < 0)
//        {
//            ret = -RED_EIO;
//        }
//        else
//        {
            gaRedVolConf[bVolNum].dev = NULL;
            ret = 0;
//        }
    }

    return ret;
}


/** @brief Return the block device geometry.

    The behavior of calling this function is undefined if the block device is
    closed.

    @param bVolNum  The volume number of the volume whose block device geometry
                    is being queried.
    @param pInfo    On successful return, populated with the geometry of the
                    block device.

    @return A negated ::REDSTATUS code indicating the operation result.

    @retval 0               Operation was successful.
    @retval -RED_EINVAL     @p bVolNum is an invalid volume number; or @p pInfo
                            is `NULL`; or invalid sector geometry.
    @retval -RED_EIO        A disk I/O error occurred.
    @retval -RED_ENOTSUPP   The geometry cannot be queried on this block device.
*/
REDSTATUS RedOsBDevGetGeometry(
    uint8_t     bVolNum,
    BDEVINFO   *pInfo)
{
    REDSTATUS   ret;

    if((bVolNum >= REDCONF_VOLUME_COUNT) || (pInfo == NULL) || (gaRedVolConf[bVolNum].dev == NULL))
    {
        ret = -RED_EINVAL;
    }
    else
    {
        ms_io_device_t *dev = (ms_io_device_t *)gaRedVolConf[bVolNum].dev;
        ms_uint32_t sector_count;

        if(dev->drv->ops->ioctl(dev->ctx, NULL, MS_BLKDEV_CMD_GET_SECT_NR, &sector_count) < 0)
        {
            ret = -RED_EIO;
        }
        else
        {
            pInfo->ullSectorCount = sector_count;

            if(dev->drv->ops->ioctl(dev->ctx, NULL, MS_BLKDEV_CMD_GET_SECT_SZ, &pInfo->ulSectorSize) < 0)
            {
                ret = -RED_EIO;
            }
            else
            {
                ret = 0;
            }
        }
    }

    return ret;
}


/** @brief Read sectors from a physical block device.

    The behavior of calling this function is undefined if the block device is
    closed or if it was opened with ::BDEV_O_WRONLY.

    @param bVolNum          The volume number of the volume whose block device
                            is being read from.
    @param ullSectorStart   The starting sector number.
    @param ulSectorCount    The number of sectors to read.
    @param pBuffer          The buffer into which to read the sector data.

    @return A negated ::REDSTATUS code indicating the operation result.

    @retval 0           Operation was successful.
    @retval -RED_EINVAL @p bVolNum is an invalid volume number, @p pBuffer is
                        `NULL`, or @p ullStartSector and/or @p ulSectorCount
                        refer to an invalid range of sectors.
    @retval -RED_EIO    A disk I/O error occurred.
*/
REDSTATUS RedOsBDevRead(
    uint8_t     bVolNum,
    uint64_t    ullSectorStart,
    uint32_t    ulSectorCount,
    void       *pBuffer)
{
    REDSTATUS   ret;

    if(    (bVolNum >= REDCONF_VOLUME_COUNT)
        || !VOLUME_SECTOR_RANGE_IS_VALID(bVolNum, ullSectorStart, ulSectorCount)
        || (pBuffer == NULL)
        || (gaRedVolConf[bVolNum].dev == NULL))
    {
        ret = -RED_EINVAL;
    }
    else
    {
        ms_io_device_t *dev = (ms_io_device_t *)gaRedVolConf[bVolNum].dev;

        if(dev->drv->ops->readblk(dev->ctx, NULL, ullSectorStart, ulSectorCount, pBuffer) < 0)
        {
            ret = -RED_EIO;
        }
        else
        {
            ret = 0;
        }
    }

    return ret;
}


#if REDCONF_READ_ONLY == 0
/** @brief Write sectors to a physical block device.

    The behavior of calling this function is undefined if the block device is
    closed or if it was opened with ::BDEV_O_RDONLY.

    @param bVolNum          The volume number of the volume whose block device
                            is being written to.
    @param ullSectorStart   The starting sector number.
    @param ulSectorCount    The number of sectors to write.
    @param pBuffer          The buffer from which to write the sector data.

    @return A negated ::REDSTATUS code indicating the operation result.

    @retval 0           Operation was successful.
    @retval -RED_EINVAL @p bVolNum is an invalid volume number, @p pBuffer is
                        `NULL`, or @p ullStartSector and/or @p ulSectorCount
                        refer to an invalid range of sectors.
    @retval -RED_EIO    A disk I/O error occurred.
*/
REDSTATUS RedOsBDevWrite(
    uint8_t     bVolNum,
    uint64_t    ullSectorStart,
    uint32_t    ulSectorCount,
    const void *pBuffer)
{
    REDSTATUS   ret;

    if(    (bVolNum >= REDCONF_VOLUME_COUNT)
        || !VOLUME_SECTOR_RANGE_IS_VALID(bVolNum, ullSectorStart, ulSectorCount)
        || (pBuffer == NULL)
        || (gaRedVolConf[bVolNum].dev == NULL))
    {
        ret = -RED_EINVAL;
    }
    else
    {
        ms_io_device_t *dev = (ms_io_device_t *)gaRedVolConf[bVolNum].dev;

        if(dev->drv->ops->writeblk(dev->ctx, MS_NULL, ullSectorStart, ulSectorCount, pBuffer) < 0)
        {
            ret = -RED_EIO;
        }
        else
        {
            ret = 0;
        }
    }

    return ret;
}


/** @brief Flush any caches beneath the file system.

    This function must synchronously flush all software and hardware caches
    beneath the file system, ensuring that all sectors written previously are
    committed to permanent storage.

    If the environment has no caching beneath the file system, the
    implementation of this function can do nothing and return success.

    The behavior of calling this function is undefined if the block device is
    closed or if it was opened with ::BDEV_O_RDONLY.

    @param bVolNum  The volume number of the volume whose block device is being
                    flushed.

    @return A negated ::REDSTATUS code indicating the operation result.

    @retval 0           Operation was successful.
    @retval -RED_EINVAL @p bVolNum is an invalid volume number.
    @retval -RED_EIO    A disk I/O error occurred.
*/
REDSTATUS RedOsBDevFlush(
    uint8_t     bVolNum)
{
    REDSTATUS   ret;

    if((bVolNum >= REDCONF_VOLUME_COUNT) || (gaRedVolConf[bVolNum].dev == NULL))
    {
        ret = -RED_EINVAL;
    }
    else
    {
        ms_io_device_t *dev = (ms_io_device_t *)gaRedVolConf[bVolNum].dev;

        if(dev->drv->ops->ioctl(dev->ctx, NULL, MS_BLKDEV_CMD_SYNC, NULL) < 0)
        {
            ret = -RED_EIO;
        }
        else
        {
            ret = 0;
        }
    }

    return ret;
}
#endif /* REDCONF_READ_ONLY == 0 */

