//  ===== Date Created: 28 April, 2020 ===== 

#if !defined(COMMON_HPP)
#define COMMON_HPP

#define INTERNAL_PASTE_TOKEN_0(x, y) x ## y
#define INTERNAL_PASTE_TOKEN(x, y) INTERNAL_PASTE_TOKEN_0(x, y)

#define ERROR_ONCE_INTERNAL(line, msg, ...)								\
	static bool INTERNAL_PASTE_TOKEN(hasErroredOnce, line) = false;\
	if (!INTERNAL_PASTE_TOKEN(hasErroredOnce, line))\
	{\
		printf(msg, __VA_ARGS__);\
		INTERNAL_PASTE_TOKEN(hasErroredOnce, line) = true;\
	}

#define ERROR_ONCE(msg, ...) ERROR_ONCE_INTERNAL(__LINE__, msg, __VA_ARGS__)

#endif
