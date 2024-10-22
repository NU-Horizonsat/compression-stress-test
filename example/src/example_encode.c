#include <stdio.h>
#include <string.h>
#include <time.h>

#include <stdlib.h>
#include <malloc.h>

#include "stb_image.h"
#include "stb_image_resize.h"
#include "pico/stdlib.h"


#define USE_ENCODE_FUNCTIONS
#define USE_UINT16_FUNCTIONS

#include "icer.h"
#include "ff.h"

const char compressed_filename[] = "image2.bin";
const char filename[] = "white.bmp";

int example_compression_function() {
    const size_t out_w = 128;
    const size_t out_h = 128;
    const int stages = 4;
    const enum icer_filter_types filt = ICER_FILTER_A;
    const int segments = 6;

    const int datastream_size = 3000;

    int src_w, src_h, n;
    uint8_t *data;
    uint8_t *resized = malloc(out_w*out_h);
    uint16_t *compress = malloc(out_w*out_h*2);

    icer_init();
  
    int res = 0;

    data = stbi_load(filename, &src_w, &src_h, &n, 1);
    if (data == NULL) {
        printf("invalid image\nexiting...\n");
        return 0;
    }

    printf("loaded image\nwidth    : %5d\nheight   : %5d\nchannels : %5d\n", src_w, src_h, n);
    printf("resizing image to width: %4u, height: %4u\n", out_w, out_h);

    res = stbir_resize_uint8(data, src_w, src_h, 0,
                             resized, out_w, out_h, 0,
                             1);
    if (res == 0) {
        printf("resize failed\nexiting...\n");
        return 0;
    }
    printf("resize complete\n");

    printf("converting to int16\n");
    for (size_t i = 0;i < out_h*out_w;i++) {
        compress[i] = resized[i];
    }
    
    uint8_t *datastream = malloc(datastream_size*2+500);
    
    icer_output_data_buf_typedef output;
    icer_init_output_struct(&output, datastream, datastream_size*2, datastream_size);

    uint64_t begin = time_us_64();  // Start time in microseconds
    printf("The error is %u\n", icer_compress_image_uint16(compress, out_w, out_h, stages, filt, segments, &output));
    uint64_t end = time_us_64();    // End time in microseconds

    float time_taken_ms = (end - begin) / 1000.0;  // Convert to milliseconds
    printf("compressed size %u, time taken: %f ms\n", output.size_used, time_taken_ms);

    FIL fil;
    f_open(&fil, compressed_filename, FA_CREATE_ALWAYS | FA_WRITE);
    UINT bw;
    f_write(&fil, output.rearrange_start, sizeof(output.rearrange_start[0]) * output.size_used, &bw);
    printf("written: %u\n", bw);
    printf("Size used: %u\n", output.size_used);

    f_close(&fil);
    free(resized);
    free(compress);
    free(datastream);
    stbi_image_free(data);

    printf("Successful Exit\n");
    return 0;
}