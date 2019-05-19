/******************************************************************
File Name: LIN_driver.h
Description: LIN driver header
AUTHORS:
DATE:
******************************************************************/

/* ============================================================ =========== **/
/* Function setting related definition */
/* ============================================================ =========== **/
#define	LIN_USE_MCU1	(0)
#define	LIN_USE_MCU3	(1)


/* ============================================================ =========== **/
/* Type declaration */
/* ============================================================ =========== **/

/* Definition for response information */
typedef union {
struct {
unsigned char id; /* ID */
unsigned char data_value; /* data length (DLC) */
unsigned char usage; /* usage: send / receive */
unsigned char csum_kind; /* Checksum: standard / extended */
} info_flag;
unsigned char byte [4];
} LIN_RESPONSE_INFO;

/* Definition for APP-LIN interface */
typedef union {
unsigned char byte [11];
struct {
unsigned char data [8]; /* data */
unsigned char tx_status; /* Transmission status */
unsigned char update; /* Notification of received data update */
unsigned char tx_hdr_req; /* Request to transmit Header */
} data;
} LIN_COM_DATA;

/* Definition for APP-LIN I / F reception buffer */
typedef union {
unsigned char byte [12];
struct {
unsigned char data [8]; /* data */
unsigned char tx_status; /* Transmission status */
unsigned char update; /* Notification of received data update */
unsigned char rx_up_request; /* Receive data update request */
unsigned char tx_up_request; /* Transmission status update request */
} data;
} LIN_COM_DATA_BUF;

/* Definition for buffer for I / F transmission between APP and LIN */
typedef struct {
unsigned char tx_data [8]; /* Transmission data */
} LIN_COM_DATA_TX;


/* ============================================================ =========== **/
/* Value definition */
/* ============================================================ =========== **/

/* Definition for frame mode */
#define LIN_FMD_IDLE (0x00) /* IDLE */
#define LIN_FMD_FRAME_ERR (0x01) /* Waiting for framing error occurrence */
#define LIN_FMD_BREAK_END (0x02) /* Wait for SyncBreak end */
#define LIN_FMD_SYNCH (0x03) /* Wait for sync reception */
#define LIN_FMD_PID (0x04) /* Waiting for ID reception */
#define LIN_FMD_DATA (0x05) /* Data field processing */
#define LIN_FMD_CHECKSUM (0x06) /* Wait for Checksum reception */

/* Response definition: Usage */
#define LIN_RESINFO_USAGE_TX (0x00) /* Send */
#define LIN_RESINFO_USAGE_RX (0x01) /* Receive */
/* Response definition: Checksum type */
#define LIN_RESINFO_CSUM_ENHANCED (0x00) /* Extended checksum */
#define LIN_RESINFO_CSUM_CLASSIC (0x01) /* standard checksum */

/* Transmission status definition */
#define LIN_TXSTATUS_CLEAR (0x00) /* Status clear */
#define LIN_TXSTATUS_SUCCESS (0x01) /* Transmission completed successfully */
#define LIN_TXSTATUS_ERROR (0x02) /* Transmission abnormal end */

/* Definition for received data update notification */
#define LIN_UPDATE_CLEAR (0x00) /* clear notification */
#define LIN_UPDATE_SET (0x01) /* updated */

