# xtrFile


Extract file from damaged/ defective media using JPEG markers. 
Designed for Windows uses "CreateFile()" to read media as a single file. Intended for large and very large capacity media.
Everything in a single file for simplicity; this is not a project but a code sample to be partially reused in another project

A part of a project I started long time ago to retrieve lost files on damaged media like Floppy, HDD, CD, tape. Only JPEG files (JFIF header)
Note: In the meantime, the frame contains more data, like geo-coordinates and quite a thumbnail of the image, usually in JPEG format self - so, we have JPEG in JPEG that adds comlexity to the process, in the respect that we can have something like  "[BOF][......][BOF][...][EOF][............][EOF]"

Reads raw disk sector as a single file (no intermediary storage) amd then outputs the result in separate file(s) in another media. Reads everything, regardless allocated or non-allocaed space, so, doesn't take care if there is an allocation table or not.

This serves just for information.  NOT FULLY TESTED AND IMPLEMENTED (pre-Alpha)


