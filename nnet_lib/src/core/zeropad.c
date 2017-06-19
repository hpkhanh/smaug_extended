#include "nnet_fwd.h"

#include "zeropad.h"

// Zeropad each image in @a by @pad zeros.
//
// a is a 4D matrix of flattened input feature maps.
//
// The result is placed in @result, which is an array that is assumed to be
// large enough for this operation.
void copy_zeropad(float* a, layer_t curr_layer, int pad, float* result) {
    int ni;

    // Yes, "output_rows" and "output_cols". "input_rows" and "input_cols" is
    // the dimensions of the data AFTER zeropadding because this is considered
    // as the "input" to the convolution itself.
    int a_rows = curr_layer.output_rows;
    int a_cols = curr_layer.output_cols;
    int a_height = curr_layer.input_height;
    int a_data_pad = curr_layer.output_data_align_pad;
    int r_rows = curr_layer.input_rows;
    int r_cols = curr_layer.input_cols;
    int r_data_pad = curr_layer.input_data_align_pad;

copy_zeropad_per_image:
    for (ni = 0; ni < NUM_TEST_CASES; ni++) {
        copy_zeropad_image3d(a, pad, ni, a_rows, a_cols, a_height, a_data_pad,
                             result, r_rows, r_cols, r_data_pad);
    }
}

void copy_zeropad_image3d(float* a,
                          int pad,
                          int img,
                          int a_rows,
                          int a_cols,
                          int a_hgt,
                          int a_data_pad,
                          float* result,
                          int r_rows,
                          int r_cols,
                          int r_data_pad) {
    int h, i, j;

    ARRAY_4D(float, _a, a, a_hgt, a_rows, a_cols + a_data_pad);
    ARRAY_4D(float, _result, result, a_hgt, r_rows, r_cols + r_data_pad);

    copy_zeropad_height:
    for (h = 0; h < a_hgt; h++) {
        copy_zeropad_first_rows:
        for (i = 0; i < pad; i++) {
            copy_zeropad_first_cols:
            for (j = 0; j < r_cols + r_data_pad; j++) {
                _result[img][h][i][j] = 0;
            }
        }

        copy_zeropad_left:
        for (i = pad; i < a_rows + pad; i++) {
            copy_zeropad_left_cols:
            for (j = 0; j < pad; j++) {
                _result[img][h][i][j] = 0;
            }
            // Copy the original array.
            copy_zeropad_copy_cols:
            for (j = pad; j < a_cols + pad; j++) {
                _result[img][h][i][j] = _a[img][h][i-pad][j-pad];
            }
            copy_zeropad_right_cols:
            for (j = a_cols + pad; j < r_cols; j++) {
                _result[img][h][i][j] = 0;
            }
            copy_zeropad_data_pad:
            for (j = r_cols; j < r_cols + r_data_pad; j++) {
                _result[img][h][i][j] = 0;
            }
        }

        copy_zeropad_last:
        for (i = a_rows + pad; i < r_rows; i++) {
            copy_zeropad_last_cols:
            for (j = 0; j < r_cols + r_data_pad; j++) {
                _result[img][h][i][j] = 0;
            }
        }
    }
}
