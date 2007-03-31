#include "mtypes.h"
#include "mproto.h"

static int qtable[][8] = {
    { 8,16,19,22,26,27,29,34},
    {16,16,22,24,27,29,34,37},
    {19,22,26,27,29,34,34,38},
    {22,22,26,27,29,34,37,40},
    {22,26,27,29,32,35,40,48},
    {26,27,29,32,35,40,48,58},
    {26,27,29,34,38,46,56,69},
    {27,29,35,38,46,56,69,83} };


/*
 *--------------------------------------------------------------
 *
 * mp_quant_block --
 *
 *    Quantizes a block -- removing information
 *    It's safe for out == in.
 *
 * Results:
 *    None.
 *
 * Side effects:
 *    Modifies the out block.
 *
 *--------------------------------------------------------------
 */
void mp_quant_block(Block in, Block out) {
    int i;

    for(i=0;i<8;i++) {
    out[i][0] = in[i][0] / qtable[i][0];
    out[i][1] = in[i][1] / qtable[i][1];
    out[i][2] = in[i][2] / qtable[i][2];
    out[i][3] = in[i][3] / qtable[i][3];
    out[i][4] = in[i][4] / qtable[i][4];
    out[i][5] = in[i][5] / qtable[i][5];
    out[i][6] = in[i][6] / qtable[i][6];
    out[i][7] = in[i][7] / qtable[i][7];
    }
}
