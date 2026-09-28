// SPDX-License-Identifier: Unlicense OR MIT
/* Minimal AAC decoding surface for the host over the Fraunhofer FDK AAC
 * decoder: configure it with an AudioSpecificConfig, feed it the raw access
 * units of an MP4 audio track, read back 16-bit samples.
 *
 * Everything the host is allowed to do with this module goes through these
 * exports. The module has no main and no reason to touch the outside world. */
#include <stdint.h>
#include <stdlib.h>

#include "aacdecoder_lib.h"

/* The most samples one access unit decodes to: 2048 frames of 8 channels. */
#define AACW_MAX (2048 * 8)

extern "C" {

__attribute__((export_name("aacw_alloc"))) void *aacw_alloc(size_t size) { return malloc(size); }

__attribute__((export_name("aacw_dealloc"))) void aacw_dealloc(void *ptr) { free(ptr); }

/* A decoder of raw access units, configured by asc; NULL on failure. */
__attribute__((export_name("aacw_open"))) HANDLE_AACDECODER aacw_open(UCHAR *asc, UINT size) {
  HANDLE_AACDECODER d = aacDecoder_Open(TT_MP4_RAW, 1);
  if (d == NULL) return NULL;
  if (aacDecoder_ConfigRaw(d, &asc, &size) != AAC_DEC_OK) {
    aacDecoder_Close(d);
    return NULL;
  }
  return d;
}

__attribute__((export_name("aacw_close"))) void aacw_close(HANDLE_AACDECODER d) {
  if (d != NULL) aacDecoder_Close(d);
}

/* Decodes one access unit into pcm, which holds AACW_MAX samples,
 * interleaved. Returns the samples put out, all channels counted, or a
 * negative AAC_DECODER_ERROR. */
__attribute__((export_name("aacw_decode"))) int aacw_decode(HANDLE_AACDECODER d, UCHAR *in, UINT size, INT_PCM *pcm) {
  UINT valid = size;
  AAC_DECODER_ERROR err = aacDecoder_Fill(d, &in, &size, &valid);
  if (err != AAC_DEC_OK) return -(int)err;
  err = aacDecoder_DecodeFrame(d, pcm, AACW_MAX, 0);
  if (err != AAC_DEC_OK) return -(int)err;
  CStreamInfo *info = aacDecoder_GetStreamInfo(d);
  if (info == NULL) return 0;
  return info->frameSize * info->numChannels;
}

/* The stream's output, known once an access unit decoded. */
__attribute__((export_name("aacw_rate"))) int aacw_rate(HANDLE_AACDECODER d) {
  CStreamInfo *info = aacDecoder_GetStreamInfo(d);
  return info ? info->sampleRate : 0;
}
__attribute__((export_name("aacw_channels"))) int aacw_channels(HANDLE_AACDECODER d) {
  CStreamInfo *info = aacDecoder_GetStreamInfo(d);
  return info ? info->numChannels : 0;
}

/* Forgets the units decoded, before decoding from another point. */
__attribute__((export_name("aacw_flush"))) void aacw_flush(HANDLE_AACDECODER d) {
  aacDecoder_SetParam(d, AAC_TPDEC_CLEAR_BUFFER, 1);
}

}
