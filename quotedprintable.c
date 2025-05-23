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

int qp_decode(char *in, char *out, char **badchar)
{
    enum {
        NORMAL,
        EQUAL,
        HEX2,
        CONTINUATION,
    } state = NORMAL;

    int hex;
    (void)badchar; // TODO

    while (*in) {
        switch (state) {
            case NORMAL:
                if (is_qp_normal(*in)) {
                    state = NORMAL;
                    *(out++) = *(in++);
                    break;
                }
                else if (*in == '=') {
                    state = EQUAL;
                    in++;
                }
                // TODO else badchar
                break;

            case EQUAL:
                if (*in == '\n' || *in == '\r') {
                    state = CONTINUATION;
                    in++;
                }
                else if (is_hex_digit(*in)) {
                    state = HEX2;
                    hex = hex_char_value(*in);
                    in++;
                }
                // TODO else badchar
                break;

            case HEX2:
                if (is_hex_digit(*in)) {
                    state = NORMAL;
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
                    state = CONTINUATION;
                    in++;
                }

                else if (*in == '=') {
                    state = EQUAL;
                    in++;
                }

                else if (is_qp_normal(*in)) {
                    state = NORMAL;
                    *(out++) = *(in++);
                    break;
                }
                // TODO else badchar
                break;
        }
    } // while (*in)

    return 0;
}

char *sample_1 = "a=09bc=20def=2E=\r\ncontinue\nnewline";
char *sample_2 = 
    "Those=20facilities=20were=20NEVER=20properly=20funded=20and=20beca=\n"
    "me=20nothing=20more=20than=20poorly=20managed=20prisons.Most=20of=\n"
    "=20the=20lunatics=20on=20the=20streets=20now=20are=20severely=20ad=\n"
    "dicted=20people=20that=20cannot=20and=20woulod=20not=20be=20cured=\n"
    "=20of=20their=20addiction.=20Fentanyl=20will=20kill=20them=20and=\n"
    "=20therw's=20no=20way=20to=20stop=20it.\n";

int main(void)
{
    char out[4096];

    qp_decode(sample_1, out, NULL);

    printf("%s\n", out);
}
