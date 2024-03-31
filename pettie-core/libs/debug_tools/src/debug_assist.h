#ifndef _DEBUG_ASSIST_H
#define _DEBUG_ASSIST_H

#ifdef __cplusplus
extern "C" {
#endif


// ----------------------------------------------------------------------
// Core functions

/**
 * @brief 
 */
void init_debug_assist(void);

/**
 * @brief Launching System stat plotter
 * 
 * @note Must be called after all task creation!
 */
void debug_assist_start(void);


#ifdef __cplusplus
}
#endif


#endif // _DEBUG_ASSIST_H