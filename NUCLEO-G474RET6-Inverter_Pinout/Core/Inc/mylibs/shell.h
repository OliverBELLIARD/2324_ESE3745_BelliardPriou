/*
 * shell.h
 *
 *  Created on: Oct 1, 2023
 *      Author: nicolas
 */

#ifndef INC_MYLIBS_SHELL_H_
#define INC_MYLIBS_SHELL_H_

/**
 * @file shell.h
 * @brief Header file for the shell interface.
 *
 * This header defines constants, macros, and function prototypes for the shell
 * interface used in the project. The shell provides a command-line interface
 * over UART for controlling various functionalities of the system.
 */

/**
 * @brief UART receive buffer size.
 *
 * Defines the size of the UART receive buffer.
 */
#define UART_RX_BUFFER_SIZE 1

/**
 * @brief UART transmit buffer size.
 *
 * Defines the size of the UART transmit buffer.
 */
#define UART_TX_BUFFER_SIZE 64

/**
 * @brief Command buffer size.
 *
 * Defines the size of the command buffer used to store incoming commands.
 */
#define CMD_BUFFER_SIZE 64

/**
 * @brief Maximum number of command arguments.
 *
 * Defines the maximum number of arguments a single command can have.
 */
#define MAX_ARGS 9

/**
 * @brief ASCII value for line feed.
 *
 * Line feed (LF) character used in command-line communication.
 */
#define ASCII_LF 0x0A

/**
 * @brief ASCII value for carriage return.
 *
 * Carriage return (CR) character used in command-line communication.
 */
#define ASCII_CR 0x0D

/**
 * @brief ASCII value for backspace.
 *
 * Backspace character used for editing command input.
 */
#define ASCII_BACK 0x08

/**
 * @brief Initializes the shell interface.
 *
 * Sets up the shell, initializes buffers, and configures UART interrupts
 * for receiving input. Displays a welcome message and starts the prompt.
 */
void Shell_Init(void);

/**
 * @brief Main loop for the shell interface.
 *
 * Processes received UART input, parses commands, and executes the corresponding
 * functionality. Supports various commands for controlling PWM, resetting the
 * system, and reading the current.
 */
void Shell_Loop(void);

/**
 * @brief Sets the PWM duty cycle ratio.
 *
 * @param ratio The desired PWM duty cycle ratio (0.0 to 1.0).
 */
void set_PWM_ratio(double ratio);

/**
 * @brief Sets the PWM speed.
 *
 * @param speed The desired PWM speed, in absolute units.
 */
void set_PWM_speed(int speed);

/**
 * @brief Starts the PWM generation.
 *
 * Configures and starts the PWM signal generation on the TIM1 channels.
 */
void start_PWM(void);

/**
 * @brief Stops the PWM generation.
 *
 * Disables the PWM signal generation on the TIM1 channels.
 */
void stop_PWM(void);

/**
 * @brief Resets the microcontroller.
 *
 * Performs a system reset using the NVIC_SystemReset() function.
 */
void reset_inverter(void);

/**
 * @brief Reads and prints the current value.
 *
 * Reads the ADC value, converts it to the current measurement, and outputs
 * the result via UART.
 */
void read_current(void);

#endif /* INC_MYLIBS_SHELL_H_ */
