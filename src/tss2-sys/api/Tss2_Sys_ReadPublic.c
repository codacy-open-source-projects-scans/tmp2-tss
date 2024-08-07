/* SPDX-License-Identifier: BSD-2-Clause */
/***********************************************************************;
 * Copyright (c) 2015 - 2017, Intel Corporation
 * All rights reserved.
 ***********************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h" // IWYU pragma: keep
#endif

#include "sysapi_util.h"      // for _TSS2_SYS_CONTEXT_BLOB, syscontext_cast
#include "tss2_common.h"      // for TSS2_RC, TSS2_SYS_RC_BAD_REFERENCE
#include "tss2_mu.h"          // for Tss2_MU_TPM2B_NAME_Unmarshal, Tss2_MU_T...
#include "tss2_sys.h"         // for TSS2_SYS_CONTEXT, TSS2L_SYS_AUTH_COMMAND
#include "tss2_tpm2_types.h"  // for TPM2B_NAME, TPM2B_PUBLIC, TPMI_DH_OBJECT

TSS2_RC Tss2_Sys_ReadPublic_Prepare(
    TSS2_SYS_CONTEXT *sysContext,
    TPMI_DH_OBJECT objectHandle)
{
    TSS2_SYS_CONTEXT_BLOB *ctx = syscontext_cast(sysContext);
    TSS2_RC rval;

    if (!ctx)
        return TSS2_SYS_RC_BAD_REFERENCE;

    rval = CommonPreparePrologue(ctx, TPM2_CC_ReadPublic);
    if (rval)
        return rval;

    rval = Tss2_MU_UINT32_Marshal(objectHandle, ctx->cmdBuffer,
                                  ctx->maxCmdSize,
                                  &ctx->nextData);
    if (rval)
        return rval;

    ctx->decryptAllowed = 0;
    ctx->encryptAllowed = 1;
    ctx->authAllowed = 1;

    return CommonPrepareEpilogue(ctx);
}

TSS2_RC Tss2_Sys_ReadPublic_Complete(
    TSS2_SYS_CONTEXT *sysContext,
    TPM2B_PUBLIC *outPublic,
    TPM2B_NAME *name,
    TPM2B_NAME *qualifiedName)
{
    TSS2_SYS_CONTEXT_BLOB *ctx = syscontext_cast(sysContext);
    TSS2_RC rval;

    if (!ctx)
        return TSS2_SYS_RC_BAD_REFERENCE;

    rval = CommonComplete(ctx);
    if (rval)
        return rval;

    rval = Tss2_MU_TPM2B_PUBLIC_Unmarshal(ctx->cmdBuffer,
                                          ctx->maxCmdSize,
                                          &ctx->nextData, outPublic);
    if (rval)
        return rval;

    rval = Tss2_MU_TPM2B_NAME_Unmarshal(ctx->cmdBuffer,
                                        ctx->maxCmdSize,
                                        &ctx->nextData, name);
    if (rval)
        return rval;

    return Tss2_MU_TPM2B_NAME_Unmarshal(ctx->cmdBuffer,
                                        ctx->maxCmdSize,
                                        &ctx->nextData, qualifiedName);
}

TSS2_RC Tss2_Sys_ReadPublic(
    TSS2_SYS_CONTEXT *sysContext,
    TPMI_DH_OBJECT objectHandle,
    TSS2L_SYS_AUTH_COMMAND const *cmdAuthsArray,
    TPM2B_PUBLIC *outPublic,
    TPM2B_NAME *name,
    TPM2B_NAME *qualifiedName,
    TSS2L_SYS_AUTH_RESPONSE *rspAuthsArray)
{
    TSS2_SYS_CONTEXT_BLOB *ctx = syscontext_cast(sysContext);
    TSS2_RC rval;

    rval = Tss2_Sys_ReadPublic_Prepare(sysContext, objectHandle);
    if (rval)
        return rval;

    rval = CommonOneCall(ctx, cmdAuthsArray, rspAuthsArray);
    if (rval)
        return rval;

    return Tss2_Sys_ReadPublic_Complete(sysContext, outPublic, name, qualifiedName);
}
