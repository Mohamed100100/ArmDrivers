/******************************************************************************
 * @file    NVIC.H
 * @author  Eng.Gemy
 * @brief   NVIC (Nested Vectored Interrupt Controller) Driver Interface Header File
 *          This file contains all the function prototypes, type definitions,
 *          and enumerations for the NVIC driver to manage interrupts
 * @date    [Current Date]
 * @version 1.0
 * @note    NVIC is part of ARM Cortex-M core and manages:
 *          - Interrupt enable/disable
 *          - Interrupt pending status
 *          - Interrupt priority configuration
 *          - System reset
 ******************************************************************************/

#ifndef MCAL_NVIC_DRIVER_NVIC_H
#define MCAL_NVIC_DRIVER_NVIC_H

/******************************************************************************
 * @brief NVIC Status Enumeration
 * @details Defines all possible return status codes for NVIC operations
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    NVIC_NOT_OK,        /**< Operation failed */
    NVIC_OK,            /**< Operation completed successfully */
    NVIC_NULL_PTR,      /**< Null pointer passed as argument */
}NVIC_Status_t;

/******************************************************************************
 * @brief NVIC Pending Status Enumeration
 * @details Defines interrupt pending states
 * @note Pending means interrupt has been triggered but not yet serviced
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    NVIC_NOT_PENDING,   /**< Interrupt is not pending */
    NVIC_PENDING        /**< Interrupt is pending (waiting to be serviced) */
}NVIC_Pending_t;

/******************************************************************************
 * @brief NVIC Active Status Enumeration
 * @details Defines interrupt active states
 * @note Active means interrupt is currently being serviced
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum {
    NVIC_NOT_ACTIVE,    /**< Interrupt is not active */
    NVIC_ACTIVE         /**< Interrupt is currently being serviced */
}NVIC_Active_t;

/******************************************************************************
 * @brief NVIC IRQ Numbers Enumeration
 * @details Defines all external interrupt request numbers (0-239)
 * @note IRQ numbers correspond to peripheral interrupt positions
 * @note Maximum 240 external interrupts supported by ARM Cortex-M
 * @note Actual available IRQs depend on specific MCU implementation
 * 
 * @note Common STM32F4 IRQ Mappings (examples):
 *       - IRQ0:  WWDG (Window Watchdog)
 *       - IRQ1:  PVD (Programmable Voltage Detector)
 *       - IRQ6:  EXTI0 (External Interrupt Line 0)
 *       - IRQ7:  EXTI1 (External Interrupt Line 1)
 *       - IRQ23: EXTI9_5 (External Interrupts 5-9)
 *       - IRQ37: USART1 global interrupt
 *       - IRQ38: USART2 global interrupt
 *       - IRQ40: EXTI15_10 (External Interrupts 10-15)
 *       - IRQ56: DMA2 Stream 0
 *       Refer to device datasheet for complete mapping
 * 
 * @author Eng.Gemy
 ******************************************************************************/
typedef enum
{
    NVIC_IRQ0   = 0,        /**< External Interrupt 0 */
    NVIC_IRQ1   = 1,        /**< External Interrupt 1 */
    NVIC_IRQ2   = 2,        /**< External Interrupt 2 */
    NVIC_IRQ3   = 3,        /**< External Interrupt 3 */
    NVIC_IRQ4   = 4,        /**< External Interrupt 4 */
    NVIC_IRQ5   = 5,        /**< External Interrupt 5 */
    NVIC_IRQ6   = 6,        /**< External Interrupt 6 */
    NVIC_IRQ7   = 7,        /**< External Interrupt 7 */
    NVIC_IRQ8   = 8,        /**< External Interrupt 8 */
    NVIC_IRQ9   = 9,        /**< External Interrupt 9 */
    NVIC_IRQ10  = 10,       /**< External Interrupt 10 */
    NVIC_IRQ11  = 11,       /**< External Interrupt 11 */
    NVIC_IRQ12  = 12,       /**< External Interrupt 12 */
    NVIC_IRQ13  = 13,       /**< External Interrupt 13 */
    NVIC_IRQ14  = 14,       /**< External Interrupt 14 */
    NVIC_IRQ15  = 15,       /**< External Interrupt 15 */
    NVIC_IRQ16  = 16,       /**< External Interrupt 16 */
    NVIC_IRQ17  = 17,       /**< External Interrupt 17 */
    NVIC_IRQ18  = 18,       /**< External Interrupt 18 */
    NVIC_IRQ19  = 19,       /**< External Interrupt 19 */
    NVIC_IRQ20  = 20,       /**< External Interrupt 20 */
    NVIC_IRQ21  = 21,       /**< External Interrupt 21 */
    NVIC_IRQ22  = 22,       /**< External Interrupt 22 */
    NVIC_IRQ23  = 23,       /**< External Interrupt 23 */
    NVIC_IRQ24  = 24,       /**< External Interrupt 24 */
    NVIC_IRQ25  = 25,       /**< External Interrupt 25 */
    NVIC_IRQ26  = 26,       /**< External Interrupt 26 */
    NVIC_IRQ27  = 27,       /**< External Interrupt 27 */
    NVIC_IRQ28  = 28,       /**< External Interrupt 28 */
    NVIC_IRQ29  = 29,       /**< External Interrupt 29 */
    NVIC_IRQ30  = 30,       /**< External Interrupt 30 */
    NVIC_IRQ31  = 31,       /**< External Interrupt 31 */
    NVIC_IRQ32  = 32,       /**< External Interrupt 32 */
    NVIC_IRQ33  = 33,       /**< External Interrupt 33 */
    NVIC_IRQ34  = 34,       /**< External Interrupt 34 */
    NVIC_IRQ35  = 35,       /**< External Interrupt 35 */
    NVIC_IRQ36  = 36,       /**< External Interrupt 36 */
    NVIC_IRQ37  = 37,       /**< External Interrupt 37 */
    NVIC_IRQ38  = 38,       /**< External Interrupt 38 */
    NVIC_IRQ39  = 39,       /**< External Interrupt 39 */
    NVIC_IRQ40  = 40,       /**< External Interrupt 40 */
    NVIC_IRQ41  = 41,       /**< External Interrupt 41 */
    NVIC_IRQ42  = 42,       /**< External Interrupt 42 */
    NVIC_IRQ43  = 43,       /**< External Interrupt 43 */
    NVIC_IRQ44  = 44,       /**< External Interrupt 44 */
    NVIC_IRQ45  = 45,       /**< External Interrupt 45 */
    NVIC_IRQ46  = 46,       /**< External Interrupt 46 */
    NVIC_IRQ47  = 47,       /**< External Interrupt 47 */
    NVIC_IRQ48  = 48,       /**< External Interrupt 48 */
    NVIC_IRQ49  = 49,       /**< External Interrupt 49 */
    NVIC_IRQ50  = 50,       /**< External Interrupt 50 */
    NVIC_IRQ51  = 51,       /**< External Interrupt 51 */
    NVIC_IRQ52  = 52,       /**< External Interrupt 52 */
    NVIC_IRQ53  = 53,       /**< External Interrupt 53 */
    NVIC_IRQ54  = 54,       /**< External Interrupt 54 */
    NVIC_IRQ55  = 55,       /**< External Interrupt 55 */
    NVIC_IRQ56  = 56,       /**< External Interrupt 56 */
    NVIC_IRQ57  = 57,       /**< External Interrupt 57 */
    NVIC_IRQ58  = 58,       /**< External Interrupt 58 */
    NVIC_IRQ59  = 59,       /**< External Interrupt 59 */
    NVIC_IRQ60  = 60,       /**< External Interrupt 60 */
    NVIC_IRQ61  = 61,       /**< External Interrupt 61 */
    NVIC_IRQ62  = 62,       /**< External Interrupt 62 */
    NVIC_IRQ63  = 63,       /**< External Interrupt 63 */
    NVIC_IRQ64  = 64,       /**< External Interrupt 64 */
    NVIC_IRQ65  = 65,       /**< External Interrupt 65 */
    NVIC_IRQ66  = 66,       /**< External Interrupt 66 */
    NVIC_IRQ67  = 67,       /**< External Interrupt 67 */
    NVIC_IRQ68  = 68,       /**< External Interrupt 68 */
    NVIC_IRQ69  = 69,       /**< External Interrupt 69 */
    NVIC_IRQ70  = 70,       /**< External Interrupt 70 */
    NVIC_IRQ71  = 71,       /**< External Interrupt 71 */
    NVIC_IRQ72  = 72,       /**< External Interrupt 72 */
    NVIC_IRQ73  = 73,       /**< External Interrupt 73 */
    NVIC_IRQ74  = 74,       /**< External Interrupt 74 */
    NVIC_IRQ75  = 75,       /**< External Interrupt 75 */
    NVIC_IRQ76  = 76,       /**< External Interrupt 76 */
    NVIC_IRQ77  = 77,       /**< External Interrupt 77 */
    NVIC_IRQ78  = 78,       /**< External Interrupt 78 */
    NVIC_IRQ79  = 79,       /**< External Interrupt 79 */
    NVIC_IRQ80  = 80,       /**< External Interrupt 80 */
    NVIC_IRQ81  = 81,       /**< External Interrupt 81 */
    NVIC_IRQ82  = 82,       /**< External Interrupt 82 */
    NVIC_IRQ83  = 83,       /**< External Interrupt 83 */
    NVIC_IRQ84  = 84,       /**< External Interrupt 84 */
    NVIC_IRQ85  = 85,       /**< External Interrupt 85 */
    NVIC_IRQ86  = 86,       /**< External Interrupt 86 */
    NVIC_IRQ87  = 87,       /**< External Interrupt 87 */
    NVIC_IRQ88  = 88,       /**< External Interrupt 88 */
    NVIC_IRQ89  = 89,       /**< External Interrupt 89 */
    NVIC_IRQ90  = 90,       /**< External Interrupt 90 */
    NVIC_IRQ91  = 91,       /**< External Interrupt 91 */
    NVIC_IRQ92  = 92,       /**< External Interrupt 92 */
    NVIC_IRQ93  = 93,       /**< External Interrupt 93 */
    NVIC_IRQ94  = 94,       /**< External Interrupt 94 */
    NVIC_IRQ95  = 95,       /**< External Interrupt 95 */
    NVIC_IRQ96  = 96,       /**< External Interrupt 96 */
    NVIC_IRQ97  = 97,       /**< External Interrupt 97 */
    NVIC_IRQ98  = 98,       /**< External Interrupt 98 */
    NVIC_IRQ99  = 99,       /**< External Interrupt 99 */
    NVIC_IRQ100 = 100,      /**< External Interrupt 100 */
    NVIC_IRQ101 = 101,      /**< External Interrupt 101 */
    NVIC_IRQ102 = 102,      /**< External Interrupt 102 */
    NVIC_IRQ103 = 103,      /**< External Interrupt 103 */
    NVIC_IRQ104 = 104,      /**< External Interrupt 104 */
    NVIC_IRQ105 = 105,      /**< External Interrupt 105 */
    NVIC_IRQ106 = 106,      /**< External Interrupt 106 */
    NVIC_IRQ107 = 107,      /**< External Interrupt 107 */
    NVIC_IRQ108 = 108,      /**< External Interrupt 108 */
    NVIC_IRQ109 = 109,      /**< External Interrupt 109 */
    NVIC_IRQ110 = 110,      /**< External Interrupt 110 */
    NVIC_IRQ111 = 111,      /**< External Interrupt 111 */
    NVIC_IRQ112 = 112,      /**< External Interrupt 112 */
    NVIC_IRQ113 = 113,      /**< External Interrupt 113 */
    NVIC_IRQ114 = 114,      /**< External Interrupt 114 */
    NVIC_IRQ115 = 115,      /**< External Interrupt 115 */
    NVIC_IRQ116 = 116,      /**< External Interrupt 116 */
    NVIC_IRQ117 = 117,      /**< External Interrupt 117 */
    NVIC_IRQ118 = 118,      /**< External Interrupt 118 */
    NVIC_IRQ119 = 119,      /**< External Interrupt 119 */
    NVIC_IRQ120 = 120,      /**< External Interrupt 120 */
    NVIC_IRQ121 = 121,      /**< External Interrupt 121 */
    NVIC_IRQ122 = 122,      /**< External Interrupt 122 */
    NVIC_IRQ123 = 123,      /**< External Interrupt 123 */
    NVIC_IRQ124 = 124,      /**< External Interrupt 124 */
    NVIC_IRQ125 = 125,      /**< External Interrupt 125 */
    NVIC_IRQ126 = 126,      /**< External Interrupt 126 */
    NVIC_IRQ127 = 127,      /**< External Interrupt 127 */
    NVIC_IRQ128 = 128,      /**< External Interrupt 128 */
    NVIC_IRQ129 = 129,      /**< External Interrupt 129 */
    NVIC_IRQ130 = 130,      /**< External Interrupt 130 */
    NVIC_IRQ131 = 131,      /**< External Interrupt 131 */
    NVIC_IRQ132 = 132,      /**< External Interrupt 132 */
    NVIC_IRQ133 = 133,      /**< External Interrupt 133 */
    NVIC_IRQ134 = 134,      /**< External Interrupt 134 */
    NVIC_IRQ135 = 135,      /**< External Interrupt 135 */
    NVIC_IRQ136 = 136,      /**< External Interrupt 136 */
    NVIC_IRQ137 = 137,      /**< External Interrupt 137 */
    NVIC_IRQ138 = 138,      /**< External Interrupt 138 */
    NVIC_IRQ139 = 139,      /**< External Interrupt 139 */
    NVIC_IRQ140 = 140,      /**< External Interrupt 140 */
    NVIC_IRQ141 = 141,      /**< External Interrupt 141 */
    NVIC_IRQ142 = 142,      /**< External Interrupt 142 */
    NVIC_IRQ143 = 143,      /**< External Interrupt 143 */
    NVIC_IRQ144 = 144,      /**< External Interrupt 144 */
    NVIC_IRQ145 = 145,      /**< External Interrupt 145 */
    NVIC_IRQ146 = 146,      /**< External Interrupt 146 */
    NVIC_IRQ147 = 147,      /**< External Interrupt 147 */
    NVIC_IRQ148 = 148,      /**< External Interrupt 148 */
    NVIC_IRQ149 = 149,      /**< External Interrupt 149 */
    NVIC_IRQ150 = 150,      /**< External Interrupt 150 */
    NVIC_IRQ151 = 151,      /**< External Interrupt 151 */
    NVIC_IRQ152 = 152,      /**< External Interrupt 152 */
    NVIC_IRQ153 = 153,      /**< External Interrupt 153 */
    NVIC_IRQ154 = 154,      /**< External Interrupt 154 */
    NVIC_IRQ155 = 155,      /**< External Interrupt 155 */
    NVIC_IRQ156 = 156,      /**< External Interrupt 156 */
    NVIC_IRQ157 = 157,      /**< External Interrupt 157 */
    NVIC_IRQ158 = 158,      /**< External Interrupt 158 */
    NVIC_IRQ159 = 159,      /**< External Interrupt 159 */
    NVIC_IRQ160 = 160,      /**< External Interrupt 160 */
    NVIC_IRQ161 = 161,      /**< External Interrupt 161 */
    NVIC_IRQ162 = 162,      /**< External Interrupt 162 */
    NVIC_IRQ163 = 163,      /**< External Interrupt 163 */
    NVIC_IRQ164 = 164,      /**< External Interrupt 164 */
    NVIC_IRQ165 = 165,      /**< External Interrupt 165 */
    NVIC_IRQ166 = 166,      /**< External Interrupt 166 */
    NVIC_IRQ167 = 167,      /**< External Interrupt 167 */
    NVIC_IRQ168 = 168,      /**< External Interrupt 168 */
    NVIC_IRQ169 = 169,      /**< External Interrupt 169 */
    NVIC_IRQ170 = 170,      /**< External Interrupt 170 */
    NVIC_IRQ171 = 171,      /**< External Interrupt 171 */
    NVIC_IRQ172 = 172,      /**< External Interrupt 172 */
    NVIC_IRQ173 = 173,      /**< External Interrupt 173 */
    NVIC_IRQ174 = 174,      /**< External Interrupt 174 */
    NVIC_IRQ175 = 175,      /**< External Interrupt 175 */
    NVIC_IRQ176 = 176,      /**< External Interrupt 176 */
    NVIC_IRQ177 = 177,      /**< External Interrupt 177 */
    NVIC_IRQ178 = 178,      /**< External Interrupt 178 */
    NVIC_IRQ179 = 179,      /**< External Interrupt 179 */
    NVIC_IRQ180 = 180,      /**< External Interrupt 180 */
    NVIC_IRQ181 = 181,      /**< External Interrupt 181 */
    NVIC_IRQ182 = 182,      /**< External Interrupt 182 */
    NVIC_IRQ183 = 183,      /**< External Interrupt 183 */
    NVIC_IRQ184 = 184,      /**< External Interrupt 184 */
    NVIC_IRQ185 = 185,      /**< External Interrupt 185 */
    NVIC_IRQ186 = 186,      /**< External Interrupt 186 */
    NVIC_IRQ187 = 187,      /**< External Interrupt 187 */
    NVIC_IRQ188 = 188,      /**< External Interrupt 188 */
    NVIC_IRQ189 = 189,      /**< External Interrupt 189 */
    NVIC_IRQ190 = 190,      /**< External Interrupt 190 */
    NVIC_IRQ191 = 191,      /**< External Interrupt 191 */
    NVIC_IRQ192 = 192,      /**< External Interrupt 192 */
    NVIC_IRQ193 = 193,      /**< External Interrupt 193 */
    NVIC_IRQ194 = 194,      /**< External Interrupt 194 */
    NVIC_IRQ195 = 195,      /**< External Interrupt 195 */
    NVIC_IRQ196 = 196,      /**< External Interrupt 196 */
    NVIC_IRQ197 = 197,      /**< External Interrupt 197 */
    NVIC_IRQ198 = 198,      /**< External Interrupt 198 */
    NVIC_IRQ199 = 199,      /**< External Interrupt 199 */
    NVIC_IRQ200 = 200,      /**< External Interrupt 200 */
    NVIC_IRQ201 = 201,      /**< External Interrupt 201 */
    NVIC_IRQ202 = 202,      /**< External Interrupt 202 */
    NVIC_IRQ203 = 203,      /**< External Interrupt 203 */
    NVIC_IRQ204 = 204,      /**< External Interrupt 204 */
    NVIC_IRQ205 = 205,      /**< External Interrupt 205 */
    NVIC_IRQ206 = 206,      /**< External Interrupt 206 */
    NVIC_IRQ207 = 207,      /**< External Interrupt 207 */
    NVIC_IRQ208 = 208,      /**< External Interrupt 208 */
    NVIC_IRQ209 = 209,      /**< External Interrupt 209 */
    NVIC_IRQ210 = 210,      /**< External Interrupt 210 */
    NVIC_IRQ211 = 211,      /**< External Interrupt 211 */
    NVIC_IRQ212 = 212,      /**< External Interrupt 212 */
    NVIC_IRQ213 = 213,      /**< External Interrupt 213 */
    NVIC_IRQ214 = 214,      /**< External Interrupt 214 */
    NVIC_IRQ215 = 215,      /**< External Interrupt 215 */
    NVIC_IRQ216 = 216,      /**< External Interrupt 216 */
    NVIC_IRQ217 = 217,      /**< External Interrupt 217 */
    NVIC_IRQ218 = 218,      /**< External Interrupt 218 */
    NVIC_IRQ219 = 219,      /**< External Interrupt 219 */
    NVIC_IRQ220 = 220,      /**< External Interrupt 220 */
    NVIC_IRQ221 = 221,      /**< External Interrupt 221 */
    NVIC_IRQ222 = 222,      /**< External Interrupt 222 */
    NVIC_IRQ223 = 223,      /**< External Interrupt 223 */
    NVIC_IRQ224 = 224,      /**< External Interrupt 224 */
    NVIC_IRQ225 = 225,      /**< External Interrupt 225 */
    NVIC_IRQ226 = 226,      /**< External Interrupt 226 */
    NVIC_IRQ227 = 227,      /**< External Interrupt 227 */
    NVIC_IRQ228 = 228,      /**< External Interrupt 228 */
    NVIC_IRQ229 = 229,      /**< External Interrupt 229 */
    NVIC_IRQ230 = 230,      /**< External Interrupt 230 */
    NVIC_IRQ231 = 231,      /**< External Interrupt 231 */
    NVIC_IRQ232 = 232,      /**< External Interrupt 232 */
    NVIC_IRQ233 = 233,      /**< External Interrupt 233 */
    NVIC_IRQ234 = 234,      /**< External Interrupt 234 */
    NVIC_IRQ235 = 235,      /**< External Interrupt 235 */
    NVIC_IRQ236 = 236,      /**< External Interrupt 236 */
    NVIC_IRQ237 = 237,      /**< External Interrupt 237 */
    NVIC_IRQ238 = 238,      /**< External Interrupt 238 */
    NVIC_IRQ239 = 239       /**< External Interrupt 239 */
} NVIC_IRQ_t;



/******************************************************************************
 *                           FUNCTION PROTOTYPES
 * @brief NVIC driver public API functions
 * @author Eng.Gemy
 ******************************************************************************/

/**
 * @brief Enable external interrupt in NVIC
 * 
 * @param[in] IRQ_Number  Interrupt request number (NVIC_IRQ0 to NVIC_IRQ239)
 * 
 * @return NVIC_Status_t Status of the operation
 * @retval NVIC_OK       Interrupt enabled successfully
 * @retval NVIC_NOT_OK   Invalid IRQ number
 * 
 * @note Enabling interrupt in NVIC allows it to trigger when peripheral requests
 * @note Peripheral interrupt must also be enabled in peripheral registers
 * @note Interrupt will trigger only if both NVIC and peripheral enable bits are set
 * 
 * @example Enable USART1 interrupt (IRQ37 in STM32F4):
 *          NVIC_EnableIRQ(NVIC_IRQ37);
 * 
 * @author Eng.Gemy
 */
NVIC_Status_t NVIC_EnableIRQ(NVIC_IRQ_t );

/**
 * @brief Disable external interrupt in NVIC
 * 
 * @param[in] IRQ_Number  Interrupt request number (NVIC_IRQ0 to NVIC_IRQ239)
 * 
 * @return NVIC_Status_t Status of the operation
 * @retval NVIC_OK       Interrupt disabled successfully
 * @retval NVIC_NOT_OK   Invalid IRQ number
 * 
 * @note Disabling interrupt prevents it from triggering even if peripheral requests
 * @note Pending interrupt will not be serviced until re-enabled
 * @warning Disabling critical interrupts may cause system malfunction
 * 
 * @example Disable USART1 interrupt:
 *          NVIC_DisableIRQ(NVIC_IRQ37);
 * 
 * @author Eng.Gemy
 */
NVIC_Status_t NVIC_DisableIRQ(NVIC_IRQ_t );

/**
 * @brief Get pending status of external interrupt
 * 
 * @param[in]  IRQ_Number    Interrupt request number (NVIC_IRQ0 to NVIC_IRQ239)
 * @param[out] PendingStatus Pointer to store pending status
 * 
 * @return NVIC_Status_t Status of the operation
 * @retval NVIC_OK        Status read successfully
 * @retval NVIC_NOT_OK    Invalid IRQ number
 * @retval NVIC_NULL_PTR  Null pointer passed
 * 
 * @note Pending means interrupt has been triggered but not yet serviced
 * @note Returns NVIC_PENDING or NVIC_NOT_PENDING
 * @note Useful for polling interrupt status without enabling interrupt
 * 
 * @example Check if USART1 interrupt is pending:
 *          NVIC_Pending_t status;
 *          NVIC_GetPendingIRQ(NVIC_IRQ37, &status);
 * 
 * @author Eng.Gemy
 */
NVIC_Status_t NVIC_GetPendingIRQ (NVIC_IRQ_t ,NVIC_Pending_t*);

/**
 * @brief Set interrupt to pending state
 * 
 * @param[in] IRQ_Number  Interrupt request number (NVIC_IRQ0 to NVIC_IRQ239)
 * 
 * @return NVIC_Status_t Status of the operation
 * @retval NVIC_OK       Interrupt set to pending successfully
 * @retval NVIC_NOT_OK   Invalid IRQ number
 * 
 * @note Software can trigger interrupt by setting it to pending
 * @note If interrupt is enabled, it will be serviced immediately
 * @note Useful for software-triggered interrupts or testing ISRs
 * @warning Use carefully to avoid unexpected interrupt behavior
 * 
 * @example Trigger USART1 interrupt from software:
 *          NVIC_SetPendingIRQ(NVIC_IRQ37);
 * 
 * @author Eng.Gemy
 */
NVIC_Status_t NVIC_SetPendingIRQ (NVIC_IRQ_t );

/**
 * @brief Clear pending status of interrupt
 * 
 * @param[in] IRQ_Number  Interrupt request number (NVIC_IRQ0 to NVIC_IRQ239)
 * 
 * @return NVIC_Status_t Status of the operation
 * @retval NVIC_OK       Pending status cleared successfully
 * @retval NVIC_NOT_OK   Invalid IRQ number
 * 
 * @note Clears pending flag to prevent interrupt from triggering
 * @note Useful to cancel pending interrupt before enabling
 * @note Does not affect currently active interrupt
 * @warning Clearing pending flag of unhandled interrupt may cause lost events
 * 
 * @example Clear pending USART1 interrupt:
 *          NVIC_ClearPendingIRQ(NVIC_IRQ37);
 * 
 * @author Eng.Gemy
 */
NVIC_Status_t NVIC_ClearPendingIRQ (NVIC_IRQ_t );

/**
 * @brief Get active status of external interrupt
 * 
 * @param[in]  IRQ_Number   Interrupt request number (NVIC_IRQ0 to NVIC_IRQ239)
 * @param[out] ActiveStatus Pointer to store active status
 * 
 * @return NVIC_Status_t Status of the operation
 * @retval NVIC_OK        Status read successfully
 * @retval NVIC_NOT_OK    Invalid IRQ number
 * @retval NVIC_NULL_PTR  Null pointer passed
 * 
 * @note Active means interrupt is currently being serviced (in ISR)
 * @note Returns NVIC_ACTIVE or NVIC_NOT_ACTIVE
 * @note Useful for debugging nested interrupt scenarios
 * @note Cannot be set by software (read-only status)
 * 
 * @example Check if USART1 interrupt is active:
 *          NVIC_Active_t status;
 *          NVIC_GetActiveIRQ(NVIC_IRQ37, &status);
 * 
 * @author Eng.Gemy
 */
NVIC_Status_t NVIC_GetActiveIRQ (NVIC_IRQ_t ,NVIC_Active_t*);

/**
 * @brief Set priority for external interrupt
 * 
 * @param[in] IRQ_Number  Interrupt request number (NVIC_IRQ0 to NVIC_IRQ239)
 * @param[in] Priority    Priority value (0 = highest priority)
 * 
 * @return NVIC_Status_t Status of the operation
 * @retval NVIC_OK       Priority set successfully
 * @retval NVIC_NOT_OK   Invalid IRQ number or priority
 * 
 * @note Lower priority number = Higher priority level (0 is highest)
 * @note Number of priority bits depends on MCU (typically 4 bits = 16 levels)
 * @note STM32F4 uses 4 bits (0-15), only upper bits are used
 * @note Priority grouping affects how bits are split between group and subpriority
 * @note Interrupts with same priority are serviced in IRQ number order
 * 
 * @example Set USART1 interrupt to priority 5:
 *          NVIC_SetPriority(NVIC_IRQ37, 5);
 * 
 * @warning Higher priority interrupts can preempt lower priority ones
 * @warning Priority 0 should be reserved for critical interrupts
 * 
 * @author Eng.Gemy
 */
NVIC_Status_t NVIC_SetPriority (NVIC_IRQ_t , uint8_t );

/**
 * @brief Get priority of external interrupt
 * 
 * @param[in]  IRQ_Number  Interrupt request number (NVIC_IRQ0 to NVIC_IRQ239)
 * @param[out] Priority    Pointer to store priority value
 * 
 * @return NVIC_Status_t Status of the operation
 * @retval NVIC_OK        Priority read successfully
 * @retval NVIC_NOT_OK    Invalid IRQ number
 * @retval NVIC_NULL_PTR  Null pointer passed
 * 
 * @note Returns the currently configured priority for the interrupt
 * @note Priority value interpretation depends on priority grouping setting
 * 
 * @example Get USART1 interrupt priority:
 *          uint8_t priority;
 *          NVIC_GetPriority(NVIC_IRQ37, &priority);
 * 
 * @author Eng.Gemy
 */
NVIC_Status_t NVIC_GetPriority (NVIC_IRQ_t ,uint8_t*);


/**
 * @brief Trigger system reset
 * 
 * @return NVIC_Status_t Status of the operation (function never returns on success)
 * @retval NVIC_OK       Reset triggered (function will not return)
 * @retval NVIC_NOT_OK   Reset failed to trigger
 * 
 * @note This function triggers a complete system reset
 * @note All registers return to reset values
 * @note Reset is performed via AIRCR register in System Control Block
 * @note Function will not return if reset is successful
 * @warning All volatile data will be lost
 * @warning Backup domain registers are NOT reset (RTC, backup SRAM)
 * 
 * @example Trigger system reset:
 *          NVIC_SystemReset();  // System will reset and restart
 * 
 * @author Eng.Gemy
 */
NVIC_Status_t NVIC_SystemReset (void);

/**
 * @brief Set priority grouping for interrupt priorities
 * 
 * @param[in] PriorityGroup  Priority grouping value
 *                           Defines split between group priority and subpriority
 * 
 * @return NVIC_Status_t Status of the operation
 * @retval NVIC_OK       Priority grouping set successfully
 * @retval NVIC_NOT_OK   Invalid grouping value
 * 
 * @note Priority grouping splits priority bits into:
 *       - Group priority: Determines preemption (higher group can interrupt lower)
 *       - Subpriority: Determines order when group priority is same
 * 
 * @note Common grouping values for 4-bit priority (STM32F4):
 *       - 0x00: 0 bits group, 4 bits sub (no preemption)
 *       - 0x04: 1 bit group, 3 bits sub
 *       - 0x05: 2 bits group, 2 bits sub
 *       - 0x06: 3 bits group, 1 bit sub
 *       - 0x07: 4 bits group, 0 bits sub (full preemption)
 * 
 * @note Formula: PRIGROUP value = (PriorityGroup - 3)
 *       AIRCR register PRIGROUP field encoding
 * 
 * @example Set 2 bits for group priority, 2 bits for subpriority:
 *          NVIC_SetPriorityGrouping(0x05);
 * 
 * @warning Changing priority grouping affects all interrupt priorities
 * @warning Should be set once at system initialization
 * 
 * @author Eng.Gemy
 */
NVIC_Status_t NVIC_SetPriorityGrouping(uint32_t);
 

#endif /* MCAL_NVIC_DRIVER_NVIC_H */

