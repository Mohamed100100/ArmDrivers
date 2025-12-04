#ifndef SWITCH_CFG_H
#define SWITCH_CFG_H

/*
 * Enumeration of all available switch/button names in the system
 * Each switch has a unique identifier for easy reference in application code
 * SWITCH_LEN must always be the last element to track the total number of switches
 */
typedef enum {
    SWITCH1_ON_KIT,     /* External switch/button #1 on development kit */
    SWITCH_LEN          /* Total number of switches - used for array sizing and bounds checking */
}SWITCH_Name_t;

#endif /* SWITCH_CFG_H */