#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <pthread.h>

#include "hba_info_read.h"
#include "scsi_test.h"
#include "logger.h"

#define GCIF_TESTAPP_VER "1.0.0"

#define HEADER_STR          "+=================================================+"
#define BLANK_SPACE_STR     "|                                                 |"
#define TEST_APP_NAME_STR   "|               GCIFA-D4 Test Tool                |"
#define UNDER_LINE_STR      "|               ------------------                |"
//                          "+=================================================+"
//                          "|                                                 |"
#define MENU_0_STR          "| 0: Exit                                         |"
#define MENU_1_STR          "| 1: Display FC HBA Information                   |"
#define MENU_2_STR          "| 2: GCIFA-D4 Scan and Verify                     |"
//                          "|                                                 |"
//                          "+=================================================+"
#define MENU_CHOICE_STR     "  Enter your choice:"

extern uint8_t g_num_host_detected;
char g_scsi_dev_path[128] = {'\0'};
static uint8_t g_test_completed = 0;
uint8_t g_esc_pressed = 0;
pthread_mutex_t g_esc_key_mutex = PTHREAD_MUTEX_INITIALIZER;

//Proto type
static void display_test_menu();
static void set_mode(int want_key);
static int get_key();
static void *key_press_monitor();
//End

/***************************************************************************
 * name: display_test_menu
 * parameter: Nil
 *      function: Display gcif test application menu on the screen
 * **************************************************************************/
static void display_test_menu()
{
    printf("%s\n", HEADER_STR);
    printf("%s\n", BLANK_SPACE_STR);
    printf("%s\n", TEST_APP_NAME_STR);
    printf("%s\n", UNDER_LINE_STR);
    printf("%s\n", HEADER_STR);
    printf("%s\n", BLANK_SPACE_STR);
    printf("%s\n", MENU_0_STR);
    printf("%s\n", MENU_1_STR);
    printf("%s\n", MENU_2_STR);
    printf("%s\n", BLANK_SPACE_STR);
    printf("%s\n", HEADER_STR);
    printf("\n");
    printf("%s", MENU_CHOICE_STR);
}

/***************************************************************************
 * name: set_mode
 * parameter:
 *      want_key: Carriage return required or not.
 *                posssible values either 1 or 0
 * function: Set the console mode based on the input want_key
 * **************************************************************************/
static void set_mode(int want_key)
{
    static struct termios old, new;

    // Make sure stdin is a terminal.
    if (isatty (STDIN_FILENO))
    {
        // It is a terminal
        if (!want_key)
        {
            tcsetattr(STDIN_FILENO, TCSANOW, &old);
        }
        else
        {
            tcgetattr(STDIN_FILENO, &old);
            new = old;
            new.c_lflag &= ~(ICANON | ECHO); // Clear ICANON and ECHO.
            tcsetattr(STDIN_FILENO, TCSANOW, &new);
            //tcsetattr(STDIN_FILENO, TCSAFLUSH, &new);
        }
    }
}

/***************************************************************************
 * name: get_key
 * parameter: Nil
 * function: This function is used to get key value pressed
 * **************************************************************************/
static int get_key()
{
    int key_val = 0;
    struct timeval tv;
    fd_set fs;
    tv.tv_usec = 10000;
    tv.tv_sec = 0;

    FD_ZERO(&fs);
    FD_SET(STDIN_FILENO, &fs);
    select(STDIN_FILENO + 1, &fs, 0, 0, &tv);

    if (FD_ISSET(STDIN_FILENO, &fs))
    {
        key_val = getchar();
        //key_val = fgetc(stdin);
        set_mode(0);
    }
    return key_val;
}

/***************************************************************************
 * name: key_press_monitor
 * parameter: Nil
 * function: This is the thread function for getting the key value
 * **************************************************************************/
static void *key_press_monitor()
{
    int c;

    // Need to exit from this loop either esc key pressed
    // or test completed
    while(( !g_esc_pressed) || (!g_test_completed))
    {
        set_mode(1);
        while (!(c = get_key()))
        {
            if(g_test_completed)
            {
                set_mode(0);
                goto exit;
            }
            usleep(10000);
        }
        //printf("key %d\n", c);
        if (c == 27)
        {
            LOG_WARN("ESC key pressed");
            pthread_mutex_lock(&g_esc_key_mutex);
            g_esc_pressed = 1;
            pthread_mutex_unlock(&g_esc_key_mutex);
        }
    }

    exit:

    pthread_exit(NULL);
}

/***************************************************************************
 * name: main
 * parameter: Nil
 * function: This is the main function of the gcif test application.
 *           All the functions will be called from the main function.
 * **************************************************************************/
int32_t main()
{
    char choice = 0;
    pthread_t key_monitor_thread;
    das_param_t das_config_param;
    int32_t ret = EXIT_SUCCESS;

    int32_t status = mkdir("./log", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    if(status == 0)
    {
        LOG_PRINT("Creating log folder");
    }
    else
    {
        LOG_WARN("log folder already available in the system");
    }

    LOG_PRINT("GCIF Test Application Version: %s ", GCIF_TESTAPP_VER);
    LOG_PRINT("GCIF Test Application Build time stamp: %s", __TIMESTAMP__); 
    LOG_PRINT("GCIF test application starting...");

    memset(&das_config_param, 0, sizeof(das_param_t));
    ret = get_gcif_config(&das_config_param);
    if(ret != EXIT_SUCCESS)
    {
        LOG_ERROR("gcif.cfg file not present or corrupted. Test aborted!!");
        printf("gcif.cfg file not present or corrupted. Test aborted!! \n");
        exit(0);
    }
    //Get host and remote port details
    ret = get_fc_host_list();
    if(ret != EXIT_SUCCESS)
    {
        LOG_ERROR("Failed to get FC host lists.");
    }

    do
    {
        display_test_menu();

        fflush(stdin);
        choice = getchar();

        //flush the \n from choice
        if(choice != '\n')
        {
            getchar();
        }
        else
        {
            printf("\n");
            continue;
        }

        switch(choice)
        {
            case '0':
            {
                //nothing Quit App
                LOG_PRINT("Exiting from the GCIF test application");
            }
            break;

            case '1':
            {
                LOG_PRINT("Display FC HBA Information - CLI option selected");

                if(g_num_host_detected)
                {
                    for(int8_t i = 0; i < g_num_host_detected; i++)
                    {
                        display_hba_info(i);
                    }
                }
                else
                {
                    LOG_ERROR("No FC host detected in the system!!");
                    printf("No FC host detected in the system!!\n");
                }
            }
            break;

            case '2':
            {
                int32_t ret = EXIT_SUCCESS;
                uint32_t test_repeat_cnt = 0;
                uint32_t test_pass_cnt = 0;
                uint32_t test_cnt=0;
                int32_t thread_ret = EXIT_SUCCESS;

                LOG_PRINT("GCIFA-D4 Scan and Verify - CLI option selected");

                show_gcif_config(&das_config_param);

                pthread_mutex_lock(&g_esc_key_mutex);
                g_esc_pressed = 0;
                pthread_mutex_unlock(&g_esc_key_mutex);
                g_test_completed = 0;

                if(g_num_host_detected == 0)
                {
                    LOG_ERROR("No FC host detected in the system!!");
                    printf("No FC host detected in the system!!\n");
                    break;
                }

                printf("Test repeat count:");
                if(scanf("%d", &test_repeat_cnt))
                {
                    LOG_PRINT("Test repeat count = %d", test_repeat_cnt);
                }

                ret = get_scsi_devpath(g_scsi_dev_path);
                if(ret != EXIT_SUCCESS)
                {
                    LOG_ERROR("Could not get vendor specific scsi device path!!");
                    printf("Could not get vendor specific scsi device path!! \n");
                    break;
                }
                else
                {
                    LOG_PRINT("generic scsi device path = %s", g_scsi_dev_path);
                }

                //add logic for inifinity if test_repeat_cnt = 0
                thread_ret = pthread_create(&key_monitor_thread, NULL, key_press_monitor, NULL);
                if(thread_ret)
                {
                    LOG_ERROR("Could not create key press monitor thread");
                    break;
                }

                for(test_cnt = 0;
                    test_cnt < (!test_repeat_cnt ? (test_cnt+1): test_repeat_cnt);
                    ++test_cnt)
                {
                    if(g_esc_pressed)
                    {
                        //terminate the test execution
                        printf("\nUser terminated the test execution by pressing ESC key!!\n");
                        goto exit;
                    }

                    ret = gcif_scan_and_verify(g_scsi_dev_path, &das_config_param);
                    if(ret != EXIT_SUCCESS)
                    {
                        printf("\n Error!! gcif_scan_and_verify failed!! \n");
                        LOG_ERROR("gcif_scan_and_verify failed!! --->Test iteration %d: Failed", test_cnt+1);
                        goto exit;
                    }
                    else
                    {
                        test_pass_cnt++;
                        //printf("\n--->Test %d: Pass\n\n", test_cnt+1);
                        set_mode(0);
                        usleep(100000);
                        printf(".");
                        set_mode(1);
                    }
                }

                exit:

                if(g_esc_pressed)
                {
                    LOG_WARN("User terminated the test execution by pressing ESC key!!");
                    printf("\nUser terminated the test execution by pressing ESC key!!\n");
                }

                // manage thread
                g_test_completed = 1;

                printf("\n\nSummary of Test Result \n");
                printf("-----------------------\n");
                printf("Total test executed: %d\n", test_cnt);
                printf("Pass count:%d/%d\n\n", test_pass_cnt, test_cnt);

                pthread_join(key_monitor_thread, NULL);
            }
            break;

            default:
                printf("Error: Invalid choice entered!!\n");
            break;
        }
    }
    while(choice != '0');

    return EXIT_SUCCESS;
}
