/*==============================================================================
 * LINKER SCRIPT FOR ARM CORTEX-M MICROCONTROLLER
 * This script defines how the compiled object files are organized in memory
 *============================================================================*/

/*------------------------------------------------------------------------------
 * ENTRY POINT DEFINITION
 * Specifies the first function to execute when the microcontroller starts
 *----------------------------------------------------------------------------*/
ENTRY(Reset_Handler)  /* Reset_Handler is called immediately after power-on/reset */

/*------------------------------------------------------------------------------
 * STACK POINTER INITIALIZATION
 * The stack grows downward from high memory addresses to low addresses
 *----------------------------------------------------------------------------*/
_estack = ORIGIN(RAM) + LENGTH(RAM);  /* Set stack pointer to the END of RAM */
                                       /* ARM Cortex-M uses a descending stack */
                                       /* This ensures stack starts at highest RAM address */

/*------------------------------------------------------------------------------
 * MINIMUM HEAP AND STACK SIZE REQUIREMENTS
 * These values ensure sufficient memory is allocated for dynamic allocation
 * and function call stack frames
 *----------------------------------------------------------------------------*/
_Min_Heap_Size = 0x200;   /* Minimum heap size = 512 bytes */
                          /* Heap is used for dynamic memory allocation (malloc, new) */

_Min_Stack_Size = 0x400;  /* Minimum stack size = 1024 bytes */
                          /* Stack is used for local variables, function parameters, */
                          /* and return addresses during function calls */

/*------------------------------------------------------------------------------
 * MEMORY REGIONS DEFINITION
 * Defines the physical memory layout of the microcontroller
 * Format: MEMORY_NAME (attributes) : ORIGIN = start_address, LENGTH = size
 * Attributes: r=read, w=write, x=execute
 *----------------------------------------------------------------------------*/
MEMORY
{
  /* RAM - Volatile memory for runtime data storage */
  RAM (xrw)    : ORIGIN = 0x20000000,   LENGTH = 64K
  /* xrw = executable, readable, writable */
  /* Starting address: 0x20000000 (typical for ARM Cortex-M SRAM) */
  /* Size: 64 KB = 65,536 bytes */
  
  /* FLASH - Non-volatile memory for program code and constants */
  FLASH (rx)   : ORIGIN = 0x8000000,    LENGTH = 256K
  /* rx = readable, executable (but NOT writable) */
  /* Starting address: 0x08000000 (typical for ARM Cortex-M Flash) */
  /* Size: 256 KB = 262,144 bytes */

  /*****************************************************************************/
  /*************************** Added Section ***********************************/
  /*************************** GEMY Section  ***********************************/
  /*************************** Post Build Config *******************************/
  /*****************************************************************************/
  /* this section won't be flashed in the ECU unless we make it NOLOAD as 
     No Flash bank connected to 0x20001000 (RAM) */
  LED_CFG_RAM   (xrw) :ORIGIN = 0x20000800 , LENGTH = 10
  
  /* this section will be loaded at the flash with read only because it is at flash */
  LED_CFG_FLASH (xrw) :ORIGIN = 0x08008000 , LENGTH = 10
}

/*------------------------------------------------------------------------------
 * SECTIONS DEFINITION
 * Defines how different types of data are organized within the memory regions
 *----------------------------------------------------------------------------*/
SECTIONS
{
  /*----------------------------------------------------------------------------
   * INTERRUPT VECTOR TABLE SECTION (.isr_vector)
   * Contains function pointers for exception handlers and interrupt routines
   * MUST MUST MUST be at the very beginning of FLASH memory
   *--------------------------------------------------------------------------*/
  .isr_vector :
  {
    . = ALIGN(4);              /* Align to 4-byte boundary (required for ARM) */
                               /* '.' is the location counter (current address) */
    
    KEEP(*(.isr_vector))       /* KEEP prevents linker from removing this even if "unused" */
                               /* *(.isr_vector) means: all .isr_vector sections from all files */
                               /* Contains: Stack pointer, Reset handler, NMI, HardFault, etc. */
    
    . = ALIGN(4);              /* Ensure next section starts on 4-byte boundary */
  } >FLASH                     /* Place this section in FLASH memory */

  /*----------------------------------------------------------------------------
   * PROGRAM CODE SECTION (.text)
   * Contains all executable instructions and related data
   *--------------------------------------------------------------------------*/
  .text :
  {
    . = ALIGN(4);              /* Align to 4-byte boundary */
    
    *(.text)                   /* All .text sections (main code functions) */
    *(.text*)                  /* All sections starting with .text (e.g., .text.main, .text.startup) */
    
    *(.glue_7)                 /* Linker-generated ARM-to-Thumb interworking code */
                               /* Used when ARM code calls Thumb code */
    
    *(.glue_7t)                /* Linker-generated Thumb-to-ARM interworking code */
                               /* Used when Thumb code calls ARM code */
    
    *(.eh_frame)               /* Exception handling frame information */
                               /* Used for C++ exceptions and stack unwinding */

    KEEP (*(.init))            /* Initialization functions (C++ constructors setup) */
                               /* Called before main() */
    
    KEEP (*(.fini))            /* Finalization functions (C++ destructors cleanup) */
                               /* Called after main() returns */

    . = ALIGN(4);              /* Align to 4-byte boundary */
    _etext = .;                /* Global symbol marking END of code section */
                               /* Can be referenced in C code as: extern uint32_t _etext; */
  } >FLASH                     /* Place this section in FLASH memory */

  /*----------------------------------------------------------------------------
   * READ-ONLY DATA SECTION (.rodata)
   * Contains constant data that never changes (const variables, string literals)
   *--------------------------------------------------------------------------*/
  .rodata :
  {
    . = ALIGN(4);              /* Align to 4-byte boundary */
    
    *(.rodata)                 /* All .rodata sections (read-only data) */
                               /* Contains: const int x = 5; char* str = "hello"; etc. */
    
    *(.rodata*)                /* All sections starting with .rodata */
    
    . = ALIGN(4);              /* Align to 4-byte boundary */
  } >FLASH                     /* Place in FLASH (constants don't need to be in RAM) */

  /*----------------------------------------------------------------------------
   * ARM EXCEPTION TABLES (.ARM.extab and .ARM.exidx)
   * Used for C++ exception handling and stack unwinding
   * Required for proper exception propagation in C++ code
   *--------------------------------------------------------------------------*/
  .ARM.extab :
  {
    . = ALIGN(4);              /* Align to 4-byte boundary */
    *(.ARM.extab* .gnu.linkonce.armextab.*)  /* Exception handling table entries */
    . = ALIGN(4);              /* Align to 4-byte boundary */
  } >FLASH                     /* Place in FLASH memory */

  .ARM : 
  {
    . = ALIGN(4);              /* Align to 4-byte boundary */
    __exidx_start = .;         /* Mark start of exception index table */
    *(.ARM.exidx*)             /* Exception index table (links code to exception handlers) */
    __exidx_end = .;           /* Mark end of exception index table */
    . = ALIGN(4);              /* Align to 4-byte boundary */
  } >FLASH                     /* Place in FLASH memory */

  /*----------------------------------------------------------------------------
   * CONSTRUCTOR/DESTRUCTOR ARRAYS
   * Used by C++ runtime to call constructors before main() and destructors
   * after main(). Also used by GCC's __attribute__((constructor/destructor))
   *--------------------------------------------------------------------------*/
  
  /* Pre-initialization array - called before .init_array */
  .preinit_array :
  {
    . = ALIGN(4);                           /* Align to 4-byte boundary */
    PROVIDE_HIDDEN (__preinit_array_start = .);  /* Hidden symbol (not exported) */
                                                   /* Marks start of array */
    KEEP (*(.preinit_array*))               /* Array of function pointers */
    PROVIDE_HIDDEN (__preinit_array_end = .);    /* Marks end of array */
    . = ALIGN(4);                           /* Align to 4-byte boundary */
  } >FLASH

  /* Initialization array - C++ constructors for global/static objects */
  .init_array :
  {
    . = ALIGN(4);                           /* Align to 4-byte boundary */
    PROVIDE_HIDDEN (__init_array_start = .);  /* Start marker symbol */
    
    KEEP (*(SORT(.init_array.*)))          /* Sort by priority (e.g., .init_array.00100) */
                                            /* Lower numbers run first */
    KEEP (*(.init_array*))                 /* Include all init_array sections */
    
    PROVIDE_HIDDEN (__init_array_end = .);   /* End marker symbol */
    . = ALIGN(4);                           /* Align to 4-byte boundary */
  } >FLASH

  /* Finalization array - C++ destructors for global/static objects */
  .fini_array :
  {
    . = ALIGN(4);                           /* Align to 4-byte boundary */
    PROVIDE_HIDDEN (__fini_array_start = .);  /* Start marker symbol */
    
    KEEP (*(SORT(.fini_array.*)))          /* Sort by priority */
    KEEP (*(.fini_array*))                 /* Include all fini_array sections */
    
    PROVIDE_HIDDEN (__fini_array_end = .);   /* End marker symbol */
    . = ALIGN(4);                           /* Align to 4-byte boundary */
  } >FLASH

  /*----------------------------------------------------------------------------
   * DATA INITIALIZATION MARKER
   * .data section needs to be copied from FLASH to RAM at startup
   * _sidata stores the FLASH address where .data is stored
   *--------------------------------------------------------------------------*/
  _sidata = LOADADDR(.data);   /* LOADADDR gets the FLASH address of .data */
                               /* Used by startup code to copy data from FLASH to RAM */

  /*----------------------------------------------------------------------------
   * INITIALIZED DATA SECTION (.data)
   * Contains initialized global and static variables
   * Stored in FLASH but copied to RAM at startup (because RAM is writable)
   * Format: } >RAM AT> FLASH means: runtime location is RAM, but stored in FLASH
   *--------------------------------------------------------------------------*/
  .data :
  {
    . = ALIGN(4);              /* Align to 4-byte boundary */
    _sdata = .;                /* Global symbol: START of .data in RAM */
                               /* Used by startup code to know where to copy data */
    
    *(.data)                   /* All .data sections (initialized variables) */
                               /* Example: int x = 10; (global or static) */
    
    *(.data*)                  /* All sections starting with .data */
    
    *(.RamFunc)                /* Functions that execute from RAM (not FLASH) */
                               /* Used for functions that need to run faster */
                               /* or that modify FLASH (can't execute from FLASH while writing to it) */
    
    *(.RamFunc*)               /* All sections starting with .RamFunc */

    . = ALIGN(4);              /* Align to 4-byte boundary */
    _edata = .;                /* Global symbol: END of .data in RAM */
                               /* Used by startup code to know when to stop copying */

  } >RAM AT> FLASH             /* Runtime location: RAM, Storage location: FLASH */
                               /* Startup code copies from FLASH (_sidata) to RAM (_sdata to _edata) */

  /*----------------------------------------------------------------------------
   * UNINITIALIZED DATA SECTION (.bss)
   * Contains uninitialized or zero-initialized global and static variables
   * Only exists in RAM (not stored in FLASH because it's all zeros)
   * Startup code initializes this entire section to zero
   *--------------------------------------------------------------------------*/
  . = ALIGN(4);                /* Align location counter to 4-byte boundary */
  
  .bss :
  {
                               /* There are startup codes work with __bss_start__ And there are working with __bss_start__ */
                               /* So we will define the Two Alias */
    _sbss = .;                 /* Global symbol: START of .bss section */
    __bss_start__ = _sbss;     /* Alternative name for compatibility */
                               /* Used by startup code to know where to start zeroing */
    
    *(.bss)                    /* All .bss sections (uninitialized variables) */
                               /* Example: int x; (global or static, not initialized) */
    
    *(.bss*)                   /* All sections starting with .bss */
    
    *(COMMON)                  /* Common symbols (uninitialized globals with weak linkage) */
                               /* Used for variables defined without 'extern' in headers */

    . = ALIGN(4);              /* Align to 4-byte boundary */
    _ebss = .;                 /* Global symbol: END of .bss section */
    __bss_end__ = _ebss;       /* Alternative name for compatibility */
                               /* Used by startup code to know when to stop zeroing */
  } >RAM                       /* Only exists in RAM (not stored in FLASH) */

  /*********************************************************************************/
  /****************************** GEMY added this section inside LED_CFG Memory ****/
  /*********************************************************************************/
  


  .led_cfg_flash_section () :
  {
    . = ALIGN(4);
    _sled_cfg = .;
    *(.led_cfg_flash_section)
    *(.led_cfg_flash_section*)
  } >LED_CFG_FLASH

  /*----------------------------------------------------------------------------
   * HEAP AND STACK RESERVATION SECTION
   * Reserves space for heap (dynamic allocation) and stack (function calls)
   * This section doesn't contain actual data, just reserves space
   * Ensures linker error if not enough RAM is available
   *--------------------------------------------------------------------------*/
  ._user_heap_stack :
  {
    . = ALIGN(8);              /* Align to 8-byte boundary (recommended for stack) */
                               /* ARM EABI requires 8-byte stack alignment */
    
    PROVIDE ( end = . );       /* Symbol 'end' marks end of .bss (start of heap) */
    PROVIDE ( _end = . );      /* Alternative name used by malloc/sbrk */
                               /* Heap grows upward from this address */
    
    . = . + _Min_Heap_Size;    /* Reserve minimum heap space (0x200 = 512 bytes) */
                               /* Linker will error if not enough RAM */
    
    . = . + _Min_Stack_Size;   /* Reserve minimum stack space (0x400 = 1024 bytes) */
                               /* Stack grows downward from _estack (defined at top) */
                               /* This reservation ensures stack won't collide with heap */
    
    . = ALIGN(8);              /* Align to 8-byte boundary */
  } >RAM                       /* Reserve space in RAM */
    . = ALIGN(4);
  
  .led_cfg_ram_section (NOLOAD) :
  {
    _sled_cfg = .;
    *(.led_cfg_ram_section)
    *(.led_cfg_ram_section*)
  } >LED_CFG_RAM
  /*----------------------------------------------------------------------------
   * DISCARD SECTION
   * Removes unnecessary sections from standard libraries to reduce binary size
   * These sections are typically not needed in embedded systems
   *--------------------------------------------------------------------------*/
  /DISCARD/ :
  {
    libc.a ( * )               /* Remove all sections from standard C library */
                               /* Example: printf formatting code you might not use */
    
    libm.a ( * )               /* Remove all sections from math library */
                               /* Example: floating-point functions you might not use */
    
    libgcc.a ( * )             /* Remove all sections from GCC runtime library */
                               /* Example: 64-bit arithmetic helpers on 32-bit systems */
  }
  /* Note: Only removes these if they're not referenced by your code */

  /*----------------------------------------------------------------------------
   * ARM ATTRIBUTES SECTION
   * Contains metadata about ARM-specific build attributes
   * Not loaded into memory, only stored in ELF file for debugging tools
   *--------------------------------------------------------------------------*/
  .ARM.attributes 0 : { *(.ARM.attributes) }   /* 0 means don't allocate memory address */
                                               /* Contains: architecture version, */
                                               /* floating-point ABI, etc. */


}

/*==============================================================================
 * MEMORY MAP SUMMARY:
 * 
 * FLASH (0x08000000 - 0x0803FFFF): 256 KB
 *   - Interrupt vectors
 *   - Program code (.text)
 *   - Read-only data (.rodata)
 *   - Initialization data for .data section
 *   - Constructor/destructor arrays
 * 
 * RAM (0x20000000 - 0x2000FFFF): 64 KB
 *   - Initialized data (.data) - copied from FLASH at startup
 *   - Uninitialized data (.bss) - zeroed at startup
 *   - Heap (grows upward)
 *   - Stack (grows downward from _estack)
 * 
 * STARTUP SEQUENCE:
 *   1. CPU loads initial stack pointer from 0x08000000
 *   2. CPU loads Reset_Handler address from 0x08000004 and jumps to it
 *   3. Reset_Handler copies .data from FLASH (_sidata) to RAM (_sdata to _edata)
 *   4. Reset_Handler zeros .bss section (_sbss to _ebss)
 *   5. Reset_Handler calls functions in .preinit_array
 *   6. Reset_Handler calls functions in .init_array (C++ constructors)
 *   7. Reset_Handler calls main()
 *   8. If main() returns, calls functions in .fini_array (C++ destructors)
 *============================================================================*/