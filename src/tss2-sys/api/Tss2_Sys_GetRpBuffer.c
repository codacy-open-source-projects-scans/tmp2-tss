/* SPDX-License-Identifier: BSD-2-Clause */
/***********************************************************************;
 * Copyright (c) 2015 - 2018, Intel Corporation
 * All rights reserved.
 ***********************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h" // IWYU pragma: keep
#endif

#include <stddef.h>           // for size_t
#include <stdint.h>           // for uint8_t

#include "sysapi_util.h"      // for _TSS2_SYS_CONTEXT_BLOB, TPM20_Header_Out
#include "tss2_common.h"      // for TSS2_RC, TSS2_RC_SUCCESS, UINT8, TSS2_S...
#include "tss2_mu.h"          // for Tss2_MU_UINT32_Unmarshal
#include "tss2_sys.h"         // for TSS2_SYS_CONTEXT, Tss2_Sys_GetRpBuffer
#include "tss2_tpm2_types.h"  // for TPM2_HANDLE, TPM2_PARAMETER_SIZE, TPM2_...

TSS2_RC Tss2_Sys_GetRpBuffer(
    TSS2_SYS_CONTEXT *sysContext,
    size_t *rpBufferUsedSize,
    const uint8_t **rpBuffer)
{
    TSS2_SYS_CONTEXT_BLOB *ctx = syscontext_cast(sysContext);
    TSS2_RC rval;

    if (!ctx || !rpBufferUsedSize || !rpBuffer)
        return TSS2_SYS_RC_BAD_REFERENCE;

    if (ctx->previousStage != CMD_STAGE_RECEIVE_RESPONSE)
        return TSS2_SYS_RC_BAD_SEQUENCE;

    /* Calculate the position of the response parameter section within the TPM
     * response as well as its size. Structure is:
     * Header (tag, responseSize, responseCode)
     * handle(if Command has handles)
     * parameterSize (if TPM_ST_SESSIONS), size of rpArea
     * rpArea
     * Sessions (if TPM_ST_SESSIONS) */
    size_t offset = sizeof(TPM20_Header_Out); /* Skip over the header */
    offset += ctx->numResponseHandles * sizeof(TPM2_HANDLE); /* Skip handle */

    if (ctx->rsp_header.tag == TPM2_ST_SESSIONS) {
        /* If sessions are used a parameterSize values exists for convenience */
        TPM2_PARAMETER_SIZE parameterSize;
        rval = Tss2_MU_UINT32_Unmarshal(ctx->cmdBuffer,
                ctx->rsp_header.responseSize, &offset, &parameterSize);
        if (rval != TSS2_RC_SUCCESS) {
            return rval;
        }
        *rpBuffer = ctx->cmdBuffer + offset;
        *rpBufferUsedSize = parameterSize;
    } else {
        /* If no session is used the remainder is the rpArea */
        *rpBuffer = ctx->cmdBuffer + offset;
        *rpBufferUsedSize = ctx->rsp_header.responseSize - offset;
    }

    return TSS2_RC_SUCCESS;
}
