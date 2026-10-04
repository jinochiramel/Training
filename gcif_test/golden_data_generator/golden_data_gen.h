#ifndef PATGEN_H_
#define PATGEN_H_

#include <stdint.h>
#include <stdio.h>

/* BYTE SWAPPING MACROS */

#define BYTE_SWAP_64(data) \
    ( \
      (((data) & 0x00000000000000ffll) << 56) \
    | (((data) & 0x000000000000ff00ll) << 40) \
    | (((data) & 0x0000000000ff0000ll) << 24) \
    | (((data) & 0x00000000ff000000ll) <<  8) \
    | (((data) & 0x000000ff00000000ll) >>  8) \
    | (((data) & 0x0000ff0000000000ll) >> 24) \
    | (((data) & 0x00ff000000000000ll) >> 40) \
    | (((data) & 0xff00000000000000ll) >> 56) \
    )

#define BYTE_SWAP_32(data) \
    ( \
      (((data) & 0x000000ff) << 24) \
    | (((data) & 0x0000ff00) <<  8) \
    | (((data) & 0x00ff0000) >>  8) \
    | (((data) & 0xff000000) >> 24) \
    )

#define BYTE_SWAP_16(data) \
   ( \
     (((data) & 0x00FF) << 8) \
   | (((data) & 0xFF00) >> 8) \
   )

#define ENDIAN_LE64 BYTE_SWAP_64
#define ENDIAN_LE32 BYTE_SWAP_32
#define ENDIAN_LE16 BYTE_SWAP_16

#define ENDIAN_BE64 BYTE_SWAP_64
#define ENDIAN_BE32 BYTE_SWAP_32
#define ENDIAN_BE16 BYTE_SWAP_16

typedef struct _das_param
{
    uint16_t  das_channels;         // channels per row (current value)
    uint8_t   rows_per_view;        // rows_per_view
    uint8_t   das_rows;             // cpld_rows (16 or 16|4)
    uint8_t   view_starting_row;    // view starting row
    uint8_t   views_per_tx;         // no:of views per transfer;das internal
    uint8_t   flags;                // simulation or das scan; 1 for simulation
    uint32_t  views_per_scan;       // no:of views in a scan
    uint32_t  views_per_sec;        // no:of views in a second
}das_param_t;

//<1000 SW filled
/***************************************************************************
 * name: low_data_rate_fllbuf16
 * parameter:
 *      dest_buf_ptr:   pointer to buffer to place generated data
 *      view_num:       View Num to be generated
 *      num_segs:       no:of rows; usually 16
 *      das_channels:   no:of das channels
 * function: Generate simulated  data for data rate <1000/sec
 * **************************************************************************/
int32_t low_data_rate_fillbuf16(uint8_t *dest_buf_ptr,
                              uint64_t view_num,
                              uint8_t num_segs,
                              uint32_t das_channels);

/***************************************************************************
 * name: low_data_rate_fllbuf4
 * parameter:
 *      dest_buf_ptr:   pointer to buffer to place generated data
 *      view_num:       View Num to be generated
 *      num_segs:       no:of rows; usually 4
 *      das_channels:   no:of das channels
 * function: Generate simulated  data for data rate <1000/sec
 * **************************************************************************/
int32_t low_data_rate_fillbuf4(uint8_t *dest_buf_ptr,
                              uint64_t view_num,
                              uint8_t num_segs,
                              uint32_t das_channels);

//>1000 FPGA filled
/***************************************************************************
 * name: high_data_rate_fillbuf16
 * parameter:
 *      dest_buf_ptr:   pointer to buffer to place generated data
 *      view_num:       View Num to be generated
 *      num_segs:       no:of rows; usually 16
 *      das_channels:   no:of das channels
 * function: Generate simulated  data for data rate >1000/sec
 * **************************************************************************/
int32_t high_data_rate_fillbuf16(uint8_t *dest_buf_ptr,
                               uint64_t view_num,
                               uint8_t num_segs,
                               uint32_t das_channels);

/***************************************************************************
 * name: high_data_rate_fillbuf4
 * parameter:
 *      dest_buf_ptr:   pointer to buffer to place generated data
 *      view_num:       View Num to be generated
 *      num_segs:       no:of rows; usually 4
 *      das_channels:   no:of das channels
 * function: Generate simulated  data for data rate >1000/sec
 * **************************************************************************/
int32_t high_data_rate_fillbuf4(uint8_t *dest_buf_ptr,
                               uint64_t view_num,
                               uint8_t num_segs,
                               uint32_t das_channels);

/***************************************************************************
 * name: pad_view
 * parameter:
 *      buff_addr:   pointer to buffer to place generated data
 *      num_segs:       no:of rows; usually 4 or 16
 *      das_channels:   no:of das channels
 * function: pad val 0x55aa at the end of each row in the generated data
 * **************************************************************************/
int32_t pad_view(uint16_t *buff_addr, uint32_t num_segs, uint32_t das_channels);

/***************************************************************************
 * name: create_golden_data_file
 * parameter:
 *      file_name:  file name of the golden data
 *      desc_str:   Header description string
 *      buf_ptr:    Generated data buffer pointer
 *      buf_size:   Size of the generated data
 * function: Create golden data file using formatted output for single view
 * **************************************************************************/
int32_t create_golden_data_file(char *file_name, char *desc_str, 
                                uint8_t *buf_ptr, uint64_t buf_size);

/***************************************************************************
 * name: create_golden_data_binfile
 * parameter:
 *      file_name:  file name of the golden data binary file
 *      buf_ptr:    Generated data buffer pointer
 *      buf_size:   Size of the generated data
 * function: Create golden data file for single view
 * **************************************************************************/
int32_t create_golden_data_binfile(char *file_name, uint8_t *buf_ptr, 
                                    uint64_t buf_size);

/***************************************************************************
 * name: get_gcif_config
 * parameter:
 *      das_config:     das configuration parameters for the test   
 * function: This function will read the configuration parameters 
 *           from the gcif.cfg file
 * **************************************************************************/
int32_t get_gcif_config(das_param_t *das_config);

/***************************************************************************
 * name: show_gcif_config
 * parameter:
 *      das_config:     das configuration parameters for the test   
 * function: This function will display the configuration parameters read
 *           from the gcif.cfg file
 * **************************************************************************/
void show_gcif_config(das_param_t *das_config);


#endif /*PATGEN_H_*/
