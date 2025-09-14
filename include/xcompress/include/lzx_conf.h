#ifndef LZX_CONF_H
#define LZX_CONF_H

#ifdef _WIN32
    #if defined(LZX_EXPORTS)
        #define LZX_API __declspec(dllexport)
    #elif defined(LZX_STATIC)
        #define LZX_API
    #else
        #define LZX_API __declspec(dllimport)
    #endif
#else
    #define LZX_API
#endif

/* return codes */
#define DECR_OK 0
#define DECR_DATA_FORMAT 1
#define DECR_ILLEGAL_DATA 2
#define DECR_NO_MEMORY 3

#define ENCODER_SUCCESS			0
#define ENCODER_READ_FAILURE 	1
#define ENCODER_WRITE_FAILURE 	2
#define ENCODER_NO_MEMORY	    3

#endif // LZX_CONF_H