#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>

#include "hba_info_read.h"
#include "logger.h"

//Global variable
hba_port_info_t g_hba_info[MAX_HBA_SZ] = {'\0'};
uint8_t g_num_host_detected = 0;

// Global variable end here

//Local function proto types
/***************************************************************************
 * name: display_fmt_helper
 * parameter:
 *      node: node string
 *      fmt_type: format type
 * function: Support function for display the hba parameters on the console
 * **************************************************************************/
static void display_fmt_helper(char *node, uint8_t fmt_type);

/***************************************************************************
 * name: read_sysfs
 * parameter:
 *      sys_path: sysfs system path 
 *      buffer: sysfs node name
 *      buffer_sz: buffer size
 * function: Read sysfs node name value from the sys path
 * **************************************************************************/
static int32_t read_sysfs(char *sys_path, char *buffer, uint32_t buffer_sz);

/***************************************************************************
 * name: get_fc_host_param
 * parameter:
 *      sys_class_fc_path: sysfs class system path 
 *      host_idx: index number of the global storage
 * function: Read the FC host parametes from the sysfs and store it in the 
 *           global structure  g_hba_info  
 * **************************************************************************/
static int32_t get_fc_host_param(char *sys_class_fc_path, uint8_t host_idx);

/***************************************************************************
 * name: get_fc_remote_param
 * parameter:
 *      sys_class_remote_fc_path: sysfs class system path of the remote dev
 *      host_idx: index number of the global storage
 * function: Read the remote FC  parametes from the sysfs and store it in the 
 *           global structure  g_hba_info  
 * **************************************************************************/
static int32_t get_fc_remote_param(char *sys_class_remote_fc_path, uint8_t host_idx);

/******************************************************************************
 * name: get_scsi_host_param
 * parameter:
 *      sys_class_scsi_host_path: sysfs class system path of the host scsi dev
 *      host_idx: index number of the global storage
 * function: Read the host scsi parametes from the sysfs and store it in the 
 *           global structure  g_hba_info  
 * ****************************************************************************/
static int32_t get_scsi_host_param(char *sys_class_scsi_host_path, uint8_t host_idx);

/******************************************************************************
 * name: read_dir
 * parameter:
 *      dir_path: directory path  
 *      dir_name: directory name pointer
 *      dir_name_sz: directory name length
 * function: Read the directory and return the dir content name 
 * ****************************************************************************/
static int32_t read_dir(char *dir_path, char *dir_name, uint32_t dir_name_sz);

//Local function proto types end here

/***************************************************************************
 * name: display_fmt_helper
 * parameter:
 *      node: node string
 *      fmt_type: format type
 * function: Support function for display the hba parameters on the console
 * **************************************************************************/
static void display_fmt_helper(char *node, uint8_t fmt_type)
{
    //fmt_type  : 0 nothing
    //fmt_type  : 1 host
    //fmt_type  : 2 remote
    //fmt_type  : 3 port name
    uint8_t top_border_len = strlen(TOP_BORDER_STR);
    uint8_t fmt_type_len = 0;
    uint8_t node_len = 0;
    uint8_t shift_len = 0;

    if(node != NULL)
    {
        node_len = strlen(node);
    }

    switch(fmt_type)
    {
        case 1:
        {
            fmt_type_len = strlen(HOST_ID_STR);
        }
        break;

        case 2:
        {
            fmt_type_len = strlen(REMOTE_ID_STR);
        }
        break;

        case 3:
        {
            fmt_type_len = strlen(PORT_NAME_STR);
        }
        break;

        default:
        fmt_type_len = 0;
    }

    shift_len = top_border_len - (fmt_type_len + node_len + 1);
    //printf(" shift len = %d\n", shift_len);

    for (uint8_t i = 0; i < shift_len-1 ; i++)
    {
         printf("%s", " ");
    }
    printf("%s \n", "|");
}

/***************************************************************************
 * name: read_sysfs
 * parameter:
 *      sys_path: sysfs system path 
 *      buffer: sysfs node name
 *      buffer_sz: buffer size
 * function: Read sysfs node name value from the sys path
 * **************************************************************************/
static int32_t read_sysfs(char *sys_path, char *buffer, uint32_t buffer_sz)
{
    //char buf[1024];
    ssize_t len  = 0;
    int fd = 0;

    if((sys_path == NULL) || (buffer == NULL))
    {
        LOG_ERROR("Invalid filename pointer passed");
        return EINVAL;
    }

    fd = open(sys_path, O_RDONLY);
    if (fd < 0)
    {
        LOG_ERROR(" open %s:%s", sys_path, strerror(errno));
        return EIO;
    }

    len = read(fd, buffer, buffer_sz-1);

    if (len < 0)
    {
        LOG_ERROR(" read %s: %s",sys_path, strerror(errno));
        return EIO;
    }

    buffer[len] = '\0';

    close(fd);

    return EXIT_SUCCESS;
}

/***************************************************************************
 * name: get_fc_host_param
 * parameter:
 *      sys_class_fc_path: sysfs class system path 
 *      host_idx: index number of the global storage
 * function: Read the FC host parametes from the sysfs and store it in the 
 *           global structure  g_hba_info  
 * **************************************************************************/
static int32_t get_fc_host_param(char *sys_class_fc_path, uint8_t host_idx)
{
    char sys_node_path[512] = {'\0'};

    if(sys_class_fc_path == NULL)
    {
        LOG_ERROR("Invalid sysfs class path passed");
        return EXIT_FAILURE;
    }

    //g_hba_info[host_idx].port_name[strlen(g_hba_info[host_idx].port_name) - 1] = '\0';
    // removing \n from the  string
    sprintf(sys_node_path, "%s/%s", sys_class_fc_path,"port_name");
    read_sysfs(sys_node_path, g_hba_info[host_idx].port_name, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].port_name[strlen(g_hba_info[host_idx].port_name) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_fc_path,"node_name");
    read_sysfs(sys_node_path, g_hba_info[host_idx].node_name, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].node_name[strlen(g_hba_info[host_idx].node_name) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_fc_path,"port_id");
    read_sysfs(sys_node_path, g_hba_info[host_idx].port_id, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].port_id[strlen(g_hba_info[host_idx].port_id) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_fc_path,"port_state");
    read_sysfs(sys_node_path, g_hba_info[host_idx].port_state, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].port_state[strlen(g_hba_info[host_idx].port_state) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_fc_path,"port_type");
    read_sysfs(sys_node_path, g_hba_info[host_idx].port_type, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].port_type[strlen(g_hba_info[host_idx].port_type) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_fc_path,"speed");
    read_sysfs(sys_node_path, g_hba_info[host_idx].speed, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].speed[strlen(g_hba_info[host_idx].speed) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_fc_path,"supported_classes");
    read_sysfs(sys_node_path, g_hba_info[host_idx].supported_class, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].supported_class[strlen(g_hba_info[host_idx].supported_class) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_fc_path,"supported_speeds");
    read_sysfs(sys_node_path, g_hba_info[host_idx].supported_speed, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].supported_speed[strlen(g_hba_info[host_idx].supported_speed) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_fc_path,"fabric_name");
    read_sysfs(sys_node_path, g_hba_info[host_idx].fabric_name, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].fabric_name[strlen(g_hba_info[host_idx].fabric_name) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_fc_path,"symbolic_name");
    read_sysfs(sys_node_path, g_hba_info[host_idx].symbolic_name, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].symbolic_name[strlen(g_hba_info[host_idx].symbolic_name) - 1] = '\0';
    sys_node_path[0] = '\0';

    return EXIT_SUCCESS;
}

/***************************************************************************
 * name: get_fc_remote_param
 * parameter:
 *      sys_class_remote_fc_path: sysfs class system path of the remote dev
 *      host_idx: index number of the global storage
 * function: Read the remote FC  parametes from the sysfs and store it in the 
 *           global structure  g_hba_info  
 * **************************************************************************/
static int32_t get_fc_remote_param(char *sys_class_remote_fc_path, uint8_t host_idx)
{
    //sys/class/fc_remote_ports//sys/class/fc_remote_ports/port_name
    char sys_node_path[512] = {'\0'};

    if(sys_class_remote_fc_path == NULL)
    {
        LOG_ERROR("Invalid sysfs class path passed");
        return EXIT_FAILURE;
    }

    //g_hba_info[host_idx].r_port_name[strlen(g_hba_info[host_idx].r_port_name) - 1] = '\0';
    // Remove \n from the string
    sprintf(sys_node_path, "%s/%s", sys_class_remote_fc_path,"port_name");
    read_sysfs(sys_node_path, g_hba_info[host_idx].r_port_name, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].r_port_name[strlen(g_hba_info[host_idx].r_port_name) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_remote_fc_path,"node_name");
    read_sysfs(sys_node_path, g_hba_info[host_idx].r_node_name, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].r_node_name[strlen(g_hba_info[host_idx].r_node_name) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_remote_fc_path,"port_id");
    read_sysfs(sys_node_path, g_hba_info[host_idx].r_port_id, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].r_port_id[strlen(g_hba_info[host_idx].r_port_id) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_remote_fc_path,"port_state");
    read_sysfs(sys_node_path, g_hba_info[host_idx].r_port_state, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].r_port_state[strlen(g_hba_info[host_idx].r_port_state) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_remote_fc_path,"roles");
    read_sysfs(sys_node_path, g_hba_info[host_idx].role, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].role[strlen(g_hba_info[host_idx].role) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_remote_fc_path,"supported_classes");
    read_sysfs(sys_node_path, g_hba_info[host_idx].r_supported_class, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].r_supported_class[strlen(g_hba_info[host_idx].r_supported_class) - 1] = '\0';
    sys_node_path[0] = '\0';

    return EXIT_SUCCESS;
}

/******************************************************************************
 * name: get_scsi_host_param
 * parameter:
 *      sys_class_scsi_host_path: sysfs class system path of the host scsi dev
 *      host_idx: index number of the global storage
 * function: Read the host scsi parametes from the sysfs and store it in the 
 *           global structure  g_hba_info  
 * ****************************************************************************/
static int32_t get_scsi_host_param(char *sys_class_scsi_host_path, uint8_t host_idx)
{
    //sys/class/scsi_host/host6/node_name
    char sys_node_path[512] = {'\0'};

    if(sys_class_scsi_host_path == NULL)
    {
        LOG_ERROR("Invalid sysfs class path passed");
        return EXIT_FAILURE;
    }

    //g_hba_info[host_idx].active_mode[strlen(g_hba_info[host_idx].active_mode) - 1] = '\0';
    // for removing \n from the string

    sprintf(sys_node_path, "%s/%s", sys_class_scsi_host_path,"active_mode");
    read_sysfs(sys_node_path, g_hba_info[host_idx].active_mode, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].active_mode[strlen(g_hba_info[host_idx].active_mode) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_scsi_host_path,"cmd_per_lun");
    read_sysfs(sys_node_path, g_hba_info[host_idx].cmd_per_lun, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].cmd_per_lun[strlen(g_hba_info[host_idx].cmd_per_lun) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_scsi_host_path,"isp_id");
    read_sysfs(sys_node_path, g_hba_info[host_idx].isp_id, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].isp_id[strlen(g_hba_info[host_idx].isp_id) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_scsi_host_path,"isp_name");
    read_sysfs(sys_node_path, g_hba_info[host_idx].isp_name, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].isp_name[strlen(g_hba_info[host_idx].isp_name) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_scsi_host_path,"link_state");
    read_sysfs(sys_node_path, g_hba_info[host_idx].link_state, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].link_state[strlen(g_hba_info[host_idx].link_state) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_scsi_host_path,"model_desc");
    read_sysfs(sys_node_path, g_hba_info[host_idx].model_desc, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].model_desc[strlen(g_hba_info[host_idx].model_desc) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_scsi_host_path,"model_name");
    read_sysfs(sys_node_path, g_hba_info[host_idx].model_name, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].model_name[strlen(g_hba_info[host_idx].model_name) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_scsi_host_path,"pci_info");
    read_sysfs(sys_node_path, g_hba_info[host_idx].pci_info, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].pci_info[strlen(g_hba_info[host_idx].pci_info) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_scsi_host_path,"supported_mode");
    read_sysfs(sys_node_path, g_hba_info[host_idx].supported_mode, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].supported_mode[strlen(g_hba_info[host_idx].supported_mode) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_scsi_host_path,"fw_version");
    read_sysfs(sys_node_path, g_hba_info[host_idx].fw_ver, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].fw_ver[strlen(g_hba_info[host_idx].fw_ver) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_scsi_host_path,"fw_state");
    read_sysfs(sys_node_path, g_hba_info[host_idx].fw_state, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].fw_state[strlen(g_hba_info[host_idx].fw_state) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_scsi_host_path,"flash_block_size");
    read_sysfs(sys_node_path, g_hba_info[host_idx].flash_blk_sz, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].flash_blk_sz[strlen(g_hba_info[host_idx].flash_blk_sz) - 1] = '\0';
    sys_node_path[0] = '\0';

    sprintf(sys_node_path, "%s/%s", sys_class_scsi_host_path,"driver_version");
    read_sysfs(sys_node_path, g_hba_info[host_idx].drv_ver, MAX_PORT_NAME_SZ);
    g_hba_info[host_idx].drv_ver[strlen(g_hba_info[host_idx].drv_ver) - 1] = '\0';
    sys_node_path[0] = '\0';

    return EXIT_SUCCESS;
}

/******************************************************************************
 * name: display_hba_info
 * parameter:
 *      host_idx: index number 
 * function: Display the HBA information on the console screen from the 
 *           globally stored structure  g_hba_info
 * ****************************************************************************/
void display_hba_info(uint8_t host_idx)
{
    //fmt_type  : 0 nothing
    //fmt_type  : 1 host
    //fmt_type  : 2 remote
    //fmt_type  : 3 port name
    printf("%s \n", TOP_BORDER_STR);
    printf("%s \n", BLANK_LINE_STR);
    printf("%s \n", HBA_INFO_HEADER_STR);
    printf("%s \n", BLANK_LINE_STR);
    printf("%s \n", TOP_BORDER_STR);
    printf("%s \n", BLANK_LINE_STR);

    printf("%s %s", HOST_ID_STR, g_hba_info[host_idx].host_name);
    display_fmt_helper(g_hba_info[host_idx].host_name, 1);
    printf("%s \n", BLANK_LINE_STR);

    printf("%s %s", PORT_NAME_STR, g_hba_info[host_idx].port_name);
    display_fmt_helper(g_hba_info[host_idx].port_name, 3);

    printf("%s %s", NODE_NAME_STR, g_hba_info[host_idx].node_name);
    display_fmt_helper(g_hba_info[host_idx].node_name, 3);

    printf("%s %s", PORT_ID_STR, g_hba_info[host_idx].port_id);
    display_fmt_helper(g_hba_info[host_idx].port_id, 3);

    printf("%s %s", PORT_STATE_STR, g_hba_info[host_idx].port_state);
    display_fmt_helper(g_hba_info[host_idx].port_state, 3);

    printf("%s %s", PORT_TYPE_STR, g_hba_info[host_idx].port_type);
    display_fmt_helper(g_hba_info[host_idx].port_type, 3);

    printf("%s %s", SPEED_STR, g_hba_info[host_idx].speed);
    display_fmt_helper(g_hba_info[host_idx].speed, 3);

    printf("%s %s", SUPPORTED_CLASS_STR, g_hba_info[host_idx].supported_class);
    display_fmt_helper(g_hba_info[host_idx].supported_class, 3);

    printf("%s %s", SUPPORTED_SPEED_STR, g_hba_info[host_idx].supported_speed);
    display_fmt_helper(g_hba_info[host_idx].supported_speed, 3);

    printf("%s %s", FABRIC_NAME_STR, g_hba_info[host_idx].fabric_name);
    display_fmt_helper(g_hba_info[host_idx].fabric_name, 3);

    printf("%s %s", SYM_NAME_STR, g_hba_info[host_idx].symbolic_name);
    display_fmt_helper(g_hba_info[host_idx].symbolic_name, 3);

    printf("%s %s", ACT_MODE_STR, g_hba_info[host_idx].active_mode);
    display_fmt_helper(g_hba_info[host_idx].active_mode, 3);

    printf("%s %s", CMD_PER_LUN_STR, g_hba_info[host_idx].cmd_per_lun);
    display_fmt_helper(g_hba_info[host_idx].cmd_per_lun, 3);

    printf("%s %s", ISP_ID_STR, g_hba_info[host_idx].isp_id);
    display_fmt_helper(g_hba_info[host_idx].isp_id, 3);

    printf("%s %s", ISP_NAME_STR, g_hba_info[host_idx].isp_name);
    display_fmt_helper(g_hba_info[host_idx].isp_name, 3);

    printf("%s %s", LINK_STATE_STR, g_hba_info[host_idx].link_state);
    display_fmt_helper(g_hba_info[host_idx].link_state, 3);

    printf("%s %s", MODEL_DESC_STR, g_hba_info[host_idx].model_desc);
    display_fmt_helper(g_hba_info[host_idx].model_desc, 3);

    printf("%s %s", MODEL_NAME_STR, g_hba_info[host_idx].model_name);
    display_fmt_helper(g_hba_info[host_idx].model_name, 3);

    printf("%s %s", PCI_INFO_STR, g_hba_info[host_idx].pci_info);
    display_fmt_helper(g_hba_info[host_idx].pci_info, 3);

    printf("%s %s", SUPPORTED_MODE_STR, g_hba_info[host_idx].supported_mode);
    display_fmt_helper(g_hba_info[host_idx].supported_mode, 3);

    printf("%s %s", FW_VER_STR, g_hba_info[host_idx].fw_ver);
    display_fmt_helper(g_hba_info[host_idx].fw_ver, 3);

    printf("%s %s", FW_STATE_STR, g_hba_info[host_idx].fw_state);
    display_fmt_helper(g_hba_info[host_idx].fw_state, 3);

    printf("%s %s", FLASH_BLK_SZ_STR, g_hba_info[host_idx].flash_blk_sz);
    display_fmt_helper(g_hba_info[host_idx].flash_blk_sz, 3);

    printf("%s %s", DRV_VER_STR, g_hba_info[host_idx].drv_ver);
    display_fmt_helper(g_hba_info[host_idx].drv_ver, 3);

    printf("%s %s", SCSI_DEV_PATH, g_hba_info[host_idx].dev_path);
    display_fmt_helper(g_hba_info[host_idx].dev_path, 3);

    printf("%s \n", BLANK_LINE_STR);
    printf("%s \n", TOP_BORDER_STR);
    printf(" \n");

    // Remote port informations
    printf("%s \n", TOP_BORDER_STR);
    printf("%s \n", BLANK_LINE_STR);
    printf("%s \n", RM_FC_INF_HDR_STR);
    printf("%s \n", BLANK_LINE_STR);
    printf("%s \n", TOP_BORDER_STR);
    printf("%s \n", BLANK_LINE_STR);

    printf("%s %s", REMOTE_ID_STR, g_hba_info[host_idx].remote_name);
    display_fmt_helper(g_hba_info[host_idx].remote_name, 2);

    printf("%s \n", BLANK_LINE_STR);

    printf("%s %s", PORT_NAME_STR, g_hba_info[host_idx].r_port_name);
    display_fmt_helper(g_hba_info[host_idx].r_port_name, 3);

    printf("%s %s", NODE_NAME_STR, g_hba_info[host_idx].r_node_name);
    display_fmt_helper(g_hba_info[host_idx].r_node_name, 3);

    printf("%s %s", PORT_ID_STR, g_hba_info[host_idx].r_port_id);
    display_fmt_helper(g_hba_info[host_idx].r_port_id, 3);

    printf("%s %s", PORT_STATE_STR, g_hba_info[host_idx].r_port_state);
    display_fmt_helper(g_hba_info[host_idx].r_port_state, 3);

    printf("%s %s", ROLE_STR, g_hba_info[host_idx].role);
    display_fmt_helper(g_hba_info[host_idx].role, 3);

    printf("%s %s", SUPPORTED_CLASS_STR, g_hba_info[host_idx].r_supported_class);
    display_fmt_helper(g_hba_info[host_idx].r_supported_class, 3);

    printf("%s %s", VENDOR_MODEL_STR, g_hba_info[host_idx].r_device_vendor_model);
    display_fmt_helper(g_hba_info[host_idx].r_device_vendor_model, 3);

    printf("%s \n", BLANK_LINE_STR);
    printf("%s \n", TOP_BORDER_STR);
    printf(" \n");
}

/******************************************************************************
 * name: read_dir
 * parameter:
 *      dir_path: directory path  
 *      dir_name: directory name pointer
 *      dir_name_sz: directory name length
 * function: Read the directory and return the dir content name 
 * ****************************************************************************/
static int32_t read_dir(char *dir_path, char *dir_name, uint32_t dir_name_sz)
{
    DIR *dir;
    struct dirent *entry;

    if((dir_path == NULL) || (dir_name == NULL))
    {
        LOG_ERROR("Invalid sys class path passed");
        return EXIT_FAILURE;
    }

    dir = opendir(dir_path);
    if( dir == NULL )
    {
        LOG_ERROR("Failed to open %s", dir_path);
        return EXIT_FAILURE;
    }

    while((entry = readdir(dir)) != NULL)
    {
        if(!strcmp(entry->d_name,".") || !strcmp(entry->d_name,".."))
        {
           continue;
        }

        strncpy(dir_name, entry->d_name, dir_name_sz);
    }

    closedir(dir);

    return EXIT_SUCCESS;
}

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
                                uint8_t host_num_idx)
{
    DIR *dir;
    struct dirent *entry;
    char model[32] = {'\0'};
    char vendor[32] = {'\0'};
    char type[32] = {'\0'};
    char sys_fs_path[1024] = {'\0'};
    char dev_num[256] = {'\0'};
    uint8_t device_found = 0;
    char *token = NULL;
    const char delim[2] = ":";
    char lun_str[16] = {'\0'};
    char host_str[16] = {'\0'};
    char host_id_str[16] = {'\0'};

    if((sys_class_sg_path == NULL) || (dev_path == NULL))
    {
        LOG_ERROR("Invalid sys class path passed");
        return EXIT_FAILURE;
    }

    //for take numeric part from the host string
    // e.g. host2 take 2 from the string
    sprintf(host_id_str, "%s",(g_hba_info[host_num_idx].host_name + 4));

    dir = opendir(sys_class_sg_path);
    if( dir == NULL )
    {
        LOG_ERROR("Failed to open %s", sys_class_sg_path);
        return EXIT_FAILURE;
    }

    while((entry = readdir(dir)) != NULL)
    {
        if(!strcmp(entry->d_name,".") || !strcmp(entry->d_name,".."))
        {
           continue;
        }

        sprintf(sys_fs_path,"%s/%s%s",sys_class_sg_path, entry->d_name, "/device/model");
        read_sysfs(sys_fs_path, model, sizeof(model));
        sys_fs_path[0] = '\0';

        sprintf(sys_fs_path,"%s/%s%s",sys_class_sg_path, entry->d_name,"/device/vendor");
        read_sysfs(sys_fs_path, vendor, sizeof(vendor));
        sys_fs_path[0] = '\0';

        sprintf(sys_fs_path,"%s/%s%s",sys_class_sg_path, entry->d_name,"/device/type");
        read_sysfs(sys_fs_path, type, sizeof(type));
        sys_fs_path[0] = '\0';

        // Trim unwanted characters from the buffer
        model[strlen(GCIFA_D3_MODEL_STR)] = '\0';

        // Remove \n from the string
        vendor[strlen(vendor)-1] = '\0';

        // Remove \n from the string
        type[strlen(type) -1] = '\0';

        //printf(" model = %s vendor = %s type = %s\n", model, vendor, type );

        if( (strcmp(model, GCIFA_D3_MODEL_STR) == 0) &&
            (strcmp(vendor, VENDOR_NAME_STR) == 0) &&
            (strcmp(type, GCIF_TYPE_STR) == 0) )
        {
            //Matching device found
            sprintf(sys_fs_path,"%s/%s%s",sys_class_sg_path, entry->d_name,"/device/scsi_device");
            read_dir(sys_fs_path, dev_num, sizeof(dev_num));
            sys_fs_path[0] = '\0';

            //Checking this is the first device under this class
            //2:0:0:0
            //2:0:0:1
            // Identifying whether the last char is '0'

            token = strtok(dev_num, delim);
            if(token)
            {
                memset(host_str, 0, sizeof(host_str));
                sprintf(host_str, "%s", token);
            }

            while (token != NULL)
            {
                memset(lun_str, 0, sizeof(lun_str));
                sprintf(lun_str, "%s", token);
                token = strtok(NULL, delim);
            }

            if( (strcmp(lun_str, "0") == 0) && (strcmp(host_id_str, host_str) == 0))
            {
                sprintf(dev_path, "%s%s", "/dev/", entry->d_name);
                sprintf(g_hba_info[host_num_idx].r_device_vendor_model,"%s %s", vendor, model);
                device_found = 1;
                break;
            }
        }

        // Clearing buffer
        model[0] = '\0';
        vendor[0] = '\0';
        type[0] = '\0';
    }

    closedir(dir);

    if(device_found)
    {
        return EXIT_SUCCESS;
    }
    else
    {
        LOG_ERROR("Could not found vendor specific SCSI device");
        return EXIT_FAILURE;
    }
}

/******************************************************************************
 * name: get_scsi_devpath
 * parameter:
 *      scsi_dev_path: scsi device path  
 * function: Get the scsi device path of the gcif
 * ****************************************************************************/
int32_t get_scsi_devpath(char *scsi_dev_path)
{
    if(scsi_dev_path == NULL)
    {
        LOG_ERROR("Invalid parameter passed.");
        return EXIT_FAILURE;
    }

    if(g_num_host_detected)
    {
        //Currently 0th element return
        strcpy(scsi_dev_path, g_hba_info[0].dev_path);
    }
    else
    {
        LOG_ERROR("No FC host present in the system.");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

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
                  char **host_list, uint8_t max_host_cnt)
{
    DIR *dir;
    struct dirent *entry;
    uint8_t count = 0;
    char *ptr = NULL;

    if( (sys_class_fc_path == NULL) || (host_list == NULL) || (host_count == NULL))
    {
        LOG_ERROR("Invalid pointer passed");
        return EXIT_FAILURE;
    }

    dir = opendir(sys_class_fc_path);
    if( dir == NULL )
    {
        LOG_ERROR("Failed to open %s", sys_class_fc_path);
        return EXIT_FAILURE;
    }

    ptr = (char *) &host_list[count];

    while((entry = readdir(dir)) != NULL)
    {
        if(!strcmp(entry->d_name,".") || !strcmp(entry->d_name,".."))
        {
           continue;
        }

        strncpy(ptr, entry->d_name, HOST_NAME_STR_SZ);
        //printf("%s\n",ptr);
        count++;
        ptr += HOST_NAME_STR_SZ ;

        if(count == max_host_cnt)
        {
            LOG_WARN("Max host count reached !!");
            break;
        }
    }

    *host_count = count;
    closedir(dir);
    return EXIT_SUCCESS;
}

/******************************************************************************
 * name: get_fc_host_list
 * parameter:Nil

 * function: Get the fc host list detected in the system
 * ****************************************************************************/
int32_t get_fc_host_list()
{
    int32_t status = EXIT_SUCCESS;
    char host_list[MAX_HBA_SZ][HOST_NAME_STR_SZ] ={'\0'};
    char sys_path[256] ={'\0'};

    //status = get_host_list("/sys/class/scsi_generic", &g_num_host_detected, &host_list, MAX_HBA_SZ);
    status = get_host_list(FC_SYS_PATH, &g_num_host_detected, (char **)host_list, MAX_HBA_SZ);

    if(status != EXIT_SUCCESS)
    {
        LOG_ERROR("Failed to get host list");
        return EXIT_FAILURE;
    }

    LOG_PRINT("Total number of host detected on the system = %d ", g_num_host_detected);

    for(uint8_t i = 0; i < g_num_host_detected; i++)
    {
        //printf("%s \n", host_list[i]);
        strcpy(g_hba_info[i].host_name, host_list[i]);

        sprintf(sys_path, "%s%s", FC_SYS_PATH, host_list[i]);
        status = get_fc_host_param(sys_path, i);
        if(status != EXIT_SUCCESS)
        {
            LOG_ERROR("Failed to get fc host parameters!!");
            return EXIT_FAILURE;
        }
        sys_path[0] = '\0';

        sprintf(sys_path, "%s%s", SCSI_SYS_PATH, host_list[i]);
        status = get_scsi_host_param(sys_path, i);
        if(status != EXIT_SUCCESS)
        {
            LOG_ERROR("Failed to get scsi host parameters!!");
            return EXIT_FAILURE;
        }
        sys_path[0] = '\0';

        status = get_generic_scsi_devpath(SYS_FS_SCSI_GEN_PATH, g_hba_info[i].dev_path, i);
        if(status == EXIT_SUCCESS)
        {
            LOG_PRINT("scsi dev path: %s", g_hba_info[i].dev_path);
        }
        else
        {
            LOG_ERROR("Failed to get vendor specific SCSI device path");
            return EXIT_FAILURE;
        }

        status = get_remote_fc_host_list(i);
        if(status != EXIT_SUCCESS)
        {
            LOG_ERROR("Failed to get fc remote port parameters!!");
            return EXIT_FAILURE;
        }
    }

    return status;
}

/******************************************************************************
 * name: get_remote_fc_host_list
 * parameter:
 *      host_num_idx: index 
 * function: Get the remote fc host list detected in the system
 * ****************************************************************************/
int32_t get_remote_fc_host_list( uint8_t host_num_idx)
{
    int32_t status = EXIT_SUCCESS;
    char host_list[MAX_HBA_SZ][HOST_NAME_STR_SZ] ={'\0'};
    char sys_path[256] ={'\0'};
    uint8_t num_host_detected=0;
    char host_id_str[16] = {'\0'};
    char remote_port[16] = {'\0'};
    char r_port[MAX_PORT_NAME_SZ] = {'\0'};
    char *token = NULL;
    const char delim[2] = ":";

    sprintf(host_id_str,"%s", (g_hba_info[host_num_idx].host_name+4));

    //printf("--->%s %d\n", host_id_str, host_id);

    status = get_host_list(REMOTE_FC_SYS_PATH, &num_host_detected, (char **)host_list, MAX_HBA_SZ);

    if(status != EXIT_SUCCESS)
    {
        LOG_ERROR("Failed to get remote fc host list!!");
        return EXIT_FAILURE;
    }

    for(uint8_t i = 0; i < num_host_detected; i++)
    {
        //Find remote port number from the string
        //from rport-1:0-0 extract 1
        strcpy(r_port, host_list[i]);

        token = strtok((r_port+6),delim);
        if(token != NULL)
        {
            memset(remote_port, 0, sizeof(remote_port));
            sprintf(remote_port, "%s", token);
        }

        //printf("--->%s %s\n", r_port, remote_port);

        if(strcmp(host_id_str, remote_port) == 0)
        {
            //printf("--->%d %s \n", __LINE__, host_list[i]);
            strcpy(g_hba_info[host_num_idx].remote_name, host_list[i]);

            sprintf(sys_path, "%s%s", REMOTE_FC_SYS_PATH, host_list[i]);
            status = get_fc_remote_param(sys_path, host_num_idx);
            if(status != EXIT_SUCCESS)
            {
                LOG_ERROR("Failed to get remote fc parameters!!");
                return EXIT_FAILURE;
            }
            sys_path[0] = '\0';
        }
    }

    return EXIT_SUCCESS;
}
