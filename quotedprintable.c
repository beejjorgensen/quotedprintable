#include <stdio.h>
#include <ctype.h>

// Unlicensed by Beej Jorgensen <beej@beej.us>
//
// This is free and unencumbered software released into the public
// domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the
// benefit of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <https://unlicense.org/>

enum state {
    NORMAL,
    EQUAL,
    HEX2,
    CONTINUATION,
    ERROR,
};

struct decoder {
    char *in;
    char *out;
    char *badchar;
    size_t input_size;
    size_t output_size;
    enum state state;
};

static int is_qp_normal(int c)
{
    return (c >= 33 && c <= 126 && c !='=') || isspace(c);
}

static int is_hex_digit(int c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F');
}

static int hex_char_value(int c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

char *qp_decode(struct decoder *decoder)
{
    int hex;

    char *in = decoder->in;
    char *out = decoder->out;

    while (in < decoder->in + decoder->input_size) {
        switch (decoder->state) {
            case NORMAL:
                if (is_qp_normal(*in)) {
                    decoder->state = NORMAL;
                    *(out++) = *(in++);
                    break;
                }
                else if (*in == '=') {
                    decoder->state = EQUAL;
                    in++;
                }
                // TODO else badchar
                break;

            case EQUAL:
                if (*in == '\n' || *in == '\r') {
                    decoder->state = CONTINUATION;
                    in++;
                }
                else if (is_hex_digit(*in)) {
                    decoder->state = HEX2;
                    hex = hex_char_value(*in);
                    in++;
                }
                // TODO else badchar
                break;

            case HEX2:
                if (is_hex_digit(*in)) {
                    decoder->state = NORMAL;
                    hex = (hex << 4) | hex_char_value(*in);
                    *(out++) = hex;
                    in++;

                }
                // TODO else badchar
                break;

            case CONTINUATION:
                // TODO: could this fail if we continue directly into a
                // newline? Would that happen? Of course it would.
                if (*in == '\n' || *in == '\r') {
                    decoder->state = CONTINUATION;
                    in++;
                }

                else if (*in == '=') {
                    decoder->state = EQUAL;
                    in++;
                }

                else if (is_qp_normal(*in)) {
                    decoder->state = NORMAL;
                    *(out++) = *(in++);
                    break;
                }
                // TODO else badchar
                break;

            case ERROR:
                // TODO
                break;
        }
    } // while (*in)

    decoder->output_size = out - decoder->out;

    return 0;
}

char *sample_1 = "a=09bc=20def=2E=\r\ncontinue\nnewline";

#define BLOCK_SIZE 4096

int main(void)
{
    struct decoder d;
    char inbuf[BLOCK_SIZE], outbuf[BLOCK_SIZE];

    d.in = inbuf;
    d.out = outbuf;
    d.state = NORMAL;

    size_t bytes_read;

    while ((bytes_read = fread(d.in, 1, BLOCK_SIZE, stdin)) > 0) {
        d.input_size = bytes_read;
        qp_decode(&d);
        fwrite(d.out, 1, d.output_size, stdout);
    }
}
