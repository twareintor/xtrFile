/************************************** xtrFile.c *********************************************/
/************ code by Twareintor ************ *************************************************/
/**** *****************************************************************************************
* PRE-ALPHA. NOT FULLY TESTED NOR IMPLEMENTED. 
* SERVES AS EXAMPLE OLNY. PART OF ANOTHER PROJECT OR GROUP OF PROJECTS. ANY USE IN ANOTHER
* PROJECT OR FOR ANOTHER PURPOSE MAY GET UNEXPECTED RESULTS. PLEASE USE CAREFULY AFTER DEEP
* INSPECTION AND CONSISTENT ADAPTATION OR MODIFICATION THAT MAY BE NEEDED
***********************************************************************************************/

#include <windows.h>
#include <sys/stat.h> // to get the lenght (size) of a file

#define MEGABYTE 	0x00100000				//     1.048.576	Bytes
#define GIGABYTE	0x40000000				// 1.073.741.824	Bytes
#define JPEGFILE	{0xff, 0xd8, 0xff, 0xe8, 0xff, 0xd9}
#define MIN_SIZE	0x00008000				//        32.768	Bytes
#define MAX_SIZE	0x00800000				//     4.194.304 * 2 = 8.388.608 Bytes

#define MBP_PACK	0x00000080				// 			 128
#define LOG_FILP	"LOG_%04d.TXT"							// for logging, this time, in the program directory

#define LOG_FILE	"x:\\tmq\\log_files\\LOG_%04d.TXT"		// log file
#define TESTDISD	"\\\\.\\Y:"					// disk to be tested
#define TESTDISK	"x:\\tnc\\all.arc.mie"				// disk to be tested
#define TESTDISF	"\\\\.\\Y:"					// disk to be tested
#define TESTPATH	""						// Pattern for recovered files
#define TESTRDSK	"x:\\tmq\\diskimage\\DISK%04d.REC"		// Pattern for recovered files
#define TESTRIMG	"x:\\tmq\\recovered\\XIMG%04d.JPG"		// Pattern for recovered files


/**
 * Extracts file from damaged (allocation table of the) media to be inspected
 */
int CXExtrFile();

/**
 *  saves the recovered data in a file
 * recovered file will be saved in a new file: a different volume than the scaned one have to be selected as destination
 */
int CXSaveBufferInNewFile(const char* szPat, int iPos, char* szBuf, DWORD dwBytes);

/**
 * independent testing the result (return) of CXSaveBufferInNewFile()
 */
void TEST__CXSaveBufferInNewFile();

/**
 * loads the second parameter with the data read back from the media
 */
short CXReadSect(const char*, char*, LARGE_INTEGER*, DWORD, DWORD*);

/**
 * determines the amount (or limit) or data to be read
 */
DWORD CXBytesToBeRead(); 		// TO BE DEVELOPED: Actually returns the system available memory

/**
 * prints a specified text in a specified file, appends or creates new file - BOOL flag
 */
int CXLogPrint(const char*, int, const char*, BOOL);	// FOR DEBUG ONLY

/**
 * returns true if the (path/to/)filename "szFnm" exists
 */
inline bool CXFileExists(const char *szFNm);



int main()
{
	// CXExtrFile();	
	return 0;
}


DWORD CXBytesToBeRead()
{
	// sets the number of bytes to be read, per cycle, based on available memory of the system:
	DWORD				dwBytes = 0; 		// Output value . 
	/*
	MEMORYSTATUSEX 		msMemStat;	// will get the memory status, to see how bit will be the 
										// 		... buffer of extracted data...

	GlobalMemoryStatusEx(&msMemStat);
	dwBytes = msMemStat.ullAvailPhys;
	*/	
	return dwBytes;
}

int CXExtrFile()
{
	char				szDskRecPat[] = TESTRDSK;	// for debug only - where the packets of szBytes ...
	char				szFilRecPat[] = TESTRIMG; // for release: the path and file pattern recovered
	char				szTem[] = {	0xff, 0xd8, 0xff, 0xe0, 0x00, 0x10, 0x45, 0x78, 0x69, 0x66, 0x00, // header
									0xff, 0x01, 0xff, 0xfe, 	// Markers from FF01 till FFFE, whereas the
									0xff, 0xd0, 0xff, 0xd9, 	// .. FFD0...FFD7, and FFD8, FFD9 are excluded
									0xff, 0xd9 					// EOI
									};		// JPG - template
	const char     		*_dsk = TESTDISF;    // disk to access
	// const char			*_dsk = TESTDISK;		// test file to access, for debug phase...
	// const char			*_dsk = TESTDISD;		// test file to access, for debug phase...
	char				*szVol = new char;
	LARGE_INTEGER		liPos;					// position	wherein the file pointer is to be moved
	HANDLE				hBuf;					// handle to buffer input .... large buffer
	char				*szBuf;					// bytes recovered from the affected disk opened as file
	DWORD				dwBytes = MBP_PACK*MEGABYTE;// number of bytes to be read from the source	
	DWORD				w = 0; 					// counts the bytes of the packet with lenght of "dwBytesRead" 
	DWORD				dwBytesRead = 0;		// number of bytes read from teh source
	char				*szMsg = new char;		// message for debug or release..3.
	int					nMaxGiga = 1;			// maximum gigabytes, in our case, 8
	int 				iRec = 0;				// counts the recovered files...
	bool				bRec = FALSE;			// when TRUE, start recording, when FALSE again, stop recording
	ULONG				u = 0;					// counts the bytes recorded into the recovery buffer
	ULONG				u0 = 0;					// stores the last position of "u", if the recording is not complete
												// 		at the end of the reading-input buffer "szBuf"
												// 		WARNING!!! some adjustment required for no extra or minus byte! WARNING!!!
	short 				nHAD = 0;				// stands for "number of bytes of header or application data"; 
	short 				iHAD = 0;				// counts the bytes containing header or application data, 
												// 		if iHAD<nHAD, we are inside such a packet and the EOI must be ignored!!!
	    
	DWORD				dwSectorsPerCluster = 0;
    DWORD				dwBytesPerSector = 0;
    DWORD				dwNumberOfFreeClusters = 0;
    DWORD				dwTotalNumberOfClusters = 0;

	// dwBytes = CXBytesToBeRead();
	szMsg = (char*)GlobalAlloc(GPTR, MAX_PATH);
	szVol = (char*)GlobalAlloc(GPTR, MAX_PATH);		
	/*
	memcpy(szVol, _dsk+4, strlen(_dsk)-4);	// just to get the letter of the volume, to transform "\\\\.\\F:" in "F:\\"
	memset(szVol+strlen(szVol), '\\', 1);
    if(GetDiskFreeSpace(szVol, &dwSectorsPerCluster, &dwBytesPerSector,	&dwNumberOfFreeClusters, &dwTotalNumberOfClusters))
	{
		wsprintf(
				szMsg, 
				"Info about Volume %s:\r\n %d sectors per cluster\r\n%d bytes per sector\r\n%d free clusters\r\n%d total clusters", 
				szVol, 
				dwSectorsPerCluster, 
				dwBytesPerSector, 
				dwNumberOfFreeClusters, 
				dwTotalNumberOfClusters
				);
	}
	else
	{
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), 
              MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), szMsg, 256, NULL);
		wsprintf(szMsg, "Error (%d) = \"%s\"", GetLastError(), szMsg);
	}
	MessageBox(NULL, szMsg, NULL, MB_OK);	// displays infos about the volume, or an error if infos are not available
	GlobalFree(szVol);
	*/

	HANDLE		hRec;
	char 		*szRec = new char;

	hRec = GlobalAlloc(GHND, MAX_SIZE);
	szRec = (char*)GlobalLock(hRec);
	memset(szRec, '\0', MAX_SIZE);
	CXLogPrint(LOG_FILE, 1, (char*)"begin of log records: _______________ \r\n\r\n", TRUE);
	// here a loop to read from the volume to be recovered and to write recovered file in the target directory
	for(int j=0; j<=1; j++)	// per packet, every 2GB large = the HighPart...
	{
		liPos.u.HighPart = j;					// increases in multiples of 2*GIGABYTE
		wsprintf(szMsg, "j==%d\r\n", j);																// DEBUG ONLY
		CXLogPrint(LOG_FILE, 1, szMsg, FALSE);														// DEBUG ONLY	
		for(int i=0; i<=0x1f; i++)				// 32*64*MEGABYTE = 2*GIGABYTE; enough...''';;;;'']]]]
		{
			// liPos.u.LowPart = i*1024*dwBytesPerSector*dwSectorsPerCluster; 
			// this time, LowPart will be a multiple of with dwBytes (to be read)
			liPos.u.LowPart = i*dwBytes;			// pointer-position in the disk to be read
			wsprintf(szMsg, "\ti==%d, j==%d, i*dwBytes==%d\r\n", i, j, i*dwBytes);						// DEBUG ONLY
			CXLogPrint(LOG_FILE, 1, szMsg, FALSE);													// DEBUG ONLY	
			hBuf = GlobalAlloc(GHND, dwBytes);		// how many bytes per packet
			szBuf = (char*)GlobalLock(hBuf);
			memset(szBuf, '\0', dwBytes);
			CXReadSect(_dsk, szBuf, &liPos, dwBytes, &dwBytesRead);
			// code to isolate the array of chars containing file to be recovered // to do something with the buffer
			u = 0;				// set the index of the recording array to zero anyway
			if(dwBytesRead>0)		// if something to read is anyway...
			{
				w = 0;
				while(w<=dwBytesRead) // reading every byte in the packet - TODO: Something to optimize here??? jumps or somethin like this!
				{
					if(TRUE)	// detecting the header of the file to be recovered...
					{
						if((memcmp(szBuf+w, szTem, 2)==0)&&((memcmp(szBuf+w+6, szTem+6, 5)==0)||(FALSE)))		// if BOF (and BOI) to be recovered encountered..
						{
							bRec = TRUE;		// start recording, 
							if(TRUE)
							{
								// iHAD = 0;			// reset counting the number of bytes containing header or application data...
								// nHAD = szBuf[w+2]*256+szBuf[w+3];
							}
							wsprintf(szMsg, 
									"\t\tJPG BOI found (0x%x 0x%x 0x%x 0x%x),  j==%d, i==%d, w==%d w==0x%08x, u==%d, u0==%d, nHAD==%d\r\n", 
									szBuf[w], szBuf[w+1], szBuf[w+2], szBuf[w+3], j, i, w, w, u, u0, nHAD);						// FOR DEBUG ONLY
							CXLogPrint(LOG_FILE, 1, szMsg, FALSE);					// FOR DEBUG ONLY
						}
					}
					if(bRec)
					{
						if(szBuf[w]==0xff)	// identifying header, application or headers data that _can_ include SOI or EOI!!!
						{									// 			don't skip them, but if EOI reached, is to be ignored!!!
							char q = szBuf[w+1];							// 
							if(((q>=szTem[12])&&(q<=szTem[16]))||((q>=szTem[14])&&(q<=szTem[18])))
							{
								// iHAD = 0;			// reset counting the number of bytes containing header or application data...
								// nHAD = szBuf[w+2]*256+szBuf[w+3];
							}
						}
						if(	// WARNING!!!!! possible segmentation fault here because the attempt to read after the size of "szBuf!!!"	
							((memcmp(szBuf+w+1, szTem, 2)==0)&&((memcmp(szBuf+w+7, szTem+6, 5)==0)||(FALSE))&&(u>1))|| // corrupted JPG, but save a part of the previous anyway
							// ((memcmp(szBuf+w-2*(w>2), szTem+19, 2))&&(u>MIN_SIZE)&&((TRUE)))||	// EOF reached, or	/// WARNUNG!!!!!!!
							((u>MAX_SIZE)&&(TRUE))||									// maximum excepted size of a file; but save the result anyway
							FALSE
						)
						{	
							wsprintf(szMsg, // WARNING!!!! also segmentation fault possible here, because the attempt to read beyond szBuf size!!!
									"\t\tJPG EOI found (0x%x 0x%x 0x%x 0x%x), j==%d, i==%d, w==%d, w==0x%08x, u==%d, u0==%d, nHAD==%d\r\n", 
									szBuf[w+1], szBuf[w+2], szBuf[w+3], szBuf[w+4], j, i, w, w, u, u0, nHAD);						// FOR DEBUG ONLY
							CXLogPrint(LOG_FILE, 1, szMsg, FALSE);																					// FOR DEBUG ONLY
							wsprintf(szMsg, 
									"\t\tszRec=%d, u0=%d, szRec+u0==%d, szBuf==%d, w==%d, u==%d, u==%d\r\n", 
									szRec, u0, szRec+u0, szBuf, w, u, u);			// FOR DEBUG ONLY
							CXLogPrint(LOG_FILE, 1, szMsg, FALSE);					// FOR DEBUG ONLY
							if((u>=MIN_SIZE))										// finally write it in an array
							{
								memcpy(szRec+u0, szBuf+w-u, u);	// as many bytes, is from previously
								CXSaveBufferInNewFile(szFilRecPat, 0x01, szRec, u);
								iRec++;
							}
							bRec = FALSE;
							memset(szRec, '\0', MAX_SIZE);		// reset the array of bytes to be ready for the next recovered file
							u = 0;		// reset the number of bytes to be copied anyway
							
						}
						else
						{
							u++;		// if recording, keep going, till to the end of the file
							// iHAD+=(iHAD<32000);				// as well as the counting of bytes since a header or application data was encountered...
						}
						if((w>=dwBytesRead)&&(TRUE))			// end-of-packet reached, after BOI encountered, but no EOI encountered...
						{
							if(TRUE)
							{
								memcpy(szRec+u0, szBuf+w-u, u);	// as many bytes, is from previously
								CXSaveBufferInNewFile(szFilRecPat, 0x01, szRec, u);
							}
							memset(szRec, '\0', MAX_SIZE);		// reset the array of bytes to be ready for the next recovered file								
							u = 0;
						}
					}
					w++;
				}
				u0 = u;	// if any, store the value of "u" (bytes recovered count) for the next packet
			}
			else
			{
				CXLogPrint(LOG_FILE, 1, "\t>>>>Empty Packet\r\n", 0);
			}
			GlobalUnlock(hBuf);
			GlobalFree(hBuf);
			CloseHandle(hBuf);
			// till here!
		}
	
	}
	GlobalUnlock(hRec);
	CloseHandle(hRec);
	// clear/ destroy the buffer of the message...
	GlobalFree(szMsg);
	// and a final message, for everyone to know the proces is finished
	char *szEndMsg = new char;
	szEndMsg = (char*)GlobalAlloc(GPTR, MAX_PATH);
	wsprintf(
				szEndMsg, 
				"Done!\r\n%d Megabytes grabbed,\r\n%d x 2GB == %d x 64MB x %dB;\r\n%d files recovered", 
				0x1f*1*dwBytes/MEGABYTE, 1, 0x1f, dwBytes, iRec
			);
	CXLogPrint(LOG_FILE, 1, szEndMsg, 0);			
	CXLogPrint(LOG_FILE, 1, "__end of log records: _______________ \r\n", FALSE);			
	MessageBox(NULL, szEndMsg, "Scan complete!", MB_OK);
	GlobalFree(szEndMsg);
	// then return to calling procedure...
	return 0x1f*1;	

}


short CXReadSect(
               const char     	*_dsk,    		// disk to access
               char           	*_buff,         // buffer where sector will be stored
               LARGE_INTEGER	*pliPos,		// position inside the file...
               DWORD			dwBytes, 		// bytes to be read
			   DWORD			*pdwBytesRead 		// bytes read
               )
{
    HANDLE          hDisk;
    //REM.M01 char 			*szMsg = new char;
    
    //REM.M01 szMsg = (char*)GlobalAlloc(GPTR, MAX_PATH);
    hDisk=CreateFile(_dsk, GENERIC_READ, FILE_SHARE_VALID_FLAGS, 0, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING, 0);
    if(hDisk==INVALID_HANDLE_VALUE) // this may happen if another program is already reading from disk
    {  
        MessageBox(NULL, "Invalid disk", _dsk, MB_OK);
        CloseHandle(hDisk);
        return 0x12;
    }
 	if(SetFilePointerEx(hDisk, *pliPos, NULL, FILE_BEGIN)) // which sector to read
 	{
 		ReadFile(hDisk, _buff, dwBytes, pdwBytesRead, 0);  // read sector
 	}
 	else
 	{
 		MessageBox(NULL, "File pointer not set", NULL, MB_OK);
 	}
 	//REM.M01 wsprintf(szMsg, "bytes read = %d", *pdwBytesRead);
	//REM.M01 MessageBox(NULL, szMsg, NULL, MB_OK);
	//REM.M01 delete[] szMsg;
    CloseHandle(hDisk);
    return 0;
    
}


int CXSaveBufferInNewFile(const char* szPat, int iPos, char* szBuf, DWORD dwBytes)
{
	HANDLE 				hFOut;				// output file		
	// HANDLE				hFind;				// handle to find data...// actually not in use...
	WIN32_FIND_DATA		fndDat;				// find data - to check if the file already exists
	char				*szFNm = new char;	// builds the file name based on the pattern szPat;
	char				*szMsg = new char;	// a message to ...
	DWORD				dwBytesRead;		// 
	
	szFNm = (char*)GlobalAlloc(GPTR, MAX_PATH);
	szMsg = (char*)GlobalAlloc(GPTR, MAX_PATH);
	wsprintf(szFNm, szPat, iPos);
	// hFind = FindFirstFile(szFNm, &fndDat);
	while(CXFileExists(szFNm))
	{
		iPos+=(iPos<10000);
		wsprintf(szFNm, szPat, iPos);		
	}
	/*
	while(hFind!=INVALID_HANDLE_VALUE)		// then the file already exists!!!
	{
		CloseHandle(hFind);		// otherwise, a lot of handles will remain opened...
		iPos+=(iPos<10000);		// at 10000, resets to zero again... / but no more CREATE_NEW in CreateFile()!!!			
		wsprintf(szFNm, szPat, iPos);
		hFind = FindFirstFile(szFNm, &fndDat); // this loop sets the next iPos available, if the file already exists 
		
	}
	CloseHandle(hFind);
	*/
	hFOut = CreateFile(szFNm, GENERIC_WRITE, FILE_SHARE_DELETE|FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFOut==INVALID_HANDLE_VALUE)
	{
		wsprintf(szMsg, "could not create/ write in file:\r\n %s", szFNm);
		MessageBox(NULL, szMsg, szFNm, MB_OK);
	}
	else
	{
		// wsprintf(szMsg, "bytes = %d", dwBytes);
		WriteFile(hFOut, szBuf, dwBytes, &dwBytesRead, NULL);
		return iPos;			// on success
	}
	CloseHandle(hFOut);
	GlobalFree(szFNm);
	GlobalFree(szMsg);
	return 0;
}

int CXLogPrint(const char* szPat, int iPos, const char* szWhat, BOOL bNew)
{
	HANDLE 				hFOut;				// output file		
	// HANDLE				hFind;				// handle to find data...// actually not in use...
	WIN32_FIND_DATA		fndDat;				// find data - to check if the file already exists
	char				*szFNm = new char;	// builds the file name based on the pattern szPat;
	char				*szMsg = new char;	// a message to ...
	DWORD				dwBytesRead;		// 
	
	szFNm = (char*)GlobalAlloc(GPTR, MAX_PATH);
	szMsg = (char*)GlobalAlloc(GPTR, MAX_PATH);
	wsprintf(szFNm, szPat, iPos);
	// hFind = FindFirstFile(szFNm, &fndDat);
	while(CXFileExists(szFNm))
	{
		iPos+=(iPos<10000);
		wsprintf(szFNm, szPat, iPos);		
	}
	/*
	while(hFind!=INVALID_HANDLE_VALUE)		// then the file already exists!!!
	{
		CloseHandle(hFind);
		iPos+=(iPos<10000);		// at 10000, resets to zero again... / but no more CREATE_NEW in CreateFile()!!!			
		wsprintf(szFNm, szPat, iPos);
		hFind = FindFirstFile(szFNm, &fndDat); // this loop sets the next iPos available, if the file already exists 
	}
	CloseHandle(hFind);
	*/
	if(!bNew)
	{
		iPos--;
		wsprintf(szFNm, szPat, iPos);
	}
	hFOut = CreateFile(szFNm, GENERIC_WRITE, 7, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	// WARNING!!! the 3rd parameter of "CreateFile()" must be 7 if you want to append something to it!!!!!! WARNING!!!
	if(hFOut==INVALID_HANDLE_VALUE)
	{
		// wsprintf(szMsg, "Last Error = %d", GetLastError());
		wsprintf(szMsg, "could not create/ write in file:\r\n %s", szFNm);
		MessageBox(NULL, szMsg, szFNm, MB_OK);
	}
	else
	{
		// wsprintf(szMsg, "bytes = %d", dwBytes);
		if(!bNew)
		{
			SetFilePointer(hFOut, 0, 0, FILE_END);
			// // LockFileEx(hFOut, LOCKFILE_FAIL_IMMEDIATELY, 0, 0, 0, NULL);
		}
		WriteFile(hFOut, szWhat, strlen(szWhat), &dwBytesRead, NULL);
		return iPos;			// on success
	}
	
	CloseHandle(hFOut);
	GlobalFree(szFNm);
	GlobalFree(szMsg);
	return 0;
}


inline bool CXFileExists(const char *szFNm)
{	// from "http://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c"
	struct stat buffer;   
	return (stat (szFNm, &buffer) == 0); 
}


void TEST__CXSaveBufferInNewFile()
{
	
	HANDLE		hBuf;
	DWORD		dwBytes = 0xffff;
	char 		*szBuf = new char;
	int 		i = 0;
	
	hBuf = GlobalAlloc(GHND, dwBytes);
	szBuf = (char*)GlobalLock(hBuf);
	// here code to fill the szBuf
	*(szBuf+i)=0x20;
	while(i<dwBytes-1)
	{
		*(szBuf+i++) = rand()%0xff;
	}
	CXSaveBufferInNewFile("d:\\tmq\\recovered\\RIMG%04d.JPG", 20, szBuf, dwBytes);
	GlobalUnlock(hBuf);
	CloseHandle(hBuf);		
	
}




