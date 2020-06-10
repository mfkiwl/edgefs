#*********************************************************************************************************
#
#                                 北京翼辉信息技术有限公司
#
#                                   微型安全实时操作系统
#
#                                       MS-RTOS(TM)
#
#                               Copyright All Rights Reserved
#
#--------------文件信息--------------------------------------------------------------------------------
#
# 文   件   名: config.mk
#
# 创   建   人: IoT Studio
#
# 文件创建日期: 2020 年 05 月 13 日
#
# 描        述: 本文件由 IoT Studio 生成，用于配置 Makefile 功能，请勿手动修改
#*********************************************************************************************************

#*********************************************************************************************************
# MS-RTOS Base Project path
#*********************************************************************************************************
MSRTOS_BASE_PATH := $(WORKSPACE_msrtos_base)

#*********************************************************************************************************
# Debug options (debug or release)
#*********************************************************************************************************
DEBUG_LEVEL := debug

#*********************************************************************************************************
# NOTICE: libmsrtos, BSP and other kernel modules projects CAN NOT use vfp!
#*********************************************************************************************************
FPU_TYPE := disable

#*********************************************************************************************************
# End
#*********************************************************************************************************
