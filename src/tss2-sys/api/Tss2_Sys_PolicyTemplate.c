/* SPDX-License-Identifier: BSD-2-Clause */
/***********************************************************************;
 * Copyright (c) 2018, Intel Corporation
 * All rights reserved.
 ***********************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h" // IWYU pragma: keep
#endif

#include "sysapi_util.h"      // for _TSS2_SYS_CONTEXT_BLOB, syscontext_cast
#include "tss2_common.h"      // for TSS2_RC, TSS2_SYS_RC_BAD_REFERENCE
#include "tss2_mu.h"          // for Tss2_MU_TPM2B_DIGEST_Marshal, Tss2_MU_T...
#include "tss2_sys.h"         // for TSS2_SYS_CONTEXT, TSS2L_SYS_AUTH_COMMAND
#include "tss2_tpm2_types.h"  // for TPM2B_DIGEST, TPMI_SH_POLICY, TPM2_CC_P...

TSS2_RC Tss2_Sys_PolicyTemplate_Prepare(
    TSS2_SYS_CONTEXT *sysContext,
    TPMI_SH_POLICY policySession,
    const TPM2B_DIGEST *templateHash)
{
    TSS2_SYS_CONTEXT_BLOB *ctx = syscontext_cast(sysContext);
    TSS2_RC rval;

    if (!ctx)
        return TSS2_SYS_RC_BAD_REFERENCE;

    rval = CommonPreparePrologue(ctx, TPM2_CC_PolicyTemplate);
    if (rval)
        return rval;

    rval = Tss2_MU_TPM2_HANDLE_Marshal(policySession, ctx->cmdBuffer,
                                       ctx->maxCmdSize,
                                       &ctx->nextData);
    if (rval)
        return rval;

    if (!templateHash) {
        rval = Tss2_MU_UINT16_Marshal(0, ctx->cmdBuffer,
                                      ctx->maxCmdSize,
                                      &ctx->nextData);

    } else {

        rval = Tss2_MU_TPM2B_DIGEST_Marshal(templateHash, ctx->cmdBuffer,
                                            ctx->maxCmdSize,
                                            &ctx->nextData);
    }

    if (rval)
        return rval;

    ctx->decryptAllowed = 1;
    ctx->encryptAllowed = 0;
    ctx->authAllowed = 1;

    return CommonPrepareEpilogue(ctx);
}

TSS2_RC Tss2_Sys_PolicyTemplate_Complete(
    TSS2_SYS_CONTEXT *sysContext)
{
    TSS2_SYS_CONTEXT_BLOB *ctx = syscontext_cast(sysContext);

    if (!ctx)
        return TSS2_SYS_RC_BAD_REFERENCE;

    return CommonComplete(ctx);
}

TSS2_RC Tss2_Sys_PolicyTemplate(
    TSS2_SYS_CONTEXT *sysContext,
    TPMI_SH_POLICY policySession,
    TSS2L_SYS_AUTH_COMMAND const *cmdAuthsArray,
    const TPM2B_DIGEST *templateHash,
    TSS2L_SYS_AUTH_RESPONSE *rspAuthsArray)
{
    TSS2_SYS_CONTEXT_BLOB *ctx = syscontext_cast(sysContext);
    TSS2_RC rval;

    rval = Tss2_Sys_PolicyTemplate_Prepare(sysContext, policySession,
                                           templateHash);
    if (rval)
        return rval;

    rval = CommonOneCall(ctx, cmdAuthsArray, rspAuthsArray);
    if (rval)
        return rval;

    return Tss2_Sys_PolicyTemplate_Complete(sysContext);
}
