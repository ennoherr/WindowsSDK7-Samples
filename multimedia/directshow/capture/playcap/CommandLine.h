#pragma once

#include <Windows.h>
#include <tchar.h>

//-----------------------------------------------------------------------------------
// The macro CommandLineToArgv maps to a function that converts
// a command-line string to argc and argv similar to the ones
// in the standard main function.  If this code is compiled for
// unicode, the build-in Windows API function is used, otherwise
// a non-unicode non-API version is used for compatibility with
// Windows versions that have no unicode support.
#ifdef UNICODE
#define CommandLineToArgv CommandLineToArgvW
#include <shellapi.h>
#else
#define CommandLineToArgv CommandLineToArgvT
#endif

LPTSTR *WINAPI CommandLineToArgvT(LPCTSTR lpCmdLine, int *lpArgc)
{
	HGLOBAL hargv;
	LPTSTR *argv, lpSrc, lpDest, lpArg;
	int argc, nBSlash;
	BOOL bInQuotes;

	// If null was passed in for lpCmdLine, there are no arguments
	if (!lpCmdLine) {
		if (lpArgc)
			*lpArgc = 0;
		return 0;
	}

	lpSrc = (LPTSTR)lpCmdLine;
	// Skip spaces at beginning
	while (*lpSrc == _T(' ') || *lpSrc == _T('\t'))
		lpSrc++;

	// If command-line starts with null, there are no arguments
	if (*lpSrc == 0) {
		if (lpArgc)
			*lpArgc = 0;
		return 0;
	}

	lpArg = lpSrc;
	argc = 0;
	nBSlash = 0;
	bInQuotes = FALSE;

	// Count the number of arguments
	while (1) {
		if (*lpSrc == 0 || ((*lpSrc == _T(' ') || *lpSrc == _T('\t')) && !bInQuotes)) {
			// Whitespace not enclosed in quotes signals the start of another argument
			argc++;

			// Skip whitespace between arguments
			while (*lpSrc == _T(' ') || *lpSrc == _T('\t'))
				lpSrc++;
			if (*lpSrc == 0)
				break;
			nBSlash = 0;
			continue;
		}
		else if (*lpSrc == _T('\\')) {
			// Count consecutive backslashes
			nBSlash++;
		}
		else if (*lpSrc == _T('\"') && !(nBSlash & 1)) {
			// Open or close quotes
			bInQuotes = !bInQuotes;
			nBSlash = 0;
		}
		else {
			// Some other character
			nBSlash = 0;
		}
		lpSrc++;
	}

	// Allocate space the same way as CommandLineToArgvW for compatibility
	hargv = GlobalAlloc(0, argc * sizeof(LPTSTR) + (_tcslen(lpArg) + 1) * sizeof(TCHAR));
	argv = (LPTSTR *)GlobalLock(hargv);

	if (!argv) {
		// Memory allocation failed
		if (lpArgc)
			*lpArgc = 0;
		return 0;
	}

	lpSrc = lpArg;
	lpDest = lpArg = (LPTSTR)(argv + argc);
	argc = 0;
	nBSlash = 0;
	bInQuotes = FALSE;

	// Fill the argument array
	while (1) {
		if (*lpSrc == 0 || ((*lpSrc == _T(' ') || *lpSrc == _T('\t')) && !bInQuotes)) {
			// Whitespace not enclosed in quotes signals the start of another argument
			// Null-terminate argument
			*lpDest++ = 0;
			argv[argc++] = lpArg;

			// Skip whitespace between arguments
			while (*lpSrc == _T(' ') || *lpSrc == _T('\t'))
				lpSrc++;
			if (*lpSrc == 0)
				break;
			lpArg = lpDest;
			nBSlash = 0;
			continue;
		}
		else if (*lpSrc == _T('\\')) {
			*lpDest++ = _T('\\');
			lpSrc++;

			// Count consecutive backslashes
			nBSlash++;
		}
		else if (*lpSrc == _T('\"')) {
			if (!(nBSlash & 1)) {
				// If an even number of backslashes are before the quotes,
				// the quotes don't go in the output
				lpDest -= nBSlash / 2;
				bInQuotes = !bInQuotes;
			}
			else {
				// If an odd number of backslashes are before the quotes,
				// output a quote
				lpDest -= (nBSlash + 1) / 2;
				*lpDest++ = _T('\"');
			}
			lpSrc++;
			nBSlash = 0;
		}
		else {
			// Copy other characters
			*lpDest++ = *lpSrc++;
			nBSlash = 0;
		}
	}

	if (lpArgc)
		*lpArgc = argc;
	return argv;
}

