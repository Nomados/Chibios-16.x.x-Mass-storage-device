/**
 * @file    usb_msd.h
 * @brief   USB mass storage driver and functions
 */

#ifndef _USB_MSD_H_
#define _USB_MSD_H_

#include "ch.h"
#include "hal.h"
#include "chdebug.h"

#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END




#define USBD USBD1
#define USB_BUS_ACTIVE palSetPad(GPIOF, GPIOF_PIN6)
#define USB_BUS_IDLE   palClearPad(GPIOF, GPIOF_PIN6)

#define SEM_TAKEN       palSetPad(GPIOF, GPIOF_PIN7)
#define SEM_RELEASED    palClearPad(GPIOF, GPIOF_PIN7)

#define         EVT_USB_RESET                           (1 << 0)
#define         EVT_BOT_RESET                           (1 << 1)
#define         EVT_SEM_TAKEN                           (1 << 2)
#define         EVT_SEM_RELEASED                        (1 << 3)
#define         EVT_USB_CONFIGURED                      (1 << 4)
#define         EVT_SCSI_REQ_TEST_UNIT_READY            (1 << 5)
#define         EVT_SCSI_REQ_READ_FMT_CAP               (1 << 6)
#define         EVT_SCSI_REQ_SENSE6                     (1 << 7)
#define         EVT_SCSI_REQ_SENSE10                    (1 << 8)
#define         EVT_WAIT_FOR_COMMAND_BLOCK              (1 << 9)
#define         EVT_SCSI_REQ_SEND_DIAGNOSTIC            (1 << 10)
#define         EVT_SCSI_REQ_READ_CAP10                 (1 << 11)
#define         EVT_SCSI_PROC_INQ                       (1 << 12)

/**
 * @brief Command Block Wrapper structure
 */
PACK_STRUCT_BEGIN typedef struct {
	uint32_t signature;
	uint32_t tag;
	uint32_t data_len;
	uint8_t flags;
	uint8_t lun;
	uint8_t scsi_cmd_len;
	uint8_t scsi_cmd_data[16];
} PACK_STRUCT_STRUCT msd_cbw_t PACK_STRUCT_END;

/**
 * @brief Command Status Wrapper structure
 */
PACK_STRUCT_BEGIN typedef struct {
	uint32_t signature;
	uint32_t tag;
	uint32_t data_residue;
	uint8_t status;
} PACK_STRUCT_STRUCT msd_csw_t PACK_STRUCT_END;

/**
 * @brief Structure holding sense data (status/error information)
 */
PACK_STRUCT_BEGIN typedef struct {
		uint8_t byte[18];
} PACK_STRUCT_STRUCT msd_scsi_sense_response_t PACK_STRUCT_END;

/**
 * @brief structure holding the data to reply to an INQUIRY SCSI command
 */
PACK_STRUCT_BEGIN typedef struct
{
    uint8_t peripheral;
    uint8_t removable;
    uint8_t version;
    uint8_t response_data_format;
    uint8_t additional_length;
    uint8_t sccstp;
    uint8_t bqueetc;
    uint8_t cmdque;
    uint8_t vendor_id[8];
    uint8_t product_id[16];
    uint8_t product_rev[4];
} PACK_STRUCT_STRUCT msd_scsi_inquiry_response_t PACK_STRUCT_END;

/**
 * @brief Possible states for the USB mass storage driver
 */
typedef enum {
    MSD_IDLE,
    MSD_READ_COMMAND_BLOCK,
    MSD_EJECTED,
    MSD_BOT_RESET
} msd_state_t;

/**
 * @brief Driver configuration structure
 */
typedef struct {
    /**
    * @brief USB driver to use for communication
    */
    USBDriver *usbp;

    /**
    * @brief Block device to use for storage
    */
    BaseBlockDevice *bbdp;

    /**
    * @brief Index of the USB endpoint to use for transfers
    */
    usbep_t bulk_ep;

    /**
    * @brief Optional callback that will be called whenever there is
    *        read/write activity
    * @note  The callback is called with argument true when activity starts,
    *        and false when activity stops.
    */
    void (*rw_activity_callback)(bool);

    /**
    * @brief Short vendor identification
    * @note  ASCII characters only, maximum 8 characters (pad with zeroes).
    */
    uint8_t short_vendor_id[8];

    /**
    * @brief Short product identification
    * @note  ASCII characters only, maximum 16 characters (pad with zeroes).
    */
    uint8_t short_product_id[16];

    /**
    * @brief Short product revision
    * @note  ASCII characters only, maximum 4 characters (pad with zeroes).
    */
    uint8_t short_product_version[4];

} USBMassStorageConfig;

/**
 * @brief   USB mass storage driver structure.
 * @details This structure holds all the states and members of a USB mass
 *          storage driver.
 */
typedef struct {
    const USBMassStorageConfig* config;
    binary_semaphore_t bsem;
    thread_t* thread;
    event_source_t evt_connected, evt_ejected;
	BlockDeviceInfo block_dev_info;
	msd_state_t state;
	msd_cbw_t cbw;
	msd_csw_t csw;
	msd_scsi_sense_response_t sense;
	msd_scsi_inquiry_response_t inquiry;
	bool reconfigured_or_reset_event;
	bool result;
	bool bot_reset;
} USBMassStorageDriver;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Initializes a USB mass storage driver.
 */
void msdInit(USBMassStorageDriver *msdp);

/**
 * @brief   Starts a USB mass storage driver.
 * @details This function is sufficient to have USB mass storage running, it internally
 *          runs a thread that handles USB requests and transfers.
 *          The block device must be connected but no file system must be mounted,
 *          everything is handled by the host system.
 */
void msdStart(USBMassStorageDriver *msdp, const USBMassStorageConfig *config);

/**
 * @brief   Stops a USB mass storage driver.
 * @details This function waits for current tasks to be finished, if any, and then
 *          stops the mass storage thread.
 */
void msdStop(USBMassStorageDriver *msdp);

/**
 * @brief   USB device configured handler.
 *
 * @param[in] msdp      pointer to the @p USBMassStorageDriver object
 *
 * @iclass
 */
void msdConfigureHookI(USBMassStorageDriver *msdp);

/**
 * @brief   Default requests hook.
 * @details Applications wanting to use the Mass Storage over USB driver can use
 *          this function as requests hook in the USB configuration.
 *          The following requests are emulated:
 *          - MSD_REQ_RESET.
 *          - MSD_GET_MAX_LUN.
 *          .
 *
 * @param[in] usbp      pointer to the @p USBDriver object
 * @return              The hook status.
 * @retval true         Message handled internally.
 * @retval false        Message not handled.
 */
bool msdRequestsHook(USBDriver *usbp);


void init_msd_driver(void * p);
void deinit_msd_driver(void);

extern void debug_print_str(char * string);

#ifdef __cplusplus
}
#endif



#endif /* _USB_MSD_H_ */
