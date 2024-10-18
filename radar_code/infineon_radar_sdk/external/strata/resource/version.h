#define NULL_TERM "\0"
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#if defined (__GNUC__) || defined(__MINGW__)
// windres doesn't handle strings so well
#define RC_STR(x) STR_HELPER(x) NULL_TERM
#else
#define RC_STR(x) x NULL_TERM
#endif
