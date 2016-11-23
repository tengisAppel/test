/** @file Stand-alone printf implementation. Note function is appended with an
 * 'n' character to ensure that collisions with stdio's printf are avoided. */

#ifndef PRINTFN_H_
#define PRINTFN_H_

/**
 * @brief  Outputs a formatted string on the DBGU stream, using a variable number of
 *         arguments.
 *
 * @param  pFormat  Format string.
 */
signed int printfn(const char *pFormat, ...);

#endif /* PRINTFN_H_ */
