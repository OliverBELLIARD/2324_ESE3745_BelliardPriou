/**
 * @file shell.c
 * @brief Shell command interface for the Nucleo-STM32G474 microcontroller.
 *
 * Provides a basic shell interface to execute commands via UART.
 * Commands include setting PWM parameters, starting/stopping PWM, resetting
 * the microcontroller, and measuring current.
 *
 * @author Nicolas
 * @date October 1, 2023
 */

#include "usart.h"
#include "mylibs/shell.h"
#include "tim.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/** @brief Shell prompt message. */
uint8_t prompt[] = "user@Nucleo-STM32G474RET6>>";

/** @brief Startup message displayed on UART when shell initializes. */
uint8_t started[] =
    "\r\n*-----------------------------*"
    "\r\n| Welcome on Nucleo-STM32G474 |"
    "\r\n*-----------------------------*\r\n";

/** @brief Newline character sequence for UART. */
uint8_t newline[] = "\r\n";

/** @brief Backspace character sequence for UART. */
uint8_t backspace[] = "\b \b";

/** @brief Error message displayed when an unknown command is entered. */
uint8_t cmdNotFound[] = "Command not found\r\n";

/** @brief Response for the "WhereisBrian?" command. */
uint8_t brian[] = "Brian is in the kitchen\r\n";

/** @brief Flag indicating UART receive event. */
uint8_t uartRxReceived;

/** @brief UART receive buffer. */
uint8_t uartRxBuffer[UART_RX_BUFFER_SIZE];

/** @brief UART transmit buffer. */
uint8_t uartTxBuffer[UART_TX_BUFFER_SIZE];

/** @brief Help message listing available commands. */
uint8_t helpMessage[] =
    "\r\nAvailable commands:"
    "\r\n- help\tDisplays this help message."
    "\r\n- ratio\tSets a new cyclic ratio."
    "\r\n- speed\tSets a new PWM speed."
    "\r\n- start\tStarts the PWM generation."
    "\r\n- stop\tStops the PWM generation."
    "\r\n- reset\tResets the microcontroller."
    "\r\n- current\tPrints the measured current of U."
    "\r\n";

/** @brief Command buffer to store user input. */
char cmdBuffer[CMD_BUFFER_SIZE];

/** @brief Index for the command buffer. */
int idx_cmd;

/** @brief Argument vector storing parsed command arguments. */
char *argv[MAX_ARGS];

/** @brief Argument count for the parsed command. */
int argc = 0;

/** @brief Token for parsing commands. */
char *token;

/** @brief Flag indicating a new command is ready to process. */
int newCmdReady = 0;

/**
 * @brief Initializes the shell interface.
 *
 * Sets up UART reception, clears buffers, and displays the startup message.
 */
void Shell_Init(void){
	memset(argv, NULL, MAX_ARGS*sizeof(char*));
	memset(cmdBuffer, NULL, CMD_BUFFER_SIZE*sizeof(char));
	memset(uartRxBuffer, NULL, UART_RX_BUFFER_SIZE*sizeof(char));
	memset(uartTxBuffer, NULL, UART_TX_BUFFER_SIZE*sizeof(char));

	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
	HAL_UART_Transmit(&huart2, started, strlen((char *)started), HAL_MAX_DELAY);
	HAL_UART_Transmit(&huart2, prompt, strlen((char *)prompt), HAL_MAX_DELAY);
}

/**
 * @brief Main loop of the shell.
 *
 * Processes received UART data, parses commands, and executes them.
 */
void Shell_Loop(void){
	if(uartRxReceived){
		switch(uartRxBuffer[0]){
		case ASCII_CR: // Nouvelle ligne, instruction à traiter
			HAL_UART_Transmit(&huart2, newline, sizeof(newline), HAL_MAX_DELAY);
			cmdBuffer[idx_cmd] = '\0';
			argc = 0;
			token = strtok(cmdBuffer, " ");
			while(token!=NULL){
				argv[argc++] = token;
				token = strtok(NULL, " ");
			}
			idx_cmd = 0;
			newCmdReady = 1;
			break;
		case ASCII_BACK: // Suppression du dernier caractère
			cmdBuffer[idx_cmd--] = '\0';
			HAL_UART_Transmit(&huart2, backspace, sizeof(backspace), HAL_MAX_DELAY);
			break;

		default: // Nouveau caractère
			cmdBuffer[idx_cmd++] = uartRxBuffer[0];
			HAL_UART_Transmit(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE, HAL_MAX_DELAY);
		}
		uartRxReceived = 0;
	}

	if(newCmdReady){
		if(strcmp(argv[0],"WhereisBrian?")==0){
			HAL_UART_Transmit(&huart2, brian, sizeof(brian), HAL_MAX_DELAY);
		}
		else if(strcmp(argv[0],"help")==0){
			HAL_UART_Transmit(&huart2, helpMessage, strlen((char *)helpMessage), HAL_MAX_DELAY);
		}
		else if(strcmp(argv[0],"ratio")==0){
			if (argc > 1) {
				set_PWM_ratio(atof(argv[1]));
			}
		}
		else if(strcmp(argv[0],"speed")==0){
			if (argc > 1) {
				set_PWM_speed(atoi(argv[1]));
			}
		}
		else if(strcmp(argv[0],"start")==0){
			start_PWM();
		}
		else if(strcmp(argv[0],"stop")==0){
			stop_PWM();
		}
		else if(strcmp(argv[0],"reset")==0){
			reset_inverter();
		}
		else if(strcmp(argv[0],"current")==0){
			read_current();
		}
		else{
			HAL_UART_Transmit(&huart2, cmdNotFound, sizeof(cmdNotFound), HAL_MAX_DELAY);
		}
		HAL_UART_Transmit(&huart2, prompt, sizeof(prompt), HAL_MAX_DELAY);
		newCmdReady = 0;
	}
}

/**
 * @brief UART receive complete callback.
 *
 * Handles reception of a single character from UART.
 *
 * @param huart UART handle
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef * huart){
	uartRxReceived = 1;
	HAL_UART_Receive_IT(&huart2, uartRxBuffer, UART_RX_BUFFER_SIZE);
}
