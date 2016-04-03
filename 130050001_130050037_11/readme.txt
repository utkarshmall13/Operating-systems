130050001 Sourabh Ghurye
130050037 Utkarsh Mall

Changes made only in bbfs.c no changes made in makefile.
Changes in bbfs.c:
    only in bb_read() and bb_write() bb_getattr() function
    see "//==============="  comment to get start and end of changes

We are using XOR encryption each character is read and is "XORed" with a key
Also before storing to write null character using pwrite:
    if character is not NULL(\0) each character is written twice
    if character is NULL any 2 different nonz zero characters are written randomly,which are not equal to each other.

encryption increases size by twice but is safe in terms of null charachter reading.

We tested code using simple commands such as wc,cat etc.
We also used editor to open and edit over file multiple times.
