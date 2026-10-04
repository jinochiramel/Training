#ifndef PATGEN_H_
#define PATGEN_H_

#include <stdint.h>
#include "scsi_test.h"

typedef struct _sim_data_info
{
    uint8_t *sim_data_buf;
    uint64_t sim_data_size;
}sim_data_info_t;

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
 * name: create_simulation_data_buf
 * parameter:
 *      rows_per_view: no:of rows in a view; 16 or 4
 *      das_channels:  no:of das channels
 * function: Create simulated data and store in memory
 * **************************************************************************/
sim_data_info_t *create_simulation_data_buf(uint8_t rows_per_view,
                                            uint16_t das_channels);

/***************************************************************************
 * name: destroy_simulation_data
 * parameter:
 *      p_sim_data: Pointer needs to be free
 * function: Deleting the simulation data pointer
 * **************************************************************************/
void destroy_simulation_data(sim_data_info_t *p_sim_data);

/***************************************************************************
 * name: show_generated_data
 * parameter:
 *      buf_ptr: buffer pointer
 *      buf_size: size of the buffer
 * function: Display the generated data on the console screen.
 * **************************************************************************/
void show_generated_data(uint8_t *buf_ptr, uint64_t buf_size);

/***************************************************************************
 * name: get_golden_data
 * parameter:
 *      golden_data_ptr: golden data buffer pointer
 *      data_size: size of the golden data
 *      view_num: view number
 *      das_config_param: DAS configuration parameters
 * function: Get the golden data for the specifed view and other
 *           DAS configuation values.
 * **************************************************************************/
int32_t get_golden_data(uint8_t *golden_data_ptr,
                        uint64_t data_size,
                        uint32_t view_num,
                        das_param_t *das_config_param);

/***************************************************************************
 * name: verify_gcif_data
 * parameter:
 *      das_data_ptr: das data buffer pointer
 *      das_data_size: size of the das data
 *      das_config_param: DAS configuration parameters
 * function: Verify the scan data came from the DAS with golden data
 * **************************************************************************/
int32_t verify_gcif_data(uint8_t *das_data_ptr, uint64_t das_data_size,
                         das_param_t *das_config_param);

//for test stub
void dummy_das_scan_data(uint8_t *das_data_ptr,
                        uint32_t views_per_scan, uint8_t rows_per_view,
                        uint32_t das_channels, uint32_t views_per_sec);

#endif /*PATGEN_H_*/
