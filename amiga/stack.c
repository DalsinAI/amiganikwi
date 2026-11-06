/* A Shell gives 4 KB of stack; libnix's startup sets this much up instead.
 * The AmiCradle Team. zlib licence, like Nikwi. */
unsigned long __stack = 256 * 1024;
static const char version[] __attribute__((used)) = "$VER: Nikwi 0.0.20120213 Amiga (06.11.2026)";
