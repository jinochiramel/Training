#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "data_rate_pat_gen.h"
#include "logger.h"

//#define DEBUG // Comment if not required

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
    LOG_PRINT("-----------------------------------------------------");
    LOG_PRINT("------------Data pattern for <1000 Views/Sec---------");
    LOG_PRINT("-----------------------------------------------------");
    #endif

    if(dest_buf_ptr == NULL)
    {
        LOG_ERROR("Invalid pointer passed");
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
	        LOG_PRINT("row=%d das channel=%d --> %016lx ", s, d, val);
            LOG_PRINT("-----------------------------------------------------");
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
    LOG_PRINT("-----------------------------------------------------");
    LOG_PRINT("------------Data pattern for >1000 Views/Sec---------");
    LOG_PRINT("-----------------------------------------------------");
    #endif

    if(dest_buf_ptr == NULL)
    {
        LOG_ERROR("Invalid pointer passed");
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
	    	LOG_PRINT("row=%d das channel=%d --> %016lx ", s, d, val);
            LOG_PRINT("-----------------------------------------------------");
            #endif
        }

        //Residue calc
        if (d < 0)
        {
            d = 0;
            x = ((view_num & 0x03) << 14 | (row_counter << 10) | (d & 0x3FF));
            x = ENDIAN_BE16(x);
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
            x = ENDIAN_BE16(x);
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
        LOG_ERROR("Invalid pointer passed ");
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
            LOG_ERROR(" Unexpected Number of segments: NumSegments=%d", num_segs);
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
 * name: create_simulation_data_buf
 * parameter:
 *      rows_per_view: no:of rows in a view; 16 or 4
 *      das_channels:  no:of das channels
 * function: Create simulated data and store in memory
 * **************************************************************************/
sim_data_info_t *create_simulation_data_buf(uint8_t rows_per_view,
                                        uint16_t das_channels)
{
    sim_data_info_t *p_sim_data_info = NULL;
    uint32_t num_dets =  (((das_channels + 3) / 4) * 4);

    p_sim_data_info = malloc(sizeof(sim_data_info_t));
    if(p_sim_data_info == NULL)
    {
        LOG_ERROR("Could not allocate memory for sim_data_info_t ");
        return NULL;
    }

    p_sim_data_info->sim_data_size = (rows_per_view * num_dets * 2);

    // (* sizeof(p_sim_data_info->sim_data_buf)) -> is used for avoiding free error.
    // Now 8 times of actuall buf size is allocated
    p_sim_data_info->sim_data_buf = malloc(p_sim_data_info->sim_data_size * sizeof(p_sim_data_info->sim_data_buf));

    if(p_sim_data_info->sim_data_buf == NULL)
    {
        LOG_ERROR("Cannot allocate %lu bytes of memory!! breaking",
                  p_sim_data_info->sim_data_size);

        free(p_sim_data_info);
        return NULL;
    }

    return p_sim_data_info;
}

/***************************************************************************
 * name: destroy_simulation_data
 * parameter:
 *      p_sim_data: Pointer needs to be free
 * function: Deleting the simulation data pointer
 * **************************************************************************/
void destroy_simulation_data(sim_data_info_t *p_sim_data)
{
    if(p_sim_data)
    {
        if(p_sim_data->sim_data_buf)
        {
            free(p_sim_data->sim_data_buf);
            p_sim_data->sim_data_buf = NULL;
        }

        free(p_sim_data);
        p_sim_data = NULL;
    }
}

/***************************************************************************
 * name: show_generated_data
 * parameter:
 *      buf_ptr: buffer pointer
 *      buf_size: size of the buffer
 * function: Display the generated data on the console screen.
 * **************************************************************************/
void show_generated_data(uint8_t *buf_ptr, uint64_t buf_size)
{
    uint64_t data = 0;
    char str[512] = {0};
    char data_str[64] = {0};
    uint64_t i = 0;

    LOG_PRINT("\n==============Generated data=================");
    for(i = 0; i < buf_size; ( i = i + sizeof(uint64_t) ))
    {
        if(((i % 32) == 0x0) && (i > 0))
        {
            LOG_PRINT("%s", str);
            memset(str, 0, sizeof(str));
        }
        data = *((uint64_t *)(buf_ptr+i));
        //For making data representation matching to the binary view
        data = ENDIAN_LE16(data);
        sprintf(data_str, "%016lx  ", data);
        strcat(str,data_str);
        memset(data_str, 0, sizeof(data_str));
    }
    //Check last str printed or not
    if(i > 0)
    {
        LOG_PRINT("%s", str);
    }

    LOG_PRINT("==============Generated data end==============\n");
}

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
                        das_param_t *das_config_param)
{
    int32_t status = EXIT_SUCCESS;

    if((golden_data_ptr == NULL) || (das_config_param == NULL))
    {
        LOG_ERROR("Invalid parameter passed");
        return EXIT_FAILURE;
    }


    //Initialize the buffer
    memset(golden_data_ptr, 0, data_size);

    if(das_config_param->views_per_sec > 1000)
    {
        if (das_config_param->rows_per_view == 16)
        {
            high_data_rate_fillbuf16(golden_data_ptr, view_num,
                               das_config_param->rows_per_view,
                               das_config_param->das_channels);
        }
        else if (das_config_param->rows_per_view == 4)
        {
            high_data_rate_fillbuf4(golden_data_ptr, view_num,
                               das_config_param->rows_per_view,
                               das_config_param->das_channels);
        }
    }
    else
    {
        if (das_config_param->rows_per_view == 16)
        {
            low_data_rate_fillbuf16(golden_data_ptr, view_num,
                              das_config_param->rows_per_view,
                              das_config_param->das_channels);
        }
        else if (das_config_param->rows_per_view == 4)
        {
            low_data_rate_fillbuf4(golden_data_ptr, view_num,
                              das_config_param->rows_per_view,
                              das_config_param->das_channels);
        }
    }

    //for padding with 0xaa55
    pad_view((uint16_t *)golden_data_ptr,
             das_config_param->rows_per_view,
             das_config_param->das_channels);

    return status;
}

/***************************************************************************
 * name: verify_gcif_data
 * parameter:
 *      das_data_ptr: das data buffer pointer
 *      das_data_size: size of the das data
 *      das_config_param: DAS configuration parameters
 * function: Verify the scan data came from the DAS with golden data
 * **************************************************************************/
int32_t verify_gcif_data(uint8_t *das_data_ptr, uint64_t das_data_size,
                         das_param_t *das_config_param)
{
    int32_t status = EXIT_SUCCESS;
    int64_t data_index = 0;

    if((das_data_ptr == NULL) || (das_config_param == NULL))
    {
        LOG_ERROR("Invalid parameter passed");
        return EXIT_FAILURE;
    }

    sim_data_info_t *sim_data_info = NULL;
    //Create buf for one view
    sim_data_info = create_simulation_data_buf(das_config_param->rows_per_view,
                                               das_config_param->das_channels);
    if(sim_data_info == NULL)
    {
        LOG_ERROR("Could not created simulation data buffer!!");
        return EXIT_FAILURE;
    }

    for(uint32_t i = 0; i < das_config_param->views_per_scan; i++)
    {
        //get golden data for a view
        status = get_golden_data(sim_data_info->sim_data_buf, sim_data_info->sim_data_size,
                                 i, das_config_param);
        if(status != EXIT_SUCCESS)
        {
            LOG_ERROR("Failed to fetch golden data for the view number = %d", i);
            goto exit;
        }

        #ifdef DEBUG
        LOG_PRINT("===========%dth view===============", i);
        show_generated_data(sim_data_info->sim_data_buf, sim_data_info->sim_data_size);
        #endif

        data_index = (i * sim_data_info->sim_data_size);

        if((data_index+sim_data_info->sim_data_size) > das_data_size)
        {
            LOG_WARN("!! possible buffer over flow detected");
            //return EXIT_FAILURE;
        }

        // Compare generated ith view with das ith view here
        for(uint32_t j = 0; j < sim_data_info->sim_data_size; j++)
        {
            if(sim_data_info->sim_data_buf[j] != das_data_ptr[data_index + j])
            {
                printf("%s:%s:%d: !!Miss match found at: view number=%d view index=%d "
                       "scan data buffer loc = %lu \n",
                        __FILE__, __func__, __LINE__, i, j, (data_index + j));

                LOG_ERROR("!!Miss match found at: view number=%d view index=%d"
                          " scan data buffer loc=%lu", i, j, (data_index + j));

                status = EXIT_FAILURE;
                goto exit;
            }
        }
    }

    exit:
    destroy_simulation_data(sim_data_info);

    return status;
}

//for test stub
void dummy_das_scan_data(uint8_t *das_data_ptr,
                        uint32_t views_per_scan, uint8_t rows_per_view,
                        uint32_t das_channels, uint32_t views_per_sec)
{
    int64_t data_index = 0;
    sim_data_info_t *p_sim_data_info = NULL;
    //Create buf for one view
    p_sim_data_info = create_simulation_data_buf(rows_per_view, das_channels);
    if(p_sim_data_info == NULL)
    {
        LOG_ERROR("Could not created simulation data!! breaking");
        //return EXIT_FAILURE;
    }

    //data genetared only for one view.
    for(uint32_t i = 0; i < views_per_scan; i++)
    {
        //Initialize the buffer
        memset(p_sim_data_info->sim_data_buf, 0, p_sim_data_info->sim_data_size);

        if(views_per_sec > 1000)
        {
            if(rows_per_view == 16)
            {
                high_data_rate_fillbuf16(p_sim_data_info->sim_data_buf, i, rows_per_view, das_channels);
            }
            else if(rows_per_view == 4)
            {
                high_data_rate_fillbuf4(p_sim_data_info->sim_data_buf, i, rows_per_view, das_channels);
            }
        }
        else
        {
            if(rows_per_view == 16)
            {
                low_data_rate_fillbuf16(p_sim_data_info->sim_data_buf, i, rows_per_view, das_channels);
            }
            else if(rows_per_view == 4)
            {
                low_data_rate_fillbuf4(p_sim_data_info->sim_data_buf, i, rows_per_view, das_channels);
            }
        }
        //for padding with 0xaa55
        pad_view((uint16_t *)p_sim_data_info->sim_data_buf, rows_per_view, das_channels);
        data_index = (i * p_sim_data_info->sim_data_size);
        memcpy((das_data_ptr+data_index), p_sim_data_info->sim_data_buf, p_sim_data_info->sim_data_size);
    }

    destroy_simulation_data(p_sim_data_info);
}
