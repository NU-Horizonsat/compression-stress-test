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
    const size_t out_w = 64;
    const size_t out_h = 64;
    const int stages = 4;
    const enum icer_filter_types filt = ICER_FILTER_A;
    const int segments = 6;

    const int datastream_size = 1000;

    int src_w, src_h, n;
    uint8_t *data;

    uint8_t *resized = malloc(out_w*out_h);
    uint16_t *compress = malloc(out_w*out_h*2);
   
    int res = 0;
    clock_t begin, end;

    icer_init();

    printf("test compression code\n");
    printf("loading image: \"%s\"\n", filename);
    printf("Avail Mem:  %u", getFreeHeap());
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
    printf("Avail Mem:  %u\n", getFreeHeap());
    printf("resize complete\n");

    printf("converting to int16\n");
    for (size_t i = 0;i < out_h*out_w;i++) {
        compress[i] = resized[i];
    }
    free(resized);
    
    printf("Avail Mem:  %u", getFreeHeap());
    stbi_image_free(data);
    uint8_t *datastream = malloc(datastream_size*2+500);
    printf("Probably this malloc is to blame?\n");
    icer_output_data_buf_typedef output;
    icer_init_output_struct(&output, datastream, datastream_size*2, datastream_size);

    printf("Avail Mem:  %u", getFreeHeap());
    begin = clock();
    icer_compress_image_uint16(compress, out_w, out_h, stages, filt, segments, &output);
    sleep_ms(1000);
    end = clock();
    printf("Where is the panic?...\n");

    printf("compressed size %u, time taken: %lf\n", output.size_used, (float)(end-begin)/CLOCKS_PER_SEC);

    FIL fil;
    f_open(&fil, compressed_filename, FA_CREATE_ALWAYS | FA_WRITE);
    UINT bw;
    f_write(&fil, output.rearrange_start, sizeof(output.rearrange_start[0]) * output.size_used, &bw);
    printf("written: %u\n", bw);
    //fflush(ptr1);
    f_close(&fil);

    printf("output saved\n");

    free(compress);
    free(datastream);
    return 0;
}