/******************************************************************************/
/* File Name   : flash_priv.h                                                 */
/* Author      : Bootloader Team                                              */
/* Description : Private definitions for STM32F401 Flash driver               */
/*               Contains register structures and bit definitions             */
/******************************************************************************/

#ifndef FLASH_PRIV_H
#define FLASH_PRIV_H

#include "LIB/stdtypes.h"

/******************************************************************************/
/*                          FLASH BASE ADDRESS                                 */
/******************************************************************************/
#define FLASH_BASE_ADDR                 ((volatile FLASH_Register_t*)0x40023C00UL)

/******************************************************************************/
/*                          FLASH UNLOCK KEYS                                  */
/******************************************************************************/
/* Keys to unlock FLASH_CR register (allow programming/erasing) */
#define FLASH_KEY1                      (0x45670123UL)
#define FLASH_KEY2                      (0xCDEF89ABUL)

/* Keys to unlock FLASH_OPTCR register (allow option bytes modification) */
#define FLASH_OPTKEY1                   (0x08192A3BUL)
#define FLASH_OPTKEY2                   (0x4C5D6E7FUL)

/******************************************************************************/
/*                    FLASH MEMORY MAP (STM32F401xE - 512KB)                   */
/******************************************************************************/
#define FLASH_BASE                      (0x08000000UL)
#define FLASH_END                       (0x0807FFFFUL)
#define FLASH_TOTAL_SIZE                (512U * 1024U)  /* 512 KB */

/* Sector base addresses */
#define FLASH_SECTOR_0_BASE             (0x08000000UL)  /* 16 KB  */
#define FLASH_SECTOR_1_BASE             (0x08004000UL)  /* 16 KB  */
#define FLASH_SECTOR_2_BASE             (0x08008000UL)  /* 16 KB  */
#define FLASH_SECTOR_3_BASE             (0x0800C000UL)  /* 16 KB  */
#define FLASH_SECTOR_4_BASE             (0x08010000UL)  /* 64 KB  */
#define FLASH_SECTOR_5_BASE             (0x08020000UL)  /* 128 KB */
#define FLASH_SECTOR_6_BASE             (0x08040000UL)  /* 128 KB */
#define FLASH_SECTOR_7_BASE             (0x08060000UL)  /* 128 KB */

/* Sector sizes in bytes */
#define FLASH_SECTOR_0_SIZE             (16U  * 1024U)
#define FLASH_SECTOR_1_SIZE             (16U  * 1024U)
#define FLASH_SECTOR_2_SIZE             (16U  * 1024U)
#define FLASH_SECTOR_3_SIZE             (16U  * 1024U)
#define FLASH_SECTOR_4_SIZE             (64U  * 1024U)
#define FLASH_SECTOR_5_SIZE             (128U * 1024U)
#define FLASH_SECTOR_6_SIZE             (128U * 1024U)
#define FLASH_SECTOR_7_SIZE             (128U * 1024U)

/* Total number of sectors */
#define FLASH_TOTAL_SECTORS             (8U)

/******************************************************************************/
/*                    FLASH_ACR (Access Control Register)                      */
/*                           Offset: 0x00                                      */
/******************************************************************************/
/* Bit positions */
#define FLASH_ACR_LATENCY_POS           (0U)
#define FLASH_ACR_PRFTEN_POS            (8U)
#define FLASH_ACR_ICEN_POS              (9U)
#define FLASH_ACR_DCEN_POS              (10U)
#define FLASH_ACR_ICRST_POS             (11U)
#define FLASH_ACR_DCRST_POS             (12U)

/* Bit masks (AND with these to clear bits) */
//                                       0b10987654321098765432109876543210
#define FLASH_ACR_LATENCY_CLR_MASK      (0b11111111111111111111111111110000UL)
#define FLASH_ACR_PRFTEN_CLR_MASK       (0b11111111111111111111111011111111UL)
#define FLASH_ACR_ICEN_CLR_MASK         (0b11111111111111111111110111111111UL)
#define FLASH_ACR_DCEN_CLR_MASK         (0b11111111111111111111101111111111UL)
#define FLASH_ACR_ICRST_CLR_MASK        (0b11111111111111111111011111111111UL)
#define FLASH_ACR_DCRST_CLR_MASK        (0b11111111111111111110111111111111UL)

/* Latency values (wait states) */
#define FLASH_ACR_LATENCY_0WS           (0x00000000UL)
#define FLASH_ACR_LATENCY_1WS           (0x00000001UL)
#define FLASH_ACR_LATENCY_2WS           (0x00000002UL)
#define FLASH_ACR_LATENCY_3WS           (0x00000003UL)
#define FLASH_ACR_LATENCY_4WS           (0x00000004UL)
#define FLASH_ACR_LATENCY_5WS           (0x00000005UL)
#define FLASH_ACR_LATENCY_6WS           (0x00000006UL)
#define FLASH_ACR_LATENCY_7WS           (0x00000007UL)
#define FLASH_ACR_LATENCY_8WS           (0x00000008UL)
#define FLASH_ACR_LATENCY_9WS           (0x00000009UL)
#define FLASH_ACR_LATENCY_10WS          (0x0000000AUL)
#define FLASH_ACR_LATENCY_11WS          (0x0000000BUL)
#define FLASH_ACR_LATENCY_12WS          (0x0000000CUL)
#define FLASH_ACR_LATENCY_13WS          (0x0000000DUL)
#define FLASH_ACR_LATENCY_14WS          (0x0000000EUL)
#define FLASH_ACR_LATENCY_15WS          (0x0000000FUL)

/* Enable/Disable values (OR with these to set bits) */
#define FLASH_ACR_PRFTEN                (0x00000100UL)
#define FLASH_ACR_ICEN                  (0x00000200UL)
#define FLASH_ACR_DCEN                  (0x00000400UL)
#define FLASH_ACR_ICRST                 (0x00000800UL)
#define FLASH_ACR_DCRST                 (0x00001000UL)

/******************************************************************************/
/*                    FLASH_SR (Status Register)                               */
/*                           Offset: 0x0C                                      */
/******************************************************************************/
/* Bit positions */
#define FLASH_SR_EOP_POS                (0U)
#define FLASH_SR_OPERR_POS              (1U)
#define FLASH_SR_WRPERR_POS             (4U)
#define FLASH_SR_PGAERR_POS             (5U)
#define FLASH_SR_PGPERR_POS             (6U)
#define FLASH_SR_PGSERR_POS             (7U)
#define FLASH_SR_RDERR_POS              (8U)
#define FLASH_SR_BSY_POS                (16U)

/* Bit values (for checking and clearing flags) */
//                                       0b10987654321098765432109876543210
#define FLASH_SR_EOP                    (0b00000000000000000000000000000001UL)
#define FLASH_SR_OPERR                  (0b00000000000000000000000000000010UL)
#define FLASH_SR_WRPERR                 (0b00000000000000000000000000010000UL)
#define FLASH_SR_PGAERR                 (0b00000000000000000000000000100000UL)
#define FLASH_SR_PGPERR                 (0b00000000000000000000000001000000UL)
#define FLASH_SR_PGSERR                 (0b00000000000000000000000010000000UL)
#define FLASH_SR_RDERR                  (0b00000000000000000000000100000000UL)
#define FLASH_SR_BSY                    (0b00000000000000010000000000000000UL)

/* Combined mask for all error flags */
#define FLASH_SR_ALL_ERRORS             (FLASH_SR_OPERR  | \
                                         FLASH_SR_WRPERR | \
                                         FLASH_SR_PGAERR | \
                                         FLASH_SR_PGPERR | \
                                         FLASH_SR_PGSERR | \
                                         FLASH_SR_RDERR)

/******************************************************************************/
/*                    FLASH_CR (Control Register)                              */
/*                           Offset: 0x10                                      */
/******************************************************************************/
/* Bit positions */
#define FLASH_CR_PG_POS                 (0U)
#define FLASH_CR_SER_POS                (1U)
#define FLASH_CR_MER_POS                (2U)
#define FLASH_CR_SNB_POS                (3U)
#define FLASH_CR_PSIZE_POS              (8U)
#define FLASH_CR_STRT_POS               (16U)
#define FLASH_CR_EOPIE_POS              (24U)
#define FLASH_CR_ERRIE_POS              (25U)
#define FLASH_CR_LOCK_POS               (31U)

/* Bit masks (AND with these to clear bits) */
//                                       0b10987654321098765432109876543210
#define FLASH_CR_PG_CLR_MASK            (0b11111111111111111111111111111110UL)
#define FLASH_CR_SER_CLR_MASK           (0b11111111111111111111111111111101UL)
#define FLASH_CR_MER_CLR_MASK           (0b11111111111111111111111111111011UL)
#define FLASH_CR_SNB_CLR_MASK           (0b11111111111111111111111110000111UL)
#define FLASH_CR_PSIZE_CLR_MASK         (0b11111111111111111111110011111111UL)
#define FLASH_CR_STRT_CLR_MASK          (0b11111111111111101111111111111111UL)
#define FLASH_CR_EOPIE_CLR_MASK         (0b11111110111111111111111111111111UL)
#define FLASH_CR_ERRIE_CLR_MASK         (0b11111101111111111111111111111111UL)
#define FLASH_CR_LOCK_CLR_MASK          (0b01111111111111111111111111111111UL)

/* Bit values (OR with these to set bits) */
#define FLASH_CR_PG                     (0x00000001UL)
#define FLASH_CR_SER                    (0x00000002UL)
#define FLASH_CR_MER                    (0x00000004UL)
#define FLASH_CR_STRT                   (0x00010000UL)
#define FLASH_CR_EOPIE                  (0x01000000UL)
#define FLASH_CR_ERRIE                  (0x02000000UL)
#define FLASH_CR_LOCK                   (0x80000000UL)

/* Sector number values (shifted to correct position) */
#define FLASH_CR_SNB_0                  (0x00000000UL)  /* Sector 0 */
#define FLASH_CR_SNB_1                  (0x00000008UL)  /* Sector 1 */
#define FLASH_CR_SNB_2                  (0x00000010UL)  /* Sector 2 */
#define FLASH_CR_SNB_3                  (0x00000018UL)  /* Sector 3 */
#define FLASH_CR_SNB_4                  (0x00000020UL)  /* Sector 4 */
#define FLASH_CR_SNB_5                  (0x00000028UL)  /* Sector 5 */
#define FLASH_CR_SNB_6                  (0x00000030UL)  /* Sector 6 */
#define FLASH_CR_SNB_7                  (0x00000038UL)  /* Sector 7 */

/* Program size (parallelism) values */
#define FLASH_CR_PSIZE_BYTE             (0x00000000UL)  /* x8  programming */
#define FLASH_CR_PSIZE_HALFWORD         (0x00000100UL)  /* x16 programming */
#define FLASH_CR_PSIZE_WORD             (0x00000200UL)  /* x32 programming */
#define FLASH_CR_PSIZE_DOUBLEWORD       (0x00000300UL)  /* x64 programming */

/******************************************************************************/
/*                    FLASH_OPTCR (Option Control Register)                    */
/*                           Offset: 0x14                                      */
/******************************************************************************/
/* Bit positions */
#define FLASH_OPTCR_OPTLOCK_POS         (0U)
#define FLASH_OPTCR_OPTSTRT_POS         (1U)
#define FLASH_OPTCR_BOR_LEV_POS         (2U)
#define FLASH_OPTCR_WDG_SW_POS          (5U)
#define FLASH_OPTCR_nRST_STOP_POS       (6U)
#define FLASH_OPTCR_nRST_STDBY_POS      (7U)
#define FLASH_OPTCR_RDP_POS             (8U)
#define FLASH_OPTCR_nWRP_POS            (16U)
#define FLASH_OPTCR_SPRMOD_POS          (31U)

/* Bit masks (AND with these to clear bits) */
//                                       0b10987654321098765432109876543210
#define FLASH_OPTCR_OPTLOCK_CLR_MASK    (0b11111111111111111111111111111110UL)
#define FLASH_OPTCR_OPTSTRT_CLR_MASK    (0b11111111111111111111111111111101UL)
#define FLASH_OPTCR_BOR_LEV_CLR_MASK    (0b11111111111111111111111111110011UL)
#define FLASH_OPTCR_WDG_SW_CLR_MASK     (0b11111111111111111111111111011111UL)
#define FLASH_OPTCR_nRST_STOP_CLR_MASK  (0b11111111111111111111111110111111UL)
#define FLASH_OPTCR_nRST_STDBY_CLR_MASK (0b11111111111111111111111101111111UL)
#define FLASH_OPTCR_RDP_CLR_MASK        (0b11111111111111110000000011111111UL)
#define FLASH_OPTCR_nWRP_CLR_MASK       (0b11111111000000001111111111111111UL)
#define FLASH_OPTCR_SPRMOD_CLR_MASK     (0b01111111111111111111111111111111UL)

/* Bit values */
#define FLASH_OPTCR_OPTLOCK             (0x00000001UL)
#define FLASH_OPTCR_OPTSTRT             (0x00000002UL)
#define FLASH_OPTCR_SPRMOD              (0x80000000UL)

/* BOR Level values */
#define FLASH_OPTCR_BOR_LEVEL_3         (0x00000000UL)  /* ~2.70V threshold */
#define FLASH_OPTCR_BOR_LEVEL_2         (0x00000004UL)  /* ~2.40V threshold */
#define FLASH_OPTCR_BOR_LEVEL_1         (0x00000008UL)  /* ~2.10V threshold */
#define FLASH_OPTCR_BOR_OFF             (0x0000000CUL)  /* BOR disabled */

/* Read protection levels */
#define FLASH_OPTCR_RDP_LEVEL_0         (0x0000AA00UL)  /* No protection */
#define FLASH_OPTCR_RDP_LEVEL_1         (0x00000000UL)  /* Read protection active */
#define FLASH_OPTCR_RDP_LEVEL_2         (0x0000CC00UL)  /* PERMANENT! Chip locked! */

/* Write protection bits (nWRP) - 0 = protected, 1 = not protected */
#define FLASH_OPTCR_nWRP_SECTOR_0       (0x00010000UL)
#define FLASH_OPTCR_nWRP_SECTOR_1       (0x00020000UL)
#define FLASH_OPTCR_nWRP_SECTOR_2       (0x00040000UL)
#define FLASH_OPTCR_nWRP_SECTOR_3       (0x00080000UL)
#define FLASH_OPTCR_nWRP_SECTOR_4       (0x00100000UL)
#define FLASH_OPTCR_nWRP_SECTOR_5       (0x00200000UL)
#define FLASH_OPTCR_nWRP_SECTOR_6       (0x00400000UL)
#define FLASH_OPTCR_nWRP_SECTOR_7       (0x00800000UL)
#define FLASH_OPTCR_nWRP_ALL            (0x00FF0000UL)  /* All sectors */

/******************************************************************************/
/*                    FLASH REGISTER STRUCTURE                                 */
/******************************************************************************/
typedef struct {
    volatile uint32_t ACR;      /* Access Control Register,     Offset: 0x00 */
    volatile uint32_t KEYR;     /* Key Register,                Offset: 0x04 */
    volatile uint32_t OPTKEYR;  /* Option Key Register,         Offset: 0x08 */
    volatile uint32_t SR;       /* Status Register,             Offset: 0x0C */
    volatile uint32_t CR;       /* Control Register,            Offset: 0x10 */
    volatile uint32_t OPTCR;    /* Option Control Register,     Offset: 0x14 */
} FLASH_Register_t;

/******************************************************************************/
/*                    TIMEOUT VALUES                                           */
/******************************************************************************/
#define FLASH_TIMEOUT_VALUE             (50000UL)    /* General timeout */
#define FLASH_ERASE_TIMEOUT_VALUE       (500000UL)   /* Sector erase timeout */
#define FLASH_MASS_ERASE_TIMEOUT_VALUE  (2000000UL)  /* Mass erase timeout */

#endif /* FLASH_PRIV_H */