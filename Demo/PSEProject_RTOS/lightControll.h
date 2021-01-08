/**
 * \file: lightControll.h
 *
 * \brief Simulation of the light controll system.
 *
 *
 */
#pragma once

#include <stdint.h>

/******************************************************************
*
* Data types definitions
*
******************************************************************/

/**
* External inputs status (locked/unlocked)
*
*/
typedef enum{
    Unlocked,           /**< External inputs change the system */
    Locked              /**< External inputs dont change the system */
} ext_input_status;

/**
* Button status (wait/pressed)
*
*/
typedef enum{
    b_Wait,             /**< Waiting for push button interrupt */
    b_Pressed           /**< Button pushed */
} button_status;

/**
* Terminal events
*
*/
typedef enum{
    t_Wait,                 /**< Waiting for terminal input interrupt */
    t_On_Off,               /**< Turn on/off the led */
    t_More_intensity,       /**< Increase led intensity */
    t_Less_intensity,       /**< Decrease led intensity */
    t_Min_Intensity,        /**< Set min led intensity */
    t_Max_Intensity,        /**< Set max led intensity */
    t_LDR_Led_On_Value,     /**< Set on value or value to mantain */
} terminal_status;

/**
* Mode of execution
*
*/
typedef enum { 
    ManualMode, 
    Dimmer, 
    AutomaticSwitch, 
    AutomaticAjust
} lightControllState;

/* System state variables */
extern button_status OnStatus;
extern terminal_status TerminalStatus;
extern ext_input_status ExtInputStatus;
extern int Intensity_Stored_Value;
extern int Intensity_Value;
extern int Min_Intensity;
extern int Max_Intensity;
extern int LDR_Led_On_Value;
extern int LDR_Value;