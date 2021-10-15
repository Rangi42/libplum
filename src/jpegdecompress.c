#include "proto.h"

#define MCU_ZERO_COORD 0xfd
#define MCU_NEXT_ROW   0xfe
#define MCU_END_LIST   0xff

void decompress_JPEG_Huffman_scan (struct context * context, struct JPEG_decompressor_state * restrict state, const struct JPEG_decoder_tables * tables,
                                   size_t rowunits, const struct JPEG_component_info * components, const size_t * offsets, unsigned shift, unsigned char first,
                                   unsigned char last) {
  size_t restart_interval;
  for (restart_interval = 0; restart_interval <= state -> restart_count; restart_interval ++) {
    size_t units = (restart_interval == state -> restart_count) ? state -> last_size : state -> restart_size;
    if (!units) break;
    size_t p, colcount = 0, rowcount = 0, skipcount = 0, skipunits = 0;
    const unsigned char * decodepos;
    const unsigned char * data = context -> data + *(offsets ++);
    int16_t (* outputunit)[64];
    size_t count = *(offsets ++);
    uint16_t prevDC[4] = {0, 0, 0, 0};
    int16_t nextvalue = 0;
    uint32_t dataword = 0;
    uint8_t bits = 0;
    while (units --) {
      for (decodepos = state -> MCU; *decodepos != MCU_END_LIST; decodepos ++) switch (*decodepos) {
        case MCU_ZERO_COORD:
          outputunit = state -> current[decodepos[1]];
          break;
        case MCU_NEXT_ROW:
          outputunit += state -> row_offset[decodepos[1]];
          break;
        default:
          for (p = first; p <= last; p ++) {
            if (!(skipcount || nextvalue || skipunits)) {
              unsigned char decompressed;
              if (p) {
                decompressed = next_JPEG_Huffman_value(context, &data, &count, &dataword, &bits, tables -> Huffman[components[*decodepos].tableAC + 4]);
                if (decompressed & 15)
                  skipcount = decompressed >> 4;
                else if (decompressed == 0xf0)
                  skipcount = 16;
                else
                  skipunits = (1u << (decompressed >> 4)) + shift_in_right_JPEG(context, decompressed >> 4, &dataword, &bits, &data, &count);
                decompressed &= 15;
              } else {
                decompressed = next_JPEG_Huffman_value(context, &data, &count, &dataword, &bits, tables -> Huffman[components[*decodepos].tableDC]);
                if (decompressed > 15) throw(context, PLUM_ERR_INVALID_FILE_FORMAT);
              }
              if (decompressed) {
                uint_fast16_t extrabits = shift_in_right_JPEG(context, decompressed, &dataword, &bits, &data, &count);
                if (!(extrabits >> (decompressed - 1))) nextvalue = ((int16_t) -1 << decompressed) + 1;
                nextvalue = make_signed_16(nextvalue + extrabits);
              }
            }
            if (skipcount || skipunits) {
              p[*outputunit] = 0;
              if (skipcount) skipcount --;
            } else {
              p[*outputunit] = nextvalue << shift;
              nextvalue = 0;
            }
            if (!p) prevDC[*decodepos] = **outputunit = make_signed_16(prevDC[*decodepos] + (uint16_t) **outputunit);
          }
          outputunit ++;
          if (skipunits) skipunits --;
      }
      if ((++ colcount) == rowunits) {
        colcount = 0;
        rowcount ++;
        if (rowcount == state -> row_skip_index) skipunits += (rowunits - state -> column_skip_count) * state -> row_skip_count;
      }
      if (colcount == state -> column_skip_index) skipunits += state -> column_skip_count;
      for (p = 0; p < 4; p ++) if (state -> current[p]) {
        state -> current[p] += state -> unit_offset[p];
        if (!colcount) state -> current[p] += state -> unit_row_offset[p];
      }
    }
    if (count || skipcount || skipunits || nextvalue) throw(context, PLUM_ERR_INVALID_FILE_FORMAT);
  }
}

void decompress_JPEG_Huffman_bit_scan (struct context * context, struct JPEG_decompressor_state * restrict state, const struct JPEG_decoder_tables * tables,
                                       size_t rowunits, const struct JPEG_component_info * components, const size_t * offsets, unsigned shift, unsigned char first,
                                       unsigned char last) {
  // this function is essentially the same as decompress_JPEG_Huffman_scan, but it uses already-initialized component data, and it decodes one bit at a time
  if (last && !first) throw(context, PLUM_ERR_INVALID_FILE_FORMAT);
  size_t restart_interval;
  for (restart_interval = 0; restart_interval <= state -> restart_count; restart_interval ++) {
    size_t units = (restart_interval == state -> restart_count) ? state -> last_size : state -> restart_size;
    if (!units) break;
    size_t p, colcount = 0, rowcount = 0, skipcount = 0, skipunits = 0;
    const unsigned char * decodepos;
    const unsigned char * data = context -> data + *(offsets ++);
    int16_t (* outputunit)[64];
    size_t count = *(offsets ++);
    int16_t nextvalue = 0;
    uint32_t dataword = 0;
    uint8_t bits = 0;
    while (units --) {
      for (decodepos = state -> MCU; *decodepos != MCU_END_LIST; decodepos ++) switch (*decodepos) {
        case MCU_ZERO_COORD:
          outputunit = state -> current[decodepos[1]];
          break;
        case MCU_NEXT_ROW:
          outputunit += state -> row_offset[decodepos[1]];
          break;
        default:
          if (first) {
            for (p = first; p <= last; p ++) {
              if (!(skipcount || nextvalue || skipunits)) {
                unsigned char decompressed = next_JPEG_Huffman_value(context, &data, &count, &dataword, &bits,
                                                                     tables -> Huffman[components[*decodepos].tableAC + 4]);
                if (decompressed & 15)
                  skipcount = decompressed >> 4;
                else if (decompressed == 0xf0)
                  skipcount = 16;
                else
                  skipunits = (1u << (decompressed >> 4)) + shift_in_right_JPEG(context, decompressed >> 4, &dataword, &bits, &data, &count);
                decompressed &= 15;
                if (decompressed) {
                  uint_fast16_t extrabits = shift_in_right_JPEG(context, decompressed, &dataword, &bits, &data, &count);
                  if (!(extrabits >> (decompressed - 1))) nextvalue = ((int16_t) -1 << decompressed) + 1;
                  nextvalue = make_signed_16(nextvalue + extrabits);
                }
              }
              if (p[*outputunit]) {
                if (shift_in_right_JPEG(context, 1, &dataword, &bits, &data, &count))
                  if (p[*outputunit] < 0)
                    p[*outputunit] -= 1 << shift;
                  else
                    p[*outputunit] += 1 << shift;
              } else if (skipcount || skipunits) {
                if (skipcount) skipcount --;
              } else {
                p[*outputunit] = nextvalue << shift;
                nextvalue = 0;
              }
            }
          } else if (!skipunits)
            **outputunit += shift_in_right_JPEG(context, 1, &dataword, &bits, &data, &count) << shift;
          outputunit ++;
          if (skipunits) skipunits --;
      }
      if ((++ colcount) == rowunits) {
        colcount = 0;
        rowcount ++;
        if (rowcount == state -> row_skip_index) skipunits += (rowunits - state -> column_skip_count) * state -> row_skip_count;
      }
      if (colcount == state -> column_skip_index) skipunits += state -> column_skip_count;
      for (p = 0; p < 4; p ++) if (state -> current[p]) {
        state -> current[p] += state -> unit_offset[p];
        if (!colcount) state -> current[p] += state -> unit_row_offset[p];
      }
    }
    if (count || skipcount || skipunits || nextvalue) throw(context, PLUM_ERR_INVALID_FILE_FORMAT);
  }
}

unsigned char next_JPEG_Huffman_value (struct context * context, const unsigned char ** data, size_t * restrict count, uint32_t * restrict dataword,
                                       uint8_t * restrict bits, const short * table) {
  unsigned short index = 0;
  while (1) {
    index += shift_in_right_JPEG(context, 1, dataword, bits, data, count);
    if (table[index] >= 0) return table[index];
    if (table[index] == -1) throw(context, PLUM_ERR_INVALID_FILE_FORMAT);
    index = -table[index];
  }
}

void decompress_JPEG_arithmetic_scan (struct context * context, struct JPEG_decompressor_state * restrict state, const struct JPEG_decoder_tables * tables,
                                      size_t rowunits, const struct JPEG_component_info * components, const size_t * offsets, unsigned shift, unsigned char first,
                                      unsigned char last) {
  // ...
}

void decompress_JPEG_arithmetic_bit_scan (struct context * context, struct JPEG_decompressor_state * restrict state, const struct JPEG_decoder_tables * tables,
                                          size_t rowunits, const struct JPEG_component_info * components, const size_t * offsets, unsigned shift,
                                          unsigned char first, unsigned char last) {
  if (last && !first) throw(context, PLUM_ERR_INVALID_FILE_FORMAT);
  // ...
}

void initialize_JPEG_decompressor_state (struct context * context, struct JPEG_decompressor_state * restrict state, const struct JPEG_component_info * components,
                                         const unsigned char * componentIDs, size_t * restrict unitsH, size_t unitsV, size_t width, size_t height,
                                         unsigned char maxH, unsigned char maxV, const struct JPEG_decoder_tables * tables, const size_t * offsets,
                                         int16_t (* restrict * output)[64]) {
  size_t p;
  for (p = 0; p < 4; p ++) state -> current[p] = NULL;
  if (componentIDs[1] != 0xff) {
    uint_fast8_t row, col;
    unsigned char * entry = state -> MCU;
    for (p = 0; (p < 4) && (componentIDs[p] != 0xff); p ++) {
      state -> unit_offset[componentIDs[p]] = components[componentIDs[p]].scaleH;
      state -> row_offset[componentIDs[p]] = *unitsH * state -> unit_offset[componentIDs[p]];
      state -> unit_row_offset[componentIDs[p]] = (components[componentIDs[p]].scaleV - 1) * state -> row_offset[componentIDs[p]];
      state -> row_offset[componentIDs[p]] -= state -> unit_offset[componentIDs[p]];
      for (row = 0; row < components[componentIDs[p]].scaleV; row ++) {
        *(entry ++) = row ? MCU_NEXT_ROW : MCU_ZERO_COORD;
        for (col = 0; col < components[componentIDs[p]].scaleH; col ++) *(entry ++) = componentIDs[p];
      }
      state -> current[componentIDs[p]] = output[componentIDs[p]];
    }
    *entry = MCU_END_LIST;
    state -> component_count = p;
    state -> row_skip_index = state -> row_skip_count = state -> column_skip_index = state -> column_skip_count = 0;
  } else {
    // if a scan contains a single component, it's considered a non-interleaved scan and the MCU is a single 8x8 block
    state -> component_count = 1;
    state -> unit_offset[*componentIDs] = 1;
    state -> row_offset[*componentIDs] = state -> unit_row_offset[*componentIDs] = 0;
    memcpy(state -> MCU, (unsigned char []) {MCU_ZERO_COORD, *componentIDs, MCU_END_LIST}, 3);
    *unitsH *= components[*componentIDs].scaleH;
    unitsV *= components[*componentIDs].scaleV;
    state -> current[*componentIDs] = output[*componentIDs];
    state -> column_skip_index = 1 + (width * components[*componentIDs].scaleH - 1) / (8 * maxH);
    state -> column_skip_count = *unitsH - state -> column_skip_index;
    state -> row_skip_index = 1 + (height * components[*componentIDs].scaleV - 1) / (8 * maxV);
    state -> row_skip_count = unitsV - state -> row_skip_index;
  }
  state -> last_size = *unitsH * unitsV;
  if (state -> restart_size = tables -> restart) {
    state -> restart_count = state -> last_size / state -> restart_size;
    state -> last_size %= state -> restart_size;
  } else
    state -> restart_count = 0;
  for (p = 0; p < state -> restart_count; p ++) if (!offsets[2 * p]) throw(context, PLUM_ERR_INVALID_FILE_FORMAT);
  if (state -> last_size && !offsets[2 * (p ++)]) throw(context, PLUM_ERR_INVALID_FILE_FORMAT);
  if (offsets[2 * p]) throw(context, PLUM_ERR_INVALID_FILE_FORMAT);
}

#undef MCU_ZERO_COORD
#undef MCU_NEXT_ROW
#undef MCU_END_LIST
