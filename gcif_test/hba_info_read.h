#ifndef READ_HBA_INFO_H_
#define READ_HBA_INFO_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_HBA_SZ          32
#define HOST_NAME_STR_SZ    256
#define MAX_PORT_NAME_SZ    128

#define FC_SYS_PATH         "/sys/class/fc_host/"
#define REMOTE_FC_SYS_PATH  "/sys/class/fc_remote_ports/"
#define SCSI_SYS_PATH       "/sys/class/scsi_host/"
#define SCSI_GEN_SYS_PATH   "/sys/class/scsi_generic/"
#define SYS_FS_SCSI_GEN_PATH    "/sys/class/scsi_generic"

#define GCIFA_D3_MODEL_STR        "GCIFA-D3"
#define GCIFA_D4_MODEL_STR        "GCIFA-D4"
#define VENDOR_NAME_STR           "TMRU,Inc"
#define VENDOR_CMSC_NAME_STR      "CMSC,Inc"
#define GCIF_TYPE_STR             "3"

//Do not change the below table strings without understanding the logic
#define TOP_BORDER_STR      "+==============================================================================+"
#define BLANK_LINE_STR      "|                                                                              |"
#define HBA_INFO_HEADER_STR "|                          Host FC HBA Information                             |"
//                          "|                                                                              |"
//                          "+==============================================================================+"
//                          "|                                                                              |"
#define HOST_ID_STR         "|  Host :"
//                          "|                                                                              |"
#define PORT_NAME_STR       "|      Port Name           :"
#define NODE_NAME_STR       "|      Node Name           :"
#define PORT_ID_STR         "|      Port ID             :"
#define PORT_STATE_STR      "|      Port State          :"
#define PORT_TYPE_STR       "|      Port Type           :"
#define SPEED_STR           "|      Speed               :"
#define SUPPORTED_CLASS_STR "|      Supported Class     :"
#define SUPPORTED_SPEED_STR "|      Supported Speed     :"
#define FABRIC_NAME_STR     "|      Fabric name         :"
#define SYM_NAME_STR        "|      Symbolic Name       :"
#define ACT_MODE_STR        "|      Active Mode         :"
#define CMD_PER_LUN_STR     "|      CMD Per LUN         :"
#define ISP_ID_STR          "|      ISP ID              :"
#define ISP_NAME_STR        "|      ISP Name            :"
#define LINK_STATE_STR      "|      Link State          :"
#define MODEL_DESC_STR      "|      Model Description   :"
#define MODEL_NAME_STR      "|      Model Name          :"
#define PCI_INFO_STR        "|      PCI Info            :"
#define SUPPORTED_MODE_STR  "|      Supported Modes     :"
#define FW_VER_STR          "|      FW Version          :"
#define FW_STATE_STR        "|      FW State            :"
#define FLASH_BLK_SZ_STR    "|      Flash Block Size    :"
#define DRV_VER_STR         "|      Driver Version      :"
#define SCSI_DEV_PATH       "|      SCSI Dev Path       :"
//                          "|                                                                              |"
//                          "+==============================================================================+"

//                          "+==============================================================================+"
//                          "|                                                                              |"
#define RM_FC_INF_HDR_STR   "|                      Remote FC HBA Information                               |"
//                          "|                                                                              |"
//                          "+==============================================================================+"
//                          "|                                                                              |"
#define REMOTE_ID_STR       "|  Remote :"
//                          "|                                                                              |"
//                          "|      Port Name           :"
//                          "|      Node Name           :"
//                          "|      Port ID             :"
//                          "|      Port State          :"
#define ROLE_STR            "|      Roll                :"
//                          "|      Supported Class     :"
#define VENDOR_MODEL_STR    "|      Vendor & Model      :"
//                          "|                                                                              |"
//                          "+==============================================================================+"


typedef struct hba_port_info_
{
    char dev_path[MAX_PORT_NAME_SZ];
    char host_name[MAX_PORT_NAME_SZ];
    char port_name[MAX_PORT_NAME_SZ];
    char node_name[MAX_PORT_NAME_SZ];
    char port_id[MAX_PORT_NAME_SZ];
    char port_state[MAX_PORT_NAME_SZ];
    char port_type[MAX_PORT_NAME_SZ];
    char speed[MAX_PORT_NAME_SZ];
    char supported_class[MAX_PORT_NAME_SZ];
    char supported_speed[MAX_PORT_NAME_SZ];
    char fabric_name[MAX_PORT_NAME_SZ];
    char symbolic_name[MAX_PORT_NAME_SZ];
    char active_mode[MAX_PORT_NAME_SZ];
    char cmd_per_lun[MAX_PORT_NAME_SZ];
    char isp_id[MAX_PORT_NAME_SZ];
    char isp_name[MAX_PORT_NAME_SZ];
    char link_state[MAX_PORT_NAME_SZ];
    char model_desc[MAX_PORT_NAME_SZ];
    char model_name[MAX_PORT_NAME_SZ];
    char pci_info[MAX_PORT_NAME_SZ];
    char supported_mode[MAX_PORT_NAME_SZ];
    char fw_ver[MAX_PORT_NAME_SZ];
    char fw_state[MAX_PORT_NAME_SZ];
    char flash_blk_sz[MAX_PORT_NAME_SZ];
    char drv_ver[MAX_PORT_NAME_SZ];
    char remote_name[MAX_PORT_NAME_SZ];
    char r_port_name[MAX_PORT_NAME_SZ];
    char r_node_name[MAX_PORT_NAME_SZ];
    char r_port_id[MAX_PORT_NAME_SZ];
    char r_port_state[MAX_PORT_NAME_SZ];
    char role[MAX_PORT_NAME_SZ];
    char r_supported_class[MAX_PORT_NAME_SZ];
    char r_device_vendor_model[MAX_PORT_NAME_SZ];
}hba_port_info_t;

/******************************************************************************
 * name: display_hba_info
 * parameter:
 *      host_idx: index number 
 * function: Display the HBA information on the console screen from the 
 *           globally stored structure  g_hba_info
 * ****************************************************************************/
void display_hba_info(uint8_t host_idx);

/******************************************************************************
 * name: get_generic_scsi_devpath
 * parameter:
 *      sys_class_sg_path: sysfs sg path  
 *      dev_path: dev path name 
 *      host_num_idx: index
 * function: Get the sg dev path of the specific vendor, model, type 
 *          here TMRU,Inc,GCIFA-D3,type3
 * ****************************************************************************/
int32_t get_generic_scsi_devpath(char *sys_class_sg_path, 
                                char *dev_path, 
                                uint8_t host_num_idx);

/******************************************************************************
 * name: get_scsi_devpath
 * parameter:
 *      scsi_dev_path: scsi device path  
 * function: Get the scsi device path of the gcif
 * ****************************************************************************/
int32_t get_scsi_devpath(char *scsi_dev_path);

/******************************************************************************
 * name: get_host_list
 * parameter:
 *      sys_class_fc_path: sysfs fc path  
 *      host_count: host count 
 *      host_list: host_list pointer
 *      max_host_cnt: max supported host count
 * function: Get the scsi host list detected in the system
 * ****************************************************************************/
int32_t get_host_list(char *sys_class_fc_path, uint8_t* host_count,
                  char **host_list, uint8_t max_host_cnt);

/******************************************************************************
 * name: get_fc_host_list
 * parameter:Nil

 * function: Get the fc host list detected in the system
 * ****************************************************************************/
int32_t get_fc_host_list();

/******************************************************************************
 * name: get_remote_fc_host_list
 * parameter:
 *      host_num_idx: index 
 * function: Get the remote fc host list detected in the system
 * ****************************************************************************/
int32_t get_remote_fc_host_list( uint8_t host_num_idx);

#endif //READ_HBA_INFO_H_
