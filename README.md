# Compression-Using-Logical-Operations
Performing Compression of bytes using Logical Operations
Uploaded : 08 Sept 2022

This was a fun project made after my attempt to make a compression algorithm of my own.
The above algorithm works best for text files and numerical data (of small values).

The compressor usings AND or OR (determinable) logical operations to compress bytes in groups of N (determinable). 
The compression ratio is on average above 100% for binary data and about 90% for text i.e every 10 letters will be represented by 9 characters if N is set to 10 in AND mode (10-AND), or every 8 characters will be represented by approx 7.4 characters.

The decompressor can then pass the compressed values using the same compression configurations to get back the original data.

These tools can be used for simple encryption where the settings are combined to form a long combination used to encrypt e.g
Encryption done with (10-AND),(5-OR),(7-AND) can only be decrypted with the reverse order (7-AND),(5-OR),(10-AND)

Documentation : Coming soon!

Code Versions in Java and Python : Coming soon!
