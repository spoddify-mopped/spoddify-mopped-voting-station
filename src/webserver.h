/**
 * Web server
 */

#include <stdint.h>

/**
 * Timestamp to trigger restart
 */
extern uint32_t restartTime;

/**
 * Start the web server
 */
void webserver_start(bool isSetup);