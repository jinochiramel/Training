#ifndef SCSI_TEST_H_
#define SCSI_TEST_H_

#include <scsi/sg.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>

typedef struct _das_info
{
    uint16_t    struct_code;
    uint16_t    struct_size;
    uint32_t    sft_version;
    uint8_t     sft_type;
    uint8_t     sft_dnload_scsi_op_code;
    /* below is valid only for GCIFA-D application */
    uint16_t    das_channels;           // channels per row (current value)
    uint8_t     view_starting_row;      // reserved1
    uint8_t     rows_per_view;          // rows_per_view
    uint8_t     das_rows;               // cpld_rows (16 or 16|4)
}das_info_t;

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

typedef struct _peripheral_device
{
    uint8_t device_code;
    char* device_type;
}peripheral_device_t;

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

#define SENSE_LEN               0xFF
#define DATA_LEN                0xFF
#define BLOCK_LEN               32
#define SCSI_TIMEOUT            20000
#define GCIFAD_FW_SIGNATURE     "download_sw"

#define SCSI_TEST_UNIT_READY    0x00
#define SCSI_INQUIRY_CMD        0x12
#define SCSI_REQUEST_SENSE      0x03
#define SCSI_READ_10            0x28

#define SCSI_GCIF_DOWNLOAD        0x02
#define SCSI_GCIF_RAW_DATA        0x21
#define SCSI_GCIF_STARTUP         0x22
#define SCSI_GCIF_DAS_ERROR       0x23
#define SCSI_GCIF_START_SCANNING  0x24

#define SCSI_STATUS_GOOD              0x00
#define SCSI_STATUS_CHECK_CONDITION   0x02

/***************************************************************************
 * name: gcif_scsi_read10
 * parameter:
 *      path: scsi generic device path 
 *      data_ptr: data buffer pointer
 *      data_size: data buffer size
 *      recv_len: length of the data received
 * function: gcif scan data read using scsi read10 command 
 * **************************************************************************/
int32_t gcif_scsi_read10(char * path, uint8_t *data_ptr,
                         uint64_t data_size, uint64_t *recv_len);

/***************************************************************************
 * name: gcif_reg_fwopcode
 * parameter:
 *      path: scsi generic dev path 
 *      evpd: custom evpd
 *      sw_download_opcode: sw down load opcode
 * function: This function send scsi command to gcif to register fw download 
 *           opcode with gcif
 * **************************************************************************/
int32_t gcif_reg_fwopcode(char * path, uint8_t evpd,
                                         uint8_t sw_download_opcode);

/***************************************************************************
 * name: gcif_test_unit_ready
 * parameter:
 *      path: scsi generic dev path 
 * function: This function send scsi command to gcif to check 
 *           the device is ready for the data acquistion
 * **************************************************************************/
int32_t gcif_test_unit_ready(char * path);

/***************************************************************************
 * name: gcif_startup
 * parameter:
 *      path: scsi generic dev path 
 *      view_starting_row: view number ;default is zero
 *      rows_per_view:  rows per view 4 or 16
 *      das_channels: das channels
 * function: This function send scsi command to gcif for configuring the gcif  
 *          for the data acquisition 
 * **************************************************************************/
int32_t gcif_startup(char * path, uint8_t view_starting_row,
                                  uint8_t rows_per_view, uint16_t das_channels);

/***************************************************************************
 * name: gcif_start_scan
 * parameter:
 *      path: scsi generic dev path 
 *      p_das_config: das configuration params for the test
 *      p_das_data: das data buffer pointer
 *      das_data_size: allocated buffer size for das data
 *      recv_data_size: received data size
 * function: This function send scsi command to gcif for starting the data 
 *          acquisition on the gcif
 * **************************************************************************/
int32_t gcif_start_scan(char *path, das_param_t *p_das_config,
                               uint8_t *p_das_data, uint64_t das_data_size,
                               uint64_t *recv_data_size);

/***************************************************************************
 * name: test_execute_inquiry
 * parameter:
 *      path:       scsi generic dev path 
 *      evpd:       custom evpd for this command
 *      page_code:  custom page code for this command
 * function: This function send scsi command to gcif for getting the inquiry 
 *          data from gcif target
 * **************************************************************************/
int32_t test_execute_inquiry(char * path, uint8_t evpd, uint8_t page_code);

/***************************************************************************
 * name: test_execute_reboot_reason
 * parameter:
 *      path:       scsi generic dev path 
 *      evpd:       custom evpd for this command
 *      page_code:  custom page code for this command
 * function: This function send scsi command to gcif for getting the reboot 
 *          reason information from gcif
 * **************************************************************************/
int32_t test_execute_reboot_reason(char * path, uint8_t evpd, uint8_t page_code);

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

/***************************************************************************
 * name: gcif_scan_and_verify
 * parameter:
 *      scsi_dev_path:          scsi genereic device path
 *      das_config_param:       das configuration parameters for the test   
 * function: This function trigger the data acquisition in the gcif and get 
 *          the scan data from the gcif. After that it will compare with the
 *          golden data and report success/failure
 * **************************************************************************/
int32_t gcif_scan_and_verify(char * scsi_dev_path, das_param_t *das_config_param);

/***************************************************************************
 * name: set_xfer_data
 * parameter:
 *      p_hdr:      poiter to sg_io_hdr struct
 *      data:       pointer to the data transfer buffer 
 *      length:     size of the data transfer buffer
 * function: Set the data transfer buffer and its size in the sg_io_hdr for the 
 *          sg_io_hdr comiplation
 * **************************************************************************/
int32_t set_xfer_data(struct sg_io_hdr * p_hdr, void * data, uint32_t length);

/***************************************************************************
 * name: set_sense_data
 * parameter:
 *      p_hdr:      poiter to sg_io_hdr struct
 *      data:       pointer to the sense buffer 
 *      length:     size of the sense buffer
 * function: Set the sense buffer and its size in the sg_io_hdr for the 
 *          sg_io_hdr comiplation
 * **************************************************************************/
int32_t set_sense_data(struct sg_io_hdr * p_hdr, uint8_t * data,
        uint32_t length);

/***************************************************************************
 * name: init_io_hdr
 * parameter:
 * function: initialize the sg_io_hdr struct fields with the most common
 *           value
 * **************************************************************************/
struct  sg_io_hdr * init_io_hdr();

/***************************************************************************
 * name: init_read_io_hdr
 * parameter:
 *      das_data_ptr: data buffer pointer
 *      das_data_size: buffer size
 * function: initialize the sg_io_hdr struct fields with the most common
 *           value
 * **************************************************************************/
struct  sg_io_hdr * init_read_io_hdr(uint8_t *das_data_ptr, uint64_t das_data_size);

/***************************************************************************
 * name: destroy_io_hdr
 * parameter:
 *      p_hdr:      poiter to sg_io_hdr struct
 * function: Delete the p_hdr pointer
 * **************************************************************************/
void destroy_io_hdr(struct sg_io_hdr * p_hdr);

/***************************************************************************
 * name: execute_inquiry
 * parameter:
 *      fd:         file descripter
 *      page_code:  cdb page code
 *      evpd:       cdb evpd
 *      p_hdr:      poiter to sg_io_hdr struct
 * function: make Inquiry cdb and execute it.
 * **************************************************************************/
int32_t execute_inquiry(int32_t fd, uint8_t page_code, uint8_t evpd, 
                        struct sg_io_hdr * p_hdr);

/***************************************************************************
 * name: execute_reg_fw_opcode
 * parameter:
 *      fd:         file descripter
 *      page_code:  cdb page code
 *      evpd:       cdb evpd
 *      p_hdr:      poiter to sg_io_hdr struct
 * function: make register fw opcode cdb and execute it.
 * **************************************************************************/
int32_t execute_reg_fw_opcode(int32_t fd, uint8_t page_code, uint8_t evpd, 
                              struct sg_io_hdr * p_hdr);

/***************************************************************************
 * name: execute_test_unit_ready
 * parameter:
 *      fd:         file descripter
 *      p_hdr:      poiter to sg_io_hdr struct
 * function: make test unit ready cdb and execute it.
 * **************************************************************************/
int32_t execute_test_unit_ready(int32_t fd, struct sg_io_hdr * p_hdr);

/***************************************************************************
 * name: execute_test_gcif_startup
 * parameter:
 *      fd:                 file descripter
 *      p_hdr:              poiter to sg_io_hdr struct
 *      view_start_row:     view starting row.usually ignored.
 *      rows_per_view:      No:of rows in a view. usually 4 or 16
 *      das_channels:       No:of channels per row. usually 768
 * function: make gcif startup cdb and execute it.
 * **************************************************************************/
int32_t execute_test_gcif_startup(int32_t fd, struct sg_io_hdr * p_hdr,
                                  uint8_t view_start_row, uint8_t rows_per_view,
                                  uint16_t das_channels);

/***************************************************************************
 * name: execute_test_gcif_start_scan
 * parameter:
 *      fd:                 file descripter
 *      p_hdr:              poiter to sg_io_hdr struct
 *      p_das_config:       pointer to das_param_t struct
 * function: make gcif start scan cdb and execute it.
 * **************************************************************************/
int32_t execute_test_gcif_start_scan(int32_t fd, struct sg_io_hdr * p_hdr,
                                     das_param_t *p_das_config);

/***************************************************************************
 * name: execute_test_gcif_scsi_read10
 * parameter:
 *      fd:                 file descripter
 *      p_hdr:              poiter to sg_io_hdr struct
 * function: get the scan data using scsi read10 method
 * **************************************************************************/
int32_t execute_test_gcif_scsi_read10(int32_t fd, struct sg_io_hdr * p_hdr);

#endif//SCSI_TEST_H_
