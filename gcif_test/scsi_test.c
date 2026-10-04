#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>

#include "data_rate_pat_gen.h"
#include "scsi_test.h"
#include "hba_info_read.h"
#include "logger.h"

#define DAS_CHANNEL_STR			"daschannels"
#define ROWS_PER_VIEW_STR		"rowsperview"
#define VIEW_START_ROW_STR		"viewstartrow"
#define VIEW_PER_TX 			"viewspertx"
#define SIMULATION_STR			"simulation"
#define VIEWS_PERSCAN_STR 		"viewsperscan"
#define VIEWS_PER_SEC_STR 		"viewspersec"

//70+4+8
    #define DUMP_U_LINE      "+-----------------------------------------------------------------------------------+"
    #define DUMP_HEADER      "| Index   |                       Error GCIF Scan Data                              |"
  //#define DUMP_U_LINE      "-------------------------------------------------------------------------------------"


//Global variables start here
das_info_t g_das_info = {0};
uint8_t g_start_flag = 0;
uint8_t g_sense_buffer[SENSE_LEN];
uint8_t g_data_buffer[BLOCK_LEN*256];
extern uint8_t g_esc_pressed;

peripheral_device_t peripheral_dev_tbl[] =
{
	{ 0x00, "SBC-4 Direct access block device (e.g., magnetic disk)   ", },
	{ 0x01, "SSC-3 Sequential-access device (e.g., magnetic tape)     ", },
	{ 0x02, "SSC Printer device                                       ", },
	{ 0x03, "SPC-2 Processor device                                   ", },
	{ 0x04, "SBC Write-once device (e.g., some optical disks)         ", },
	{ 0x05, "MMC-5 CD/DVD device                                      ", },
	{ 0x06, "Scanner device (obsolete)                                ", },
	{ 0x07, "SBC Optical memory device (e.g., some optical disks)     ", },
	{ 0x08, "SMC-3 Medium changer device (e.g., jukeboxes)            ", },
	{ 0x09, "Communications device (obsolete)                         ", },
	{ 0x0A, "Obsolete                                                 ", },
	{ 0x0B, "Obsolete                                                 ", },
	{ 0x0C, "SCC-2 Storage array controller device (e.g., RAID)       ", },
	{ 0x0D, "SES Enclosure services device                            ", },
	{ 0x0E, "RBC Simplified direct-access device (e.g., magnetic disk ", },
	{ 0x0F, "OCRW Optical card reader/writer device                   ", },
	{ 0x10, "BCC Bridge Controller Commands                           ", },
	{ 0x11, "OSD Object-based Storage Device                          ", },
	{ 0x12, "ADC-2 Automation/Drive Interface                         ", },
	{ 0x13, "Reserved                                                 ", },
	{ 0x14, "Reserved                                                 ", },
	{ 0x15, "Reserved                                                 ", },
	{ 0x16, "Reserved                                                 ", },
	{ 0x17, "Reserved                                                 ", },
	{ 0x18, "Reserved                                                 ", },
	{ 0x19, "Reserved                                                 ", },
	{ 0x1A, "Reserved                                                 ", },
	{ 0x1B, "Reserved                                                 ", },
	{ 0x1C, "Reserved                                                 ", },
	{ 0x1D, "Reserved                                                 ", },
	{ 0x1E, "Well known logical unit [b]                              ", },
	{ 0x1F, "Unknown or no device type                                ", },
};

//Global variables end here

//Local function proto type

/***************************************************************************
 * name: show_peripheral_dev
 * parameter:
 *      device_code: device_code
 * function: Print device_code description on the log file
 * **************************************************************************/
static void show_peripheral_dev(uint8_t device_code);

/***************************************************************************
 * name: show_peripheral_qualifier
 * parameter:
 *      device_qualifier: device_qualifier
 * function: Print device_qualifier description on the log file
 * **************************************************************************/
static void show_peripheral_qualifier(uint8_t device_qualifier);

/***************************************************************************
 * name: show_RMB
 * parameter:
 *      rmb: rmb
 * function: Print RMB description on the log file
 * **************************************************************************/
static void show_RMB(uint8_t rmb);

/***************************************************************************
 * name: show_dash
 * parameter: Nil
 * function: Print dash on the log file
 * **************************************************************************/
static void show_dash();

/***************************************************************************
 * name: show_general_info
 * parameter:
 *      hdr: sg_io_hdr pointer
 * function: Display the GCIF general info on the log file
 * **************************************************************************/
static void show_general_info(struct sg_io_hdr * hdr);

/***************************************************************************
 * name: show_gcif_sw_ver
 * parameter:
 *      sw_ver: sw version
 * function: Display the GCIF sw version on the log file
 * **************************************************************************/
static void show_gcif_sw_ver(uint32_t sw_ver);

/***************************************************************************
 * name: show_das_info
 * parameter:
 *      hdr: sg_io_hdr pointer
 * function: Display the GCIF information on the log file
 * **************************************************************************/
static void show_das_info(struct sg_io_hdr * hdr);

/***************************************************************************
 * name: show_hdr_outputs
 * parameter:
 *      hdr: sg_io_hdr pointer
 * function: Display the SG IOCTL status on the log file
 * **************************************************************************/
static void show_hdr_outputs(struct sg_io_hdr * hdr);

/***************************************************************************
 * name: show_sense_buffer
 * parameter:
 *      hdr: sg_io_hdr pointer
 * function: Display the sense buffer on the log file
 * **************************************************************************/
static void show_sense_buffer(struct sg_io_hdr * hdr);

/***************************************************************************
 * name: show_vendor
 * parameter:
 *      hdr: sg_io_hdr pointer
 * function: Display the vendor id on the log file
 * **************************************************************************/
static void show_vendor(struct sg_io_hdr * hdr);

/***************************************************************************
 * name: show_product
 * parameter:
 *      hdr: sg_io_hdr pointer
 * function: Display the product id on the log file
 * **************************************************************************/
static void show_product(struct sg_io_hdr * hdr);

/***************************************************************************
 * name: show_product_rev
 * parameter:
 *      hdr: sg_io_hdr pointer
 * function: Display the product version on the log file
 * **************************************************************************/
static void show_product_rev(struct sg_io_hdr * hdr);

/***************************************************************************
 * name: show_inquiry_info
 * parameter:
 *      p_hdr: sg_io_hdr pointer
 * function: Display the inquiry data on the log file
 * **************************************************************************/
static void show_inquiry_info(struct sg_io_hdr * p_hdr);

/***************************************************************************
 * name: show_reboot_reason
 * parameter:
 *      hdr: sg_io_hdr pointer
 * function: Display the reboot reason on the log file
 * **************************************************************************/
static void show_reboot_reason(struct sg_io_hdr * hdr);

/***************************************************************************
 * name: gcif_get_scan_data
 * parameter:
 *      path: scsi generic dev path
 *      das_data_ptr: data buffer pointer to store scan data
 *      das_data_size: size of the buffer to store the scan data
 *      recv_len: received data size from gcif
 * function: Get the scan data from the gcif
 * **************************************************************************/
static int32_t gcif_get_scan_data(char *path, uint8_t *das_data_ptr,
                                  uint64_t das_data_size, uint64_t *recv_len);

/***************************************************************************
 * name: parse_config_param
 * parameter:
 *      config_str:     config string
 *      das_config:     das configuration parameters for the test
 * function: This function will parse the configuration string read from the
 *          gcif.cfg and saved in the das_config structure.
 * **************************************************************************/
static int32_t parse_config_param(char *config_str, das_param_t *das_config);

/***************************************************************************
 * name: gcif_scan_data_dump
 * parameter:
 *      das_data_ptr:       poiter to das data buffer
 *      das_data_size:      das data buffer length
 * function: This function will create formatted file for the scan data
 *          received from the gcif.
 * **************************************************************************/
static void gcif_scan_data_dump(uint8_t *das_data_ptr, uint64_t das_data_size);

/***************************************************************************
 * name: gcif_scan_data_dump_bin
 * parameter:
 *      das_data_ptr:       poiter to das data buffer
 *      das_data_size:      das data buffer length
 * function: This function will create binary file for the scan data
 *          received from the gcif.
 * **************************************************************************/
static void gcif_scan_data_dump_bin(uint8_t *das_data_ptr, uint64_t das_data_size);

//Local function proto type end here

/***************************************************************************
 * name: show_peripheral_dev
 * parameter:
 *		device_code: device_code
 * function: Print device_code description on the log file
 * **************************************************************************/
static void show_peripheral_dev(uint8_t device_code)
{
	LOG_PRINT("Peripheral Device Type: %s ", peripheral_dev_tbl[device_code].device_type);
}

/***************************************************************************
 * name: show_peripheral_qualifier
 * parameter:
 *		device_qualifier: device_qualifier
 * function: Print device_qualifier description on the log file
 * **************************************************************************/
static void show_peripheral_qualifier(uint8_t device_qualifier)
{
	LOG_PRINT("Peripheral Device Qualifier: %d ", device_qualifier);
}

/***************************************************************************
 * name: show_RMB
 * parameter:
 *		rmb: rmb
 * function: Print RMB description on the log file
 * **************************************************************************/
static void show_RMB(uint8_t rmb)
{
	if (rmb & 0x80)
	{
		LOG_PRINT("Removable media ");
	}
	else
	{
		LOG_PRINT("Non removable media ");
	}
}

/***************************************************************************
 * name: show_dash
 * parameter: Nil
 * function: Print dash on the log file
 * **************************************************************************/
static void show_dash()
{
	LOG_PRINT("----------------------------------------------");
}

/***************************************************************************
 * name: show_general_info
 * parameter:
 *		hdr: sg_io_hdr pointer
 * function: Display the GCIF general info on the log file
 * **************************************************************************/
static void show_general_info(struct sg_io_hdr * hdr)
{
	uint8_t * buf = hdr->dxferp;

	LOG_PRINT("------------Standard INQUIRY Output-----------");

	//Peripheral device type
	show_peripheral_dev(buf[0] & 0x1F);
	//Peripheral qualifier
	show_peripheral_qualifier((buf[0] & 0xE0)>>5);
	//RMB
	show_RMB(buf[1]);
	//Version
	LOG_PRINT("Ver:%d ",buf[2]);
	//Response
	LOG_PRINT("Response:%d ",buf[3]);
	//Additional Length
	LOG_PRINT("Additional Length:%d ",buf[4]);
	//Address
	LOG_PRINT("Addr:%d ",buf[6]);
	//Bus info
	LOG_PRINT("Bus Info:%d ",buf[7]);
}

/***************************************************************************
 * name: show_gcif_sw_ver
 * parameter:
 *		sw_ver: sw version
 * function: Display the GCIF sw version on the log file
 * **************************************************************************/
static void show_gcif_sw_ver(uint32_t sw_ver)
{
	uint8_t major = (sw_ver>>24);
	uint8_t minor = (sw_ver>>16 & 0xFF);
	uint16_t build = (sw_ver & 0xFFFF);

	LOG_PRINT("GCIFAD SW Version: %d.%d.%d", major, minor, build );
}

/***************************************************************************
 * name: show_das_info
 * parameter:
 *		hdr: sg_io_hdr pointer
 * function: Display the GCIF information on the log file
 * **************************************************************************/
static void show_das_info(struct sg_io_hdr * hdr)
{
	uint16_t two_byte_val = 0;
	uint32_t four_byte_val = 0;
	uint8_t * buffer = hdr->dxferp;

	two_byte_val = *((uint16_t *) (buffer + 36)); //36
	two_byte_val = ENDIAN_LE16(two_byte_val);
	g_das_info.struct_code = two_byte_val;
	LOG_PRINT("GCIFA SCSI INQ Data Struct Code = %d ", two_byte_val);

	two_byte_val = *((uint16_t *) (buffer + 38)); //38
	two_byte_val = ENDIAN_LE16(two_byte_val);
	g_das_info.struct_size = two_byte_val;
	LOG_PRINT("GCIFA SCSI INQ Data Size = %d ", two_byte_val);

	four_byte_val = *((uint32_t *) (buffer + 40)); //40
	four_byte_val = ENDIAN_LE32(four_byte_val);
	g_das_info.sft_version = four_byte_val;
	show_gcif_sw_ver(four_byte_val);

	//software type
	g_das_info.sft_type = buffer[44];
	switch(g_das_info.sft_type)
	{
		case 0: LOG_PRINT("Software Type: Flash ");
		break;

		case 1: LOG_PRINT("Software Type: Application ");
		break;

		case 2: LOG_PRINT("Software Type: GCIFA Dx ");
		break;

		default: LOG_PRINT("Software Type: Unknown ");
		break;
	}

	//SW download scsi opcode
	g_das_info.sft_dnload_scsi_op_code = buffer[45];
	LOG_PRINT("SW download SCSI OPCode: 0x%0x ", g_das_info.sft_dnload_scsi_op_code);

	//DAS Channel
	two_byte_val = *((uint16_t *) (buffer + 46));
	two_byte_val = ENDIAN_LE16(two_byte_val);
	g_das_info.das_channels = two_byte_val;
	LOG_PRINT("GCIF Channels per ROW= %d ", g_das_info.das_channels);

	//View starting ROW
	g_das_info.view_starting_row = buffer[48];
	LOG_PRINT("GCIF View starting ROW: %d ", g_das_info.view_starting_row);

	//ROWS per view
	g_das_info.rows_per_view = buffer[49];
	LOG_PRINT("GCIF ROWs per View: %d ", g_das_info.rows_per_view);

	//No:Of ROW
	g_das_info.das_rows = buffer[50];
	LOG_PRINT("GCIF No:Of ROW:%d ", g_das_info.das_rows);
}

/***************************************************************************
 * name: show_hdr_outputs
 * parameter:
 *		hdr: sg_io_hdr pointer
 * function: Display the SG IOCTL status on the log file
 * **************************************************************************/
static void show_hdr_outputs(struct sg_io_hdr *hdr)
{
	if(hdr != NULL)
	{
		#ifdef DEBUG
		show_dash();
		LOG_PRINT("SCSI IOCTL hdr status:");
		LOG_PRINT("status:%d", hdr->status);
		LOG_PRINT("masked_status:%d", hdr->masked_status);
		LOG_PRINT("msg_status:%d", hdr->msg_status);
		LOG_PRINT("sb_len_wr:%d", hdr->sb_len_wr);
		LOG_PRINT("host_status:%d", hdr->host_status);
		LOG_PRINT("driver_status:%d", hdr->driver_status);
		LOG_PRINT("resid:%d", hdr->resid);
		LOG_PRINT("duration:%d", hdr->duration);
		LOG_PRINT("info:%d", hdr->info);
		show_dash();
		#endif
	}
}

/***************************************************************************
 * name: show_sense_buffer
 * parameter:
 *		hdr: sg_io_hdr pointer
 * function: Display the sense buffer on the log file
 * **************************************************************************/
static void show_sense_buffer(struct sg_io_hdr * hdr)
{
	uint8_t * buffer = hdr->sbp;
	int32_t i = 0;
	char out_str[256] = {0};

	if (hdr->masked_status)
       	LOG_PRINT("INQUIRY SCSI masked status=0x%x", hdr->masked_status);
	if (hdr->host_status)
		LOG_PRINT("INQUIRY host_status=0x%x", hdr->host_status);
	if (hdr->driver_status)
		LOG_PRINT("INQUIRY driver_status=0x%x", hdr->driver_status);

	if(hdr->sb_len_wr > 0)
	{
		for (i=0; i<hdr->sb_len_wr; ++i)
		{
			//putchar(buffer[i]);
			out_str[i] = buffer[i];
		}
		LOG_PRINT("%s", out_str);
	}
}

/***************************************************************************
 * name: show_vendor
 * parameter:
 *		hdr: sg_io_hdr pointer
 * function: Display the vendor id on the log file
 * **************************************************************************/
static void show_vendor(struct sg_io_hdr * hdr)
{
	uint8_t * buffer = hdr->dxferp;
	uint8_t i = 0;
	char vendor[10] = {0};

	for (i=8; i<16; ++i)
	{
		//putchar(buffer[i]);
		vendor[i-8] = buffer[i];
	}
	LOG_PRINT("Vendor id: %s", vendor);
}

/***************************************************************************
 * name: show_product
 * parameter:
 *		hdr: sg_io_hdr pointer
 * function: Display the product id on the log file
 * **************************************************************************/
static void show_product(struct sg_io_hdr * hdr)
{
	uint8_t * buffer = hdr->dxferp;
	uint8_t i = 0;
	char product[20] = {0};

	for (i=16; i<32; ++i)
	{
		//putchar(buffer[i]);
		product[i-16] = buffer[i];
	}
	LOG_PRINT("Product id: %s", product);
}

/***************************************************************************
 * name: show_product_rev
 * parameter:
 *		hdr: sg_io_hdr pointer
 * function: Display the product version on the log file
 * **************************************************************************/
static void show_product_rev(struct sg_io_hdr * hdr)
{
	uint8_t * buffer = hdr->dxferp;
	uint8_t i = 0;
	char rev[10] = {0};

	for (i=32; i<36; ++i)
	{
		//putchar(buffer[i]);
		rev[i-32] = buffer[i];
	}
	LOG_PRINT("Product ver: %s",rev);
}

/***************************************************************************
 * name: show_inquiry_info
 * parameter:
 *		p_hdr: sg_io_hdr pointer
 * function: Display the inquiry data on the log file
 * **************************************************************************/
static void show_inquiry_info(struct sg_io_hdr * p_hdr)
{
	show_dash();
	show_general_info(p_hdr);
	show_vendor(p_hdr);
	show_product(p_hdr);
	show_product_rev(p_hdr);
	show_das_info(p_hdr);
	show_dash();
}

/***************************************************************************
 * name: show_reboot_reason
 * parameter:
 *		hdr: sg_io_hdr pointer
 * function: Display the reboot reason on the log file
 * **************************************************************************/
static void show_reboot_reason(struct sg_io_hdr * hdr)
{
	uint16_t two_byte_val = 0;
	uint8_t * buffer = hdr->dxferp;

	LOG_PRINT("------------GCIFA-Dx Reboot Reason------------");
	two_byte_val = *((uint16_t *) (buffer));
	two_byte_val = ENDIAN_LE16(two_byte_val);
	LOG_PRINT("Reboot info code:%x", two_byte_val);

	two_byte_val = *((uint16_t *) (buffer+2));
	two_byte_val = ENDIAN_LE16(two_byte_val);
	LOG_PRINT("Size:%x", two_byte_val);

	LOG_PRINT("Reboot reason code:%x", buffer[4]);
	LOG_PRINT("Reboot reason text:%s", (buffer + 5));
}

/***************************************************************************
 * name: gcif_get_scan_data
 * parameter:
 *		path: scsi generic dev path
 *		das_data_ptr: data buffer pointer to store scan data
 *		das_data_size: size of the buffer to store the scan data
 *		recv_len: received data size from gcif
 * function: Get the scan data from the gcif
 * **************************************************************************/
static int32_t gcif_get_scan_data(char *path, uint8_t *das_data_ptr,
                                  uint64_t das_data_size, uint64_t *recv_len)
{
	int32_t status = EXIT_SUCCESS;
	uint8_t *buffer = NULL;


	if((path == NULL) ||(das_data_ptr == NULL) || (recv_len == NULL))
	{
		LOG_ERROR("Invalid pointer passed");
		return EFAULT;
	}

	buffer = calloc(das_data_size, sizeof(uint8_t));
	if(buffer == NULL)
	{
		LOG_ERROR(" Cannot allocated %lu bytes of memory",das_data_size );
		return EFAULT;
	}
	memset(buffer, 0, das_data_size);

	sleep(2);
	LOG_PRINT("Waiting for get scan data...");
	status = SCSI_STATUS_CHECK_CONDITION;

	while((status == SCSI_STATUS_CHECK_CONDITION) && !g_esc_pressed)
	{
		//using SCSI_read_10 for reading scan data from GCIF
		LOG_PRINT("Calling gcif_scsi_read10");

		status = gcif_scsi_read10(path, buffer, das_data_size, recv_len);

		LOG_PRINT("gcif_scsi_read10 status = %d", status);

		if(status == SCSI_STATUS_CHECK_CONDITION)
		{
			LOG_PRINT("Going to sleep 5 Secs");
			sleep(5);
		}
		else
		{
			break;
		}
	}

	if ((status & SG_INFO_OK_MASK) == SG_INFO_OK)
	{
		if(*recv_len <= das_data_size)
		{
			LOG_PRINT("Copying received data...of %lu bytes ", *recv_len);
			memcpy(das_data_ptr, buffer, *recv_len);
		}
		else
		{
			LOG_WARN("Received data size = %lu  buffer size = %lu", *recv_len, das_data_size);
		}
	}
	else
	{
		//Error
		LOG_ERROR("gcif_scsi_read10 failed");
		status = EIO;
	}

	if(buffer)
	{
		free(buffer);
		buffer = NULL;
	}

	return status;
}

/***************************************************************************
 * name: gcif_scsi_read10
 * parameter:
 *		path: scsi generic device path
 *      data_ptr: data buffer pointer
 *      data_size: data buffer size
 *		recv_len: length of the data received
 * function: gcif scan data read using scsi read10 command
 * **************************************************************************/
int32_t gcif_scsi_read10(char * path, uint8_t *data_ptr,
                         uint64_t data_size, uint64_t *recv_len)
{
    int32_t status = EXIT_SUCCESS;

    if((path == NULL) ||(data_ptr == NULL) || (recv_len == NULL))
    {
        LOG_ERROR("Invalid pointer passed");
        return EFAULT;
    }

    struct sg_io_hdr * p_hdr = init_read_io_hdr(data_ptr, data_size);

    if(p_hdr == NULL)
    {
        LOG_ERROR("sg_io_hdr init failed");
        return EFAULT;
    }

    int32_t fd = open(path, O_RDWR);

    if (fd > 0)
    {
        status = execute_test_gcif_scsi_read10(fd, p_hdr);
        LOG_PRINT("scsi read status = %d", status);
        //show_hdr_outputs(p_hdr); // optional

        if(status == SG_INFO_OK)
        {
            // data received in data_ptr
            *recv_len = p_hdr->dxfer_len;
        }

        if ((status & SG_INFO_OK_MASK) != SG_INFO_OK)
        {
            LOG_WARN("Showing Sense buffer");
            show_sense_buffer(p_hdr);
        }
    }
    else
    {
        LOG_ERROR("failed to open sg dev file");
        status = EBADF;
    }

    destroy_io_hdr(p_hdr);

    return status;
}

/***************************************************************************
 * name: gcif_reg_fwopcode
 * parameter:
 *		path: scsi generic dev path
 *		evpd: custom evpd
 *		sw_download_opcode: sw down load opcode
 * function: This function send scsi command to gcif to register fw download
 *			 opcode with gcif
 * **************************************************************************/
int32_t gcif_reg_fwopcode(char * path, uint8_t evpd,
                                         uint8_t sw_download_opcode)
{
	int32_t status = EXIT_SUCCESS;
	struct sg_io_hdr * p_hdr = init_io_hdr();

	if(p_hdr == NULL)
	{
		LOG_ERROR("sg_io_hdr init failed ");
		return EFAULT;
	}

	int32_t exec_status = 0;
	int32_t fd = open(path, O_RDWR);
	if (fd>0)
	{
		exec_status = execute_reg_fw_opcode(fd, sw_download_opcode, evpd, p_hdr);
		LOG_PRINT("the cmd execute status is %d", exec_status);

		if ((exec_status & SG_INFO_OK_MASK) != SG_INFO_OK)
		{
			show_sense_buffer(p_hdr);
		}
		else
		{
			//response
			show_inquiry_info(p_hdr);
		}
		close(fd);
	}
	else
	{
		LOG_ERROR("Failed to open sg file %s", path);
		status = EBADF;
	}

	destroy_io_hdr(p_hdr);
	return status;
}

/***************************************************************************
 * name: gcif_test_unit_ready
 * parameter:
 *		path: scsi generic dev path
 * function: This function send scsi command to gcif to check
 *			 the device is ready for the data acquistion
 * **************************************************************************/
int32_t gcif_test_unit_ready(char * path)
{
	int32_t status = EXIT_SUCCESS;
	struct sg_io_hdr * p_hdr = init_io_hdr();

	if((p_hdr == NULL) || (path == NULL))
	{
		LOG_ERROR("sg_io_hdr init failed or invalid parameter passed");
		return EFAULT;
	}

	int32_t exec_status = 0;
	int32_t fd = open(path, O_RDWR);
	if (fd>0)
	{
		exec_status = execute_test_unit_ready(fd, p_hdr);
		LOG_PRINT("the cmd execute status is %d", exec_status);

		show_hdr_outputs(p_hdr); // optional

		if ((exec_status & SG_INFO_OK_MASK) != SG_INFO_OK)
		{
			show_sense_buffer(p_hdr);
		}
		else
		{
			//response
			LOG_PRINT("Test unit ready");
		}
		close(fd);
	}
	else
	{
		LOG_ERROR("Failed to open sg file path:%s", path);
		status = EBADF;
	}

	destroy_io_hdr(p_hdr);
	return status;
}

/***************************************************************************
 * name: gcif_startup
 * parameter:
 *		path: scsi generic dev path
 *		view_starting_row: view number ;default is zero
 *		rows_per_view:	rows per view 4 or 16
 *		das_channels: das channels
 * function: This function send scsi command to gcif for configuring the gcif
 *			for the data acquisition
 * **************************************************************************/
int32_t gcif_startup(char * path, uint8_t view_starting_row,
								  uint8_t rows_per_view, uint16_t das_channels)
{
	int32_t status = EXIT_SUCCESS;
	struct sg_io_hdr * p_hdr = init_io_hdr();

	if((p_hdr == NULL) || (path == NULL))
	{
		LOG_ERROR("sg_io_hdr init failed or invalid parameter passed");
		return EFAULT;
	}

	int32_t exec_status = 0;
	int32_t fd = open(path, O_RDWR);
	if (fd>0)
	{
		exec_status = execute_test_gcif_startup(fd, p_hdr, view_starting_row,
												 rows_per_view, das_channels);
		LOG_PRINT("the cmd execute status is %d", exec_status);

		//show_hdr_outputs(p_hdr); // optional

		if ((exec_status & SG_INFO_OK_MASK) != SG_INFO_OK)
		{
			show_sense_buffer(p_hdr);
		}
		else
		{
			//response
			LOG_PRINT("GCIF startup success.");

		}
		close(fd);
	}
	else
	{
		LOG_ERROR("Failed to open sg file path:%s", path);
		status = EBADF;
	}

	destroy_io_hdr(p_hdr);
	return status;
}

/***************************************************************************
 * name: gcif_start_scan
 * parameter:
 *		path: scsi generic dev path
 *		p_das_config: das configuration params for the test
 *		p_das_data:	das data buffer pointer
 *		das_data_size: allocated buffer size for das data
 *		recv_data_size: received data size
 * function: This function send scsi command to gcif for starting the data
 *			acquisition on the gcif
 * **************************************************************************/
int32_t gcif_start_scan(char *path, das_param_t *p_das_config,
							   uint8_t *p_das_data, uint64_t das_data_size,
							   uint64_t *recv_data_size)
{
	int32_t status = EXIT_SUCCESS;

	if((path == NULL) || (p_das_config == NULL) ||
	   (p_das_data == NULL) || (recv_data_size == NULL))
	{
		LOG_ERROR("Invalid pointer passed");
		return EFAULT;
	}
	struct sg_io_hdr * p_hdr = init_io_hdr();

	if(p_hdr == NULL)
	{
		LOG_ERROR("sg_io_hdr init failed ");
		return EFAULT;
	}

	int32_t exec_status = 0;
	int32_t fd = open(path, O_RDWR);
	if (fd>0)
	{
		exec_status = execute_test_gcif_start_scan(fd, p_hdr, p_das_config);

		LOG_PRINT("the cmd execute status is %d", exec_status);

		//show_hdr_outputs(p_hdr); // optional

		if ((exec_status & SG_INFO_OK_MASK) != SG_INFO_OK)
		{
			show_sense_buffer(p_hdr);
		}
		else
		{
			LOG_PRINT("GCIF start scan success");
			close(fd);
			//get das data here. get data from gcifa
			status = gcif_get_scan_data(path, p_das_data, das_data_size, recv_data_size);
			if (status != EXIT_SUCCESS)
			{
				LOG_ERROR("get scan data failed!!");
			}
		}
	}
	else
	{
		LOG_ERROR("Failed to open sg file path:%s", path);
		status = EBADF;
	}

	destroy_io_hdr(p_hdr);
	return status;
}

/***************************************************************************
 * name: test_execute_inquiry
 * parameter:
 *		path:		scsi generic dev path
 *		evpd:		custom evpd for this command
 *		page_code:	custom page code for this command
 * function: This function send scsi command to gcif for getting the inquiry
 *			data from gcif target
 * **************************************************************************/
int32_t test_execute_inquiry(char * path, uint8_t evpd, uint8_t page_code)
{
	int32_t status = EXIT_SUCCESS;
	struct sg_io_hdr * p_hdr = init_io_hdr();

	if((p_hdr == NULL) || (path == NULL))
	{
		LOG_ERROR("sg_io_hdr init failed or invalid parameter passed");
		return EFAULT;
	}

	int32_t exec_status = 0;
	int32_t fd = open(path, O_RDWR);
	if (fd>0)
	{
		exec_status = execute_inquiry(fd, page_code, evpd, p_hdr);
		LOG_PRINT("the cmd execute status is %d", exec_status);

		//show_hdr_outputs(p_hdr); // optional
		if ((exec_status & SG_INFO_OK_MASK) != SG_INFO_OK)
		{
			show_sense_buffer(p_hdr);
		}
		else
		{
			show_inquiry_info(p_hdr);
		}
		close(fd);
	}
	else
	{
		LOG_ERROR("Failed to open sg file path:%s", path);
		status = EBADF;
	}

	destroy_io_hdr(p_hdr);

	return status;
}

/***************************************************************************
 * name: test_execute_reboot_reason
 * parameter:
 *		path:		scsi generic dev path
 *		evpd:		custom evpd for this command
 *		page_code:	custom page code for this command
 * function: This function send scsi command to gcif for getting the reboot
 *			reason information from gcif
 * **************************************************************************/
int32_t test_execute_reboot_reason(char * path, uint8_t evpd, uint8_t page_code)
{
	int32_t status = EXIT_SUCCESS;
	struct sg_io_hdr * p_hdr = init_io_hdr();

	if((p_hdr == NULL) || (path == NULL))
	{
		LOG_ERROR("sg_io_hdr init failed or invalid parameter passed");
		return EFAULT;
	}

	int32_t exec_status = 0;
	int32_t fd = open(path, O_RDWR);
	if (fd>0)
	{
		exec_status = execute_inquiry(fd, page_code, evpd, p_hdr);
		LOG_PRINT("the cmd execute status is %d", exec_status);

		//show_hdr_outputs(p_hdr); // optional
		if ((exec_status & SG_INFO_OK_MASK) != SG_INFO_OK)
		{
			show_sense_buffer(p_hdr);
		}
		else
		{
			show_dash();
			//reboot reason
			show_reboot_reason(p_hdr);
			show_dash();
		}
		close(fd);
	}
	else
	{
		LOG_ERROR("Failed to open sg file path:%s", path);
		status = EBADF;
	}

	destroy_io_hdr(p_hdr);
	return status;
}

/***************************************************************************
 * name: parse_config_param
 * parameter:
 *		config_str:		config string
 *		das_config:		das configuration parameters for the test
 * function: This function will parse the configuration string read from the
 *			gcif.cfg and saved in the das_config structure.
 * **************************************************************************/
static int32_t parse_config_param(char *config_str, das_param_t *das_config)
{
   	const char delim[2] = ":";
   	char *token = NULL;
   	char *ptr = NULL;
   	char param_str[80] = {'\0'};
   	char value_str[32] = {'\0'};
   	uint32_t val = 0;

	if((config_str == NULL) || (das_config == NULL))
	{
		LOG_ERROR("Invalid parameter passed");
		return EXIT_FAILURE;
	}

	/* get the first token */
   	token = strtok(config_str, delim);

   	if(token != NULL)
   	{
   		snprintf( param_str, sizeof(param_str),"%s", token);
   	}

	token = strtok(NULL, delim);
	if(token != NULL)
   	{
   		snprintf( value_str,sizeof(value_str), "%s", token );
   		val = strtoul(value_str, &ptr, 10);
   	}

   	//printf( "p = %s\n", param_str );
   	//printf( "v =  %s\n", value_str );

   	//"daschannels"
   	if((strcmp(DAS_CHANNEL_STR,param_str)) == 0)
   	{
   		das_config->das_channels = val;
   	}
   	//rowsperview
   	else if((strcmp(ROWS_PER_VIEW_STR,param_str)) == 0)
   	{
   		// 4 and 16 supported
   		das_config->rows_per_view = val;
   	}
   	//viewstartrow
   	else if((strcmp(VIEW_START_ROW_STR,param_str)) == 0)
   	{
   		das_config->view_starting_row = val;
   	}
   	//viewspertx
   	else if((strcmp(VIEW_PER_TX,param_str)) == 0)
   	{
   		// must be <=32 as per GCIFA-D3_SDD.docx
   		das_config->views_per_tx = (val > 32) ? 32 : val;
   	}
   	//simulation
   	else if((strcmp(SIMULATION_STR,param_str)) == 0)
   	{
   		// 0 and 1 are acceptable
   		das_config->flags = (val > 1) ? 1 : val;
   	}
	//viewsperscan
   	else if((strcmp(VIEWS_PERSCAN_STR,param_str)) == 0)
   	{
   		das_config->views_per_scan = val;
   	}
   	//viewspersec
   	else if((strcmp(VIEWS_PER_SEC_STR,param_str)) == 0)
   	{
   		das_config->views_per_sec = val;
   	}

	return EXIT_SUCCESS;
}

/***************************************************************************
 * name: get_gcif_config
 * parameter:
 *		das_config:		das configuration parameters for the test
 * function: This function will read the configuration parameters
 *			 from the gcif.cfg file
 * **************************************************************************/
int32_t get_gcif_config(das_param_t *das_config)
{
	char str[80] = {'\0'};
	int32_t ret = EXIT_SUCCESS;

	if(das_config == NULL)
	{
		LOG_ERROR("Invalid parameter passed");
		return EFAULT;
	}

	FILE *fp = fopen("gcif.cfg", "r");
    if(fp == NULL)
    {
        LOG_ERROR("Unable to open gcif.cfg! \n");
        return EXIT_FAILURE;
    }

    while(fgets(str, sizeof(str), fp) != NULL)
    {
        ret = parse_config_param(str, das_config);
        if(ret != EXIT_SUCCESS)
        {
        	LOG_ERROR("Config file parse error");
        }
    }

    if(fp)
    {
    	fclose(fp);
	}

   	return ret;
}

/***************************************************************************
 * name: show_gcif_config
 * parameter:
 *		das_config:		das configuration parameters for the test
 * function: This function will display the configuration parameters read
 *			 from the gcif.cfg file
 * **************************************************************************/
void show_gcif_config(das_param_t *das_config)
{
	if(das_config != NULL)
	{
		LOG_PRINT("rows_per_view = %d", das_config->rows_per_view);
		LOG_PRINT("das_channels = %d", das_config->das_channels);
		LOG_PRINT("views_per_tx = %d", das_config->views_per_tx);
		LOG_PRINT("flags = %d", das_config->flags);
		LOG_PRINT("views_per_scan = %d", das_config->views_per_scan);
		LOG_PRINT("views_per_sec = %d", das_config->views_per_sec);
		LOG_PRINT("view_starting_row = %d", das_config->view_starting_row);
	}
}

/***************************************************************************
 * name: gcif_scan_and_verify
 * parameter:
 * 		scsi_dev_path:			scsi genereic device path
 *		das_config_param:		das configuration parameters for the test
 * function: This function trigger the data acquisition in the gcif and get
 *			the scan data from the gcif. After that it will compare with the
 *			golden data and report success/failure
 * **************************************************************************/
int32_t gcif_scan_and_verify(char * scsi_dev_path, das_param_t *das_config_param)
{
	int32_t ret = EXIT_SUCCESS;
	uint64_t das_data_size = 0;
	uint8_t *das_data_ptr = NULL;
	uint64_t recv_data_size = 0;
	uint8_t evpd = 0;

	#if 0
	//Need to provide gcif.cfg file for reading this config values
	das_config_param->rows_per_view = DAS_ROWS_PER_VIEW; //16
	das_config_param->das_channels = DAS_CHANNELS; //786
	das_config_param->views_per_tx = DAS_VIEWS_PER_TRANSFER; //8
	das_config_param->flags = DAS_SIMULATION;
	das_config_param->views_per_scan = DAS_VIEWS_PER_SCAN; //900
	das_config_param->views_per_sec = DAS_VIEWS_PER_SEC; //1890
	das_config_param->view_starting_row = DAS_VIEW_START_ROW;
	#endif

	if((scsi_dev_path == NULL) || (das_config_param == NULL))
	{
		LOG_ERROR("Invalid pointer passed ");
		return EFAULT;
	}

	if(!g_start_flag)
	{
		g_start_flag = 1;

		// Send SCSI test unit ready cmd
		ret = gcif_test_unit_ready(scsi_dev_path);
		if (ret != EXIT_SUCCESS)
		{
			LOG_ERROR(" Error in gcif test unit ready!!");
			return ret;
		}
		// Send SCSI inquiry cmd with extended info
		// for registering sw download opcode with GCIFA
		evpd = 3;
		ret = gcif_reg_fwopcode(scsi_dev_path, evpd, SCSI_GCIF_DOWNLOAD);
		if (ret != EXIT_SUCCESS)
		{
			LOG_ERROR("Error in reg fwopcode!!");
			return ret;
		}

		ret = gcif_startup(scsi_dev_path,
		                                das_config_param->view_starting_row,
				                        das_config_param->rows_per_view,
				                        das_config_param->das_channels);
	}

	if (ret != EXIT_SUCCESS)
	{
		LOG_ERROR(" Error in gcif startup!!");
		return ret;
	}

	//buf size = no:of views*16*788*2 = nv*24KB
	uint32_t num_dets =  (((das_config_param->das_channels + 3) / 4) * 4);

	das_data_size = (das_config_param->views_per_scan *
	                 das_config_param->rows_per_view *
	                 num_dets * 2);

	//allocated buffer for das scan data
	das_data_ptr = calloc(das_data_size, sizeof(uint8_t));
	if (das_data_ptr == NULL)
	{
		LOG_ERROR("Cannot allocate %lu bytes of memory!! breaking",
				  das_data_size);
		return EFAULT;
	}

	ret = gcif_start_scan(scsi_dev_path, das_config_param,
	                      das_data_ptr, das_data_size, &recv_data_size);

	if (ret != EXIT_SUCCESS)
	{
		LOG_ERROR("Error in gcif start scanning!!");
		goto exit;
	}

	#if 0
	dummy_das_scan_data(das_data_ptr, das_data_size,
						das_config_param.views_per_scan,
						das_config_param.rows_per_view,
						das_config_param.das_channels,
						das_config_param.views_per_sec);
	#endif
	if(recv_data_size > 0)
	{
		ret = verify_gcif_data(das_data_ptr, das_data_size, das_config_param);

		if (ret != EXIT_SUCCESS)
		{
			LOG_ERROR("Data verification failed");
			// create dump for the gcif scan data
			LOG_PRINT("Error dump created ./log/error_scan_data.dump");
			gcif_scan_data_dump(das_data_ptr, das_data_size);
			gcif_scan_data_dump_bin(das_data_ptr, das_data_size);
		}
		else
		{
			LOG_PRINT("Scan data verification completed successfully.");
		}
	}
	else
	{
		ret = EXIT_FAILURE;
		LOG_ERROR("Received data size is zero");
		LOG_ERROR("!!Miss match found at: view number=0  index=0 buffer location=0");
		printf("!!Miss match found at: view number=0  buffer location=0 \n");
	}

	exit:

	if(das_data_ptr)
	{
		free(das_data_ptr);
		das_data_ptr = NULL;
	}

	return ret;
}

/***************************************************************************
 * name: gcif_scan_data_dump
 * parameter:
 * 		das_data_ptr:		poiter to das data buffer
 *		das_data_size:		das data buffer length
 * function: This function will create formatted file for the scan data
 *			received from the gcif.
 * **************************************************************************/
static void gcif_scan_data_dump(uint8_t *das_data_ptr, uint64_t das_data_size)
{
	FILE *fp;
	char str[512] = {0};
    char data_str[128] = {0};
    uint16_t data = 0;
    uint64_t i = 0;

	if(das_data_ptr != NULL)
	{

	    sprintf(str,"%s\n", DUMP_U_LINE);

	    sprintf(data_str, "%s\n", DUMP_HEADER);
	    strcat(str, data_str);
	    memset(data_str, 0, sizeof(data_str));

	    sprintf(data_str, "%s\n", DUMP_U_LINE);
	    strcat(str, data_str);
	    memset(data_str, 0, sizeof(data_str));

	    sprintf(data_str," %08lu    ", i);
	    strcat(str, data_str);
	    memset(data_str, 0, sizeof(data_str));

		fp = fopen ("./log/scan_data_error.dump","w");
		if(fp)
		{

			for(i = 0; i < das_data_size; ( i = i + sizeof(uint16_t) ))
		    {
		        if(((i % 16) == 0x0) && (i > 0))
		        {
		            fprintf(fp, "%s\n", str);
		            memset(str, 0, sizeof(str));
                	sprintf(str," %08lu    ", i);
		        }
		        data = *((uint16_t *)(das_data_ptr+i));
		        //For making data representation matching to the binary view
            	data = ENDIAN_LE16(data);
		        sprintf(data_str, "%04x  ", data);
		        strcat(str,data_str);
		        memset(data_str, 0, sizeof(data_str));
		    }
		    //check last partial data printed or not.
		    if(i > 0)
	        {
	            fprintf(fp, "%s\n", str);
	            memset(str, 0, sizeof(str));
	        }

		    fclose(fp);
		}
		else
		{
			LOG_ERROR("Could not able to open file");
		}
	}
	else
	{
		LOG_ERROR("Invalid pointer passed");
	}
}

/***************************************************************************
 * name: gcif_scan_data_dump_bin
 * parameter:
 * 		das_data_ptr:		poiter to das data buffer
 *		das_data_size:		das data buffer length
 * function: This function will create binary file for the scan data
 *			received from the gcif.
 * **************************************************************************/
static void gcif_scan_data_dump_bin(uint8_t *das_data_ptr, uint64_t das_data_size)
{
	int32_t fd = 0;
    uint64_t len = 0;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

	if(das_data_ptr != NULL)
	{
		fd = open("./log/scan_data_error.bin", O_RDWR | O_CREAT, mode);
		if(fd)
		{
			len = write(fd, das_data_ptr, das_data_size);
	        if(len != das_data_size)
	        {
	            LOG_WARN("Write length not match with buf size."
	                     "write len = %lu buf size = %lu ", len, das_data_size);
	        }

		    close(fd);
		}
		else
		{
			LOG_ERROR("Could not able to open file");
		}
	}
	else
	{
		LOG_ERROR("Invalid pointer passed");
	}
}

/***************************************************************************
 * name: set_xfer_data
 * parameter:
 * 		p_hdr:		poiter to sg_io_hdr struct
 *		data:		pointer to the data transfer buffer
 *		length:		size of the data transfer buffer
 * function: Set the data transfer buffer and its size in the sg_io_hdr for the
 *			sg_io_hdr comiplation
 * **************************************************************************/
int32_t set_xfer_data(struct sg_io_hdr * p_hdr, void * data, uint32_t length)
{
	if ((p_hdr == NULL) || (data == NULL))
	{
		LOG_ERROR("Invalid pointer passed ");
		return EFAULT;
	}
	else
	{
		p_hdr->dxferp = data;
		p_hdr->dxfer_len = length;
	}
	return EXIT_SUCCESS;
}

/***************************************************************************
 * name: set_sense_data
 * parameter:
 * 		p_hdr:		poiter to sg_io_hdr struct
 *		data:		pointer to the sense buffer
 *		length:		size of the sense buffer
 * function: Set the sense buffer and its size in the sg_io_hdr for the
 *			sg_io_hdr comiplation
 * **************************************************************************/
int32_t set_sense_data(struct sg_io_hdr * p_hdr, uint8_t * data,
		uint32_t length)
{
	if ((p_hdr == NULL) || (data == NULL))
	{
		LOG_ERROR(" Invalid pointer passed ");
		return EFAULT;
	}
	else
	{
		p_hdr->sbp = data;
		p_hdr->mx_sb_len = length;
	}
	return EXIT_SUCCESS;
}

/***************************************************************************
 * name: init_io_hdr
 * parameter:
 * function: initialize the sg_io_hdr struct fields with the most common
 * 			 value
 * **************************************************************************/
struct  sg_io_hdr * init_io_hdr()
{
	struct sg_io_hdr *p_scsi_hdr = NULL;
	int32_t status = EXIT_SUCCESS;

	p_scsi_hdr = (struct sg_io_hdr *)malloc(sizeof(struct sg_io_hdr));
	if (p_scsi_hdr)
	{
		memset(p_scsi_hdr, 0, sizeof(struct sg_io_hdr));
		p_scsi_hdr->interface_id = 'S'; 			/* this is the only choice we have! */
		p_scsi_hdr->flags = (SG_FLAG_LUN_INHIBIT | SG_FLAG_DIRECT_IO);//SG_FLAG_LUN_INHIBIT; 	/* this would put the LUN to 2nd byte of cdb*/
		p_scsi_hdr->dxfer_direction = SG_DXFER_TO_FROM_DEV;//SG_DXFER_FROM_DEV;
		p_scsi_hdr->timeout = 20000;     			/* 20000 millisecs == 20 seconds */

		//status = set_xfer_data(p_scsi_hdr, g_data_buffer, BLOCK_LEN*256);
		status = set_xfer_data(p_scsi_hdr, g_data_buffer, DATA_LEN);
		if (status != EXIT_SUCCESS)
		{
			LOG_ERROR("set_xfer_data returned error");
			free(p_scsi_hdr);
			p_scsi_hdr = NULL;
			return NULL;
		}

		status = set_sense_data(p_scsi_hdr, g_sense_buffer, SENSE_LEN);
		if (status != EXIT_SUCCESS)
		{
			LOG_ERROR("set_sense_data returned error");
			free(p_scsi_hdr);
			p_scsi_hdr = NULL;
			return NULL;
		}
	}
	else
	{
		LOG_ERROR("Memory allocation failed ");
	}

	return p_scsi_hdr;
}

/***************************************************************************
 * name: init_read_io_hdr
 * parameter:
 *		das_data_ptr: data buffer pointer
 *		das_data_size: buffer size
 * function: initialize the sg_io_hdr struct fields with the most common
 * 			 value
 * **************************************************************************/
struct  sg_io_hdr * init_read_io_hdr(uint8_t *das_data_ptr, uint64_t das_data_size)
{
	struct sg_io_hdr *p_scsi_hdr = NULL;
	int32_t status = EXIT_SUCCESS;
	uint8_t cdb[16] = {0};

	if(das_data_ptr == NULL)
	{
		LOG_ERROR("Invalid pointer passed");
		return NULL;
	}

	cdb[0] = SCSI_GCIF_RAW_DATA;

	p_scsi_hdr = (struct sg_io_hdr *)malloc(sizeof(struct sg_io_hdr));
	if (p_scsi_hdr)
	{
		memset(p_scsi_hdr, 0, sizeof(struct sg_io_hdr));
		p_scsi_hdr->interface_id = 'S'; 			/* this is the only choice we have! */
		p_scsi_hdr->flags = (SG_FLAG_LUN_INHIBIT | SG_FLAG_DIRECT_IO);
		p_scsi_hdr->dxfer_direction = SG_DXFER_FROM_DEV;
		p_scsi_hdr->timeout = 20000;     			/* 20000 millisecs == 20 seconds */
		p_scsi_hdr->cmdp = cdb;
		p_scsi_hdr->cmd_len = sizeof(cdb);

		status = set_xfer_data(p_scsi_hdr, das_data_ptr, das_data_size);
		if (status != EXIT_SUCCESS)
		{
			free(p_scsi_hdr);
			p_scsi_hdr = NULL;
			LOG_ERROR("set_xfer_data returns error!! ");
			return NULL;
		}

		status = set_sense_data(p_scsi_hdr, g_sense_buffer, SENSE_LEN);
		if (status != EXIT_SUCCESS)
		{
			free(p_scsi_hdr);
			p_scsi_hdr = NULL;
			LOG_ERROR("set_sense_data returns error!! ");
			return NULL;
		}
	}
	else
	{
		LOG_ERROR(" Memory allocation failed");
	}

	return p_scsi_hdr;
}

/***************************************************************************
 * name: destroy_io_hdr
 * parameter:
 * 		p_hdr:		poiter to sg_io_hdr struct
 * function: Delete the p_hdr pointer
 * **************************************************************************/
void destroy_io_hdr(struct sg_io_hdr * p_hdr)
{
	if (p_hdr)
	{
		free(p_hdr);
		p_hdr = NULL;
	}
	else
	{
		LOG_ERROR("Invalid pointer passed");
	}
}

/***************************************************************************
 * name: execute_inquiry
 * parameter:
 * 		fd:			file descripter
 * 		page_code:	cdb page code
 * 		evpd:		cdb evpd
 * 		p_hdr:		poiter to sg_io_hdr struct
 * function: make Inquiry cdb and execute it.
 * **************************************************************************/
int32_t execute_inquiry(int32_t fd, uint8_t page_code, uint8_t evpd,
						struct sg_io_hdr * p_hdr)
{
	uint8_t cdb[6];

	if(p_hdr == NULL)
	{
		LOG_ERROR("Invalid pointer passed ");
		return EFAULT;
	}

	/* set the cdb format */
	cdb[0] = SCSI_INQUIRY_CMD; /*This is for Inquery - 0x12*/
	cdb[1] = evpd & 1;
	cdb[2] = page_code & 0xff;
	cdb[3] = 0; //rsvd
	cdb[4] = 0xff; //size
	cdb[5] = 0; /*For control filed, just use 0*/

	p_hdr->dxfer_direction = SG_DXFER_FROM_DEV;
	p_hdr->cmdp = cdb;
	p_hdr->cmd_len = sizeof(cdb);

	int ret = ioctl(fd, SG_IO, p_hdr);
	if (ret<0) {
		LOG_ERROR("Sending SCSI Command failed.");
		close(fd);
		return EIO;
	}

	return p_hdr->status;
}

/***************************************************************************
 * name: execute_reg_fw_opcode
 * parameter:
 * 		fd:			file descripter
 * 		page_code:	cdb page code
 * 		evpd:		cdb evpd
 * 		p_hdr:		poiter to sg_io_hdr struct
 * function: make register fw opcode cdb and execute it.
 * **************************************************************************/
int32_t execute_reg_fw_opcode(int32_t fd, uint8_t page_code, uint8_t evpd,
							  struct sg_io_hdr * p_hdr)
{
	uint8_t cdb[6];

	if(p_hdr == NULL)
	{
		LOG_ERROR("Invalid pointer passed ");
		return EFAULT;
	}

	/* set the cdb format */
	cdb[0] = SCSI_INQUIRY_CMD; /*This is for Inquery -0x12*/
	cdb[1] = evpd & 3;
	cdb[2] = page_code & 0xff;
	cdb[3] = 0xaa; //rsvd
	cdb[4] = 0xff; //size
	cdb[5] = 0; /*For control filed, just use 0*/

	p_hdr->dxfer_direction = SG_DXFER_FROM_DEV;
	p_hdr->cmdp = cdb;
	p_hdr->cmd_len = sizeof(cdb);

	int ret = ioctl(fd, SG_IO, p_hdr);
	if (ret<0) {
		LOG_ERROR("Sending SCSI Command failed.");
		close(fd);
		return EIO;
	}

	return p_hdr->status;
}

/***************************************************************************
 * name: execute_test_unit_ready
 * parameter:
 *      fd:         file descripter
 *      p_hdr:      poiter to sg_io_hdr struct
 * function: make test unit ready cdb and execute it.
 * **************************************************************************/
int32_t execute_test_unit_ready(int32_t fd, struct sg_io_hdr * p_hdr)
{
	uint8_t cdb[6];

	if(p_hdr == NULL)
	{
		LOG_ERROR(" Invalid pointer passed ");
		return EFAULT;
	}

	/* set the cdb format */
	cdb[0] = SCSI_TEST_UNIT_READY; /*This is for test unit ready - 0x0*/
	cdb[1] = 0x0;
	cdb[2] = 0x0;
	cdb[3] = 0x0;
	cdb[4] = 0x0;
	cdb[5] = 0x0; /*For control filed, just use 0*/

	p_hdr->dxfer_direction = SG_DXFER_NONE;
	p_hdr->cmdp = cdb;
	p_hdr->cmd_len = sizeof(cdb);

	int ret = ioctl(fd, SG_IO, p_hdr);
	if (ret<0) {
		LOG_ERROR("Sending SCSI Command failed.");
		close(fd);
		return EIO;
	}

	return p_hdr->status;
}

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
                                  uint16_t das_channels)
{
	uint8_t cdb[16] = {0};

	if(p_hdr == NULL)
	{
		LOG_ERROR("Invalid pointer passed ");
		return EFAULT;
	}

	/* set the cdb format */
	cdb[0] = SCSI_GCIF_STARTUP; /*This is for gcif startup-0x22*/
	cdb[1] = view_start_row;
	cdb[2] = rows_per_view;
    *((uint16_t *) (cdb + 3)) = ENDIAN_BE16(das_channels);
	cdb[5] = 0x0;// cdb 5 to 15 reserved

	p_hdr->dxfer_direction = SG_DXFER_TO_DEV;
	p_hdr->cmdp = cdb;
	p_hdr->cmd_len = sizeof(cdb);

	int ret = ioctl(fd, SG_IO, p_hdr);
	if (ret<0) {
		LOG_ERROR("Sending SCSI Command failed.");
		close(fd);
		return EIO;
	}

	return p_hdr->status;
}

/***************************************************************************
 * name: execute_test_gcif_start_scan
 * parameter:
 *      fd:                 file descripter
 *      p_hdr:              poiter to sg_io_hdr struct
 *		p_das_config:		pointer to das_param_t struct
 * function: make gcif start scan cdb and execute it.
 * **************************************************************************/
int32_t execute_test_gcif_start_scan(int32_t fd, struct sg_io_hdr * p_hdr,
                                     das_param_t *p_das_config)
{
	uint8_t cdb[16] = {0};

	if((p_hdr == NULL) || (p_das_config == NULL))
	{
		LOG_ERROR(" Invalid pointer passed ");
		return EFAULT;
	}

	#ifdef DEBUG
	LOG_PRINT("views_per_tx = %d ", p_das_config->views_per_tx);
	LOG_PRINT("flags = %d ", p_das_config->flags);
	LOG_PRINT("views_per_scan = %d ", p_das_config->views_per_scan);
	LOG_PRINT("views_per_sec = %d ", p_das_config->views_per_sec);
	#endif

	/* set the cdb format */
	cdb[0] = SCSI_GCIF_START_SCANNING; /*This is for gcif start scanning - 0x24*/
	cdb[1] = p_das_config->views_per_tx;
	cdb[2] = p_das_config->flags;
	cdb[3] = 0; // reserved
    *((uint32_t *) (cdb + 4)) = ENDIAN_BE32(p_das_config->views_per_scan);
    *((uint32_t *) (cdb + 8)) = ENDIAN_BE32(p_das_config->views_per_sec);
    // cdb[12 - 15] reserved

	p_hdr->dxfer_direction = SG_DXFER_TO_DEV;
	p_hdr->cmdp = cdb;
	p_hdr->cmd_len = sizeof(cdb);

	int ret = ioctl(fd, SG_IO, p_hdr);
	if (ret<0) {
		LOG_ERROR("Sending SCSI Command failed.");
		close(fd);
		return EIO;
	}

	return p_hdr->status;
}

/***************************************************************************
 * name: execute_test_gcif_scsi_read10
 * parameter:
 *      fd:                 file descripter
 *      p_hdr:              poiter to sg_io_hdr struct
 * function: get the scan data using scsi read10 method
 * **************************************************************************/
int32_t execute_test_gcif_scsi_read10(int32_t fd, struct sg_io_hdr * p_hdr)
{
	uint8_t cdb[16] = {0};

	if(p_hdr == NULL)
	{
		LOG_ERROR(" Invalid pointer passed ");
		return EFAULT;
	}
//p_hdr->dxfer_len
	/* set the cdb format */
	cdb[0] = SCSI_READ_10; /*This is for scsi read 10 opcode - 0x28*/
	cdb[1] = 0; //RSVD
	cdb[2] = 0; //LBA MSB
	cdb[3] = 0;
	cdb[4] = 0;
	cdb[5] = 0; //LBA LSB
	cdb[6] = 0; //GRP Number
	cdb[7] = 0; //TL MSB
	cdb[8] = 1; //TL LSB
	cdb[9] = 0; //ctrl

    // cdb[10 - 15] reserved
	p_hdr->dxfer_direction = SG_DXFER_FROM_DEV;
	p_hdr->cmdp = cdb;
	p_hdr->cmd_len = sizeof(cdb);

	int ret = ioctl(fd, SG_IO, p_hdr);
	if (ret<0) {
		LOG_ERROR("Sending SCSI Command failed.");
		close(fd);
		return EIO;
	}

	return p_hdr->status;
}
