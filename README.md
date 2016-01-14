# csefsck
File System Checker

As with any filesystem, there exists the possibility that errors will be introduced.  This code is custom designed for our file system type so that it can examine everything to check consistent.  

The File System Checker checks the following:
1)	DeviceID
2)	All times are in the past
3)	Free Block List includes all the free blocks and no files/directories stored on items should be listed in the free block list
4)	Each directory contains . and .. and their block numbers are correct
5)	Each directory’s link count matches the number of links in the filename_to_inode_dict
6)	If the data contained in a location pointer is an array, that indirect is one
7)	The size is valid for the number of block pointers in the location array. The three possibilities are:
    a.	size<blocksize if  indirect=0 and size>0
    b.	size<blocksize*length of location array if indirect!=0
    c.	size>blocksize*(length of location array-1) if indirect !=0
