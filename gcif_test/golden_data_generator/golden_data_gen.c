#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "golden_data_gen.h"

//#define DEBUG // Comment if not required
//70+4+8
    #define U_LINE      "+------------------------------------------------------------------------------------------+"
    #define HEADER      "| Index   |                           Golden Data                                          |"
  //#define U_LINE      "--------------------------------------------------------------------------------------------"
    #define HEADER_1    "|         |      View number:%4d  Number of row:%4d  Data rate:%4d                      |"

#define DAS_CHANNEL_STR         "daschannels"
#define ROWS_PER_VIEW_STR       "rowsperview"
#define VIEW_START_ROW_STR      "viewstartrow"
#define VIEW_PER_TX             "viewspertx"
#define SIMULATION_STR          "simulation"
#define VIEWS_PERSCAN_STR       "viewsperscan"
#define VIEWS_PER_SEC_STR       "viewspersec"

//LOcal prototype 
/***************************************************************************
 * name: parse_config_param
 * parameter:
 *      config_str:     config string 
 *      das_config:     das configuration parameters for the test   
 * function: This function will parse the configuration string read from the 
 *          gcif.cfg and saved in the das_config structure. 
 * **************************************************************************/
static int32_t parse_config_param(char *config_str, das_param_t *das_config);

//End local prototype

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
                              uint32_t das_channels)
{
    uint32_t s,d;
    uint8_t repeat = 4;
    uint64_t val = 0;
    uint64_t x = 0;
    uint32_t num_dets =  (((das_channels + 3) / 4) * 4);

    #ifdef DEBUG
    printf("-----------------------------------------------------\n");
    printf("------------Data pattern for <1000 Views/Sec---------\n");
    printf("-----------------------------------------------------\n");
    #endif

    if(dest_buf_ptr == NULL)
    {
        printf("Invalid pointer passed\n");
        return EFAULT;
    }

    for ( s = 0; s < num_segs; s++ )
    {
        for ( d = 0;  d < num_dets;  d += repeat )
        {
            x = (view_num + s%16 + d/4) & 0xFF;

            val = ((x << 56) | (x << 48) | (x << 40) |
                  (x << 32) | (x << 24) | (x << 16) | (x << 8)| (x));

            memcpy((uint64_t *)dest_buf_ptr, &val, sizeof(val));
            dest_buf_ptr +=sizeof(val);
            #ifdef DEBUG
	            printf("row=%d das channel=%d --> %016lx \n", s, d, val);
            	printf("-----------------------------------------------------\n");
            #endif
        }
    }

    return EXIT_SUCCESS;
}

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
                              uint32_t das_channels)
{
    uint32_t s,d;
    uint8_t repeat = 2;
    uint32_t val = 0;
    uint32_t x = 0;
    uint32_t num_dets =  (((das_channels + 3) / 4) * 4);

    #ifdef DEBUG
    printf("-----------------------------------------------------\n");
    printf("------------Data pattern for <1000 Views/Sec---------\n");
    printf("-----------------------------------------------------\n");
    #endif

    if(dest_buf_ptr == NULL)
    {
        printf("Invalid pointer passed\n");
        return EFAULT;
    }

    for ( s = 0; s < num_segs; s++ )
    {
        //base_line_byte = (view_num + s%16) & 0xFF;
        for ( d = 0;  d < num_dets;  d += repeat )
        {
            //x = (base_line_byte + d/2) & 0xFF;
            x = (view_num + s%16 + d/2) & 0xFF;

            val = ((x << 24) | (x << 16) | (x << 8)| (x));

            memcpy((uint32_t *)dest_buf_ptr, &val, sizeof(val));
            dest_buf_ptr += sizeof(val);
            #ifdef DEBUG
                printf("row=%d das channel=%d --> %08x \n", s, d, val);
                printf("-----------------------------------------------------\n");
            #endif
        }
    }

    return EXIT_SUCCESS;
}

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
                               uint32_t das_channels)
{
    uint32_t s = 0;
    int32_t d = 0;
    uint8_t repeat = 4;
    uint8_t row_counter = num_segs-1;
    uint64_t val = 0;
    uint64_t x = 0;

    #ifdef DEBUG
    printf("\n-----------------------------------------------------\n");
    printf("------------Data pattern for >1000 Views/Sec---------\n");
    printf("-----------------------------------------------------\n");
    #endif

    if(dest_buf_ptr == NULL)
    {
        printf("Invalid pointer passed \n");
        return EFAULT;
    }

   for ( s = 0; s < num_segs; s++ )
    {
        for ( d = das_channels - 4;  d >= 0;  d -= repeat )
        {
            x = ((view_num & 0x03) << 14 | (row_counter << 10) | (d & 0x3FF));
            x = ENDIAN_BE16(x);
	    	val = ((x << 48) | (x << 32) | (x << 16) | (x));

            memcpy((uint64_t *)dest_buf_ptr, &val, sizeof(val));
            dest_buf_ptr += sizeof(val);

            #ifdef DEBUG
	    	    printf("row=%d das channel=%d --> %016lx \n", s, d, val);
            	printf("-----------------------------------------------------\n");
            #endif
        }

        //Residue calc
        if (d < 0)
        {
            d = 0;
            x = ((view_num & 0x03) << 14 | (row_counter << 10) | (d & 0x3FF));
            val = ((x << 48) | (x << 32) | (x << 16) | (x));
            memcpy((uint64_t *)dest_buf_ptr, &val, sizeof(val));
            dest_buf_ptr += sizeof(val);
        }

		if (row_counter)
		{
			row_counter --;
		}
		else
		{
			row_counter = num_segs-1;
		}
    }

    return EXIT_SUCCESS;
}

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
                               uint32_t das_channels)
{
    uint32_t s = 0;
    int32_t d = 0;
    uint8_t repeat = 2;
    uint8_t row_counter = num_segs-1;
    uint32_t val = 0;
    uint32_t x = 0;

    #ifdef DEBUG
    printf("\n-----------------------------------------------------\n");
    printf("------------Data pattern for >1000 Views/Sec---------\n");
    printf("-----------------------------------------------------\n");
    #endif

    if(dest_buf_ptr == NULL)
    {
        printf("Invalid pointer passed \n");
        return EFAULT;
    }

   for ( s = 0; s < num_segs; s++ )
    {
        for ( d = das_channels - 2;  d >= 0;  d -= repeat )
        {
            x = ((view_num & 0x03) << 14 | (row_counter << 10) | (d & 0x3FF));
            x = ENDIAN_BE16(x);
            val = ((x << 16) | (x));

            memcpy((uint32_t *)dest_buf_ptr, &val, sizeof(val));
            dest_buf_ptr += sizeof(val);

            #ifdef DEBUG
                printf("row=%d das channel=%d --> %08lx \n", s, d, val);
                printf("-----------------------------------------------------\n");
            #endif
        }

        //Residue calc
        if (d < 0)
        {
            d = 0;
            x = ((view_num & 0x03) << 14 | (row_counter << 10) | (d & 0x3FF));
            val = ((x << 16) | (x));
            memcpy((uint32_t *)dest_buf_ptr, &val, sizeof(val));
            dest_buf_ptr += sizeof(val);
        }

        if (row_counter)
        {
            row_counter --;
        }
        else
        {
            row_counter = num_segs-1;
        }
    }

    return EXIT_SUCCESS;
}

/***************************************************************************
 * name: pad_view
 * parameter:
 *      buff_addr:   pointer to buffer to place generated data
 *      num_segs:       no:of rows; usually 4 or 16
 *      das_channels:   no:of das channels
 * function: pad val 0x55aa at the end of each row in the generated data
 * **************************************************************************/
int32_t pad_view(uint16_t *buff_addr, uint32_t num_segs, uint32_t das_channels)
{
    uint32_t repeat = 0;
    uint32_t num_pattern_dets_per_seg = 0;
    uint32_t total_num_chan_per_seg = 0;
    uint32_t num_chan_to_pad = 0;
    uint16_t *data_ptr = NULL;
    uint16_t seg_num = 0;
    const uint16_t pad_value = 0x55aa;
    uint32_t i = 0;

    if(buff_addr == NULL)
    {
        printf("Invalid pointer passed \n");
        return EFAULT;
    }

    switch (num_segs)
    {
        case 4:
        {
            repeat=2;
            break;
        }

        case 16:
        {
            repeat=4;
            break;
        }

        default:
        {
            printf("ERROR: Unexpected Number of segments: NumSegments=%d\n", num_segs);
            return EXIT_FAILURE;
            break;
        }
    }

    total_num_chan_per_seg = ((das_channels + 3) / 4) * 4;
    num_pattern_dets_per_seg = ((das_channels + repeat - 1) / repeat) * repeat;
    num_chan_to_pad = total_num_chan_per_seg - num_pattern_dets_per_seg;

    #if DEBUG
    printf("total_num_chan_per_seg = %d \n", total_num_chan_per_seg);
    printf("num_pattern_dets_per_seg = %d \n", num_pattern_dets_per_seg);
    printf("num_chan_to_pad = %d \n", num_chan_to_pad);
    #endif

    if (num_chan_to_pad > 0)
    {
        for (seg_num = 0; seg_num < num_segs; seg_num++)
        {
            data_ptr = &buff_addr[seg_num * total_num_chan_per_seg + num_pattern_dets_per_seg];

            for (i = 0; i < num_chan_to_pad; i++)
            {
                *data_ptr = ENDIAN_BE16(pad_value);
                data_ptr++;
            }
        }
    }

    return EXIT_SUCCESS;
}

/***************************************************************************
 * name: create_golden_data_file
 * parameter:
 *      file_name:  file name of the golden data
 *      desc_str:   Header description string
 *      buf_ptr:    Generated data buffer pointer
 *      buf_size:   Size of the generated data
 * function: Create golden data file using formatted output for single view
 * **************************************************************************/
int32_t create_golden_data_file(char *file_name, char *desc_str, uint8_t *buf_ptr, uint64_t buf_size)
{
    uint16_t data = 0;
    uint64_t i = 0;
    char str[512] = {0};
    char data_str[128] = {0};
    FILE *fp = NULL;

    if((file_name == NULL) || (desc_str == NULL) || (buf_ptr == NULL))
    {
        return EXIT_FAILURE;
    }

    sprintf(str,"%s\n", U_LINE);

    sprintf(data_str, "%s\n", HEADER);
    strcat(str, data_str);
    memset(data_str, 0, sizeof(data_str));

    sprintf(data_str, "%s\n", desc_str);
    strcat(str, data_str);
    memset(data_str, 0, sizeof(data_str));

    sprintf(data_str, "%s\n", U_LINE);
    strcat(str, data_str);
    memset(data_str, 0, sizeof(data_str));

    sprintf(data_str," %08lu    ", i);
    strcat(str, data_str);
    memset(data_str, 0, sizeof(data_str));

    fp = fopen(file_name, "w");

    if(fp)
    {
        for(i = 0; i < buf_size; ( i = i + sizeof(uint16_t) ))
        {
            if(((i % 16) == 0x0) && (i > 0))
            {
                fprintf(fp,"%s \n", str);
                memset(str, 0, sizeof(str));
                sprintf(str," %08lu    ", i);
            }

            data = *((uint16_t *)(buf_ptr+i));
            //For making data representation matching to the binary view
            data = ENDIAN_LE16(data);
            sprintf(data_str, "%04x  ", data);
            strcat(str,data_str);
            memset(data_str, 0, sizeof(data_str));
        }

        //Check last str printed or not
        if(i > 0)
        {
            fprintf(fp, "%s \n", str);
        }

        fclose(fp);
    }
    else
    {
        printf("%s:%d: Could not able to open file %s\n",
                __FILE__, __LINE__, file_name);
    }

    return EXIT_SUCCESS;
}

/***************************************************************************
 * name: create_golden_data_binfile
 * parameter:
 *      file_name:  file name of the golden data binary file
 *      buf_ptr:    Generated data buffer pointer
 *      buf_size:   Size of the generated data
 * function: Create golden data file for single view
 * **************************************************************************/
int32_t create_golden_data_binfile(char *file_name, uint8_t *buf_ptr, uint64_t buf_size)
{
    int32_t fd = 0;
    uint64_t len = 0;
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    if((file_name == NULL) || (buf_ptr == NULL))
    {
        return EXIT_FAILURE;
    }

    fd = open(file_name, O_RDWR | O_CREAT, mode);

    if(fd)
    {
        len = write(fd, buf_ptr,buf_size);
        if(len != buf_size)
        {
            printf("%s:%d: write len not match with buf size.\n",
                __FILE__, __LINE__);
        }
        close(fd);
    }
    else
    {
        printf("%s:%d: Could not able to open file %s\n",
                __FILE__, __LINE__, file_name);
    }

    return EXIT_SUCCESS;
}

/***************************************************************************
 * name: parse_config_param
 * parameter:
 *      config_str:     config string 
 *      das_config:     das configuration parameters for the test   
 * function: This function will parse the configuration string read from the 
 *          gcif.cfg and saved in the das_config structure. 
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
        printf("Invalid parameter passed\n");
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
 *      das_config:     das configuration parameters for the test   
 * function: This function will read the configuration parameters 
 *           from the gcif.cfg file
 * **************************************************************************/
int32_t get_gcif_config(das_param_t *das_config)
{
    char str[80] = {'\0'};
    int32_t ret = EXIT_SUCCESS;

    if(das_config == NULL)
    {
        printf("Invalid parameter passed\n");
        return EFAULT;
    }

    FILE *fp = fopen("gcif.cfg", "r");
    if(fp == NULL)
    {
        printf("Unable to open gcif.cfg! \n");
        return EXIT_FAILURE;
    }

    while(fgets(str, sizeof(str), fp) != NULL)
    {
        ret = parse_config_param(str, das_config);
        if(ret != EXIT_SUCCESS)
        {
            printf("Config file parse error\n");
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
 *      das_config:     das configuration parameters for the test   
 * function: This function will display the configuration parameters read
 *           from the gcif.cfg file
 * **************************************************************************/
void show_gcif_config(das_param_t *das_config)
{
    if(das_config != NULL)
    {
        printf("rows_per_view = %d\n", das_config->rows_per_view);
        printf("das_channels = %d\n", das_config->das_channels);
        printf("views_per_tx = %d\n", das_config->views_per_tx);
        printf("flags = %d\n", das_config->flags);
        printf("views_per_scan = %d\n", das_config->views_per_scan);
        printf("views_per_sec = %d\n", das_config->views_per_sec);
        printf("view_starting_row = %d\n", das_config->view_starting_row);
    }
}

int main()
{

    uint8_t *data_ptr = NULL;
    uint64_t data_size = 0;
    uint32_t view_num = 0;
    uint32_t num_dets = 0;
    char file_name[64] = {0};
    char desc_str[128] = {0};
    das_param_t das_config_param;
    int32_t ret = EXIT_SUCCESS;

    printf(" Golden data generator application for a specified view number\n");
    int32_t status = mkdir("./golden_data", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if(status == 0)
    {
        printf("Creating log folder \n");
    }
    else
    {
        printf("log folder already available in the system \n");
    }

    memset(&das_config_param, 0, sizeof(das_param_t));
    ret = get_gcif_config(&das_config_param);
    if(ret != EXIT_SUCCESS)
    {
        printf("gcif.cfg file not present or corrupted. Test aborted!! \n");
        exit(0);
    }

    printf("Please enter the view number:");
    if(scanf("%d", &view_num))
    {
        //
    }

    //As per the dsign doc = 788
    num_dets =  (((das_config_param.das_channels + 3) / 4) * 4);

	data_size = (das_config_param.rows_per_view * num_dets * 2);

    printf("num_row = %d view_num=%d data_rate = %d data_size =%lu\n",
           das_config_param.rows_per_view, view_num, das_config_param.views_per_sec, data_size );

	//allocated buffer for das scan data
	data_ptr = calloc(data_size, sizeof(uint8_t));
	if (data_ptr == NULL)
	{
		printf("%s:%d:Cannot allocate %lu bytes of memory!! breaking \n",
				__FILE__, __LINE__, data_size);

		return EXIT_FAILURE;
	}

	if (das_config_param.views_per_sec > 1000)
	{
        if (das_config_param.rows_per_view == 16)
        {
		    high_data_rate_fillbuf16(data_ptr, view_num, das_config_param.rows_per_view,
                               das_config_param.das_channels);
        }
        else if (das_config_param.rows_per_view == 4)
        {
            high_data_rate_fillbuf4(data_ptr, view_num, das_config_param.rows_per_view,
                               das_config_param.das_channels);
        }

	}
	else
	{
        if (das_config_param.rows_per_view == 16)
        {
		    low_data_rate_fillbuf16(data_ptr, view_num, das_config_param.rows_per_view,
                                  das_config_param.das_channels);
        }
        else if(das_config_param.rows_per_view == 4)
        {
            low_data_rate_fillbuf4(data_ptr, view_num, das_config_param.rows_per_view,
                                  das_config_param.das_channels);
        }
	}
    //for padding with 0xaa55
    pad_view((uint16_t *)data_ptr, das_config_param.rows_per_view, das_config_param.das_channels);

    //gd_'view_num'_'num_row'_'data_rate'
    sprintf(file_name,"./golden_data/gd_v%d_r%d_%d", view_num, das_config_param.rows_per_view, das_config_param.views_per_sec );

    sprintf(desc_str,HEADER_1,view_num, das_config_param.rows_per_view, das_config_param.views_per_sec );

    create_golden_data_file(file_name, desc_str, data_ptr, data_size);

    sprintf(file_name,"./golden_data/gd_v%d_r%d_%d.bin", view_num, das_config_param.rows_per_view, das_config_param.views_per_sec );
    create_golden_data_binfile(file_name, data_ptr, data_size);

    if(data_ptr)
    {
        free(data_ptr);
    }

	return EXIT_SUCCESS;
}
