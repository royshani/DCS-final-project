#ifndef HEADER_FLASH_H_  // Include guard to prevent multiple inclusions
#define HEADER_FLASH_H_
#include <msp430g2553.h>  // Include the MSP430 header file for specific definitions

// Function declarations
extern void ScriptData(void);   // Function to set file size
extern void SetPtrData(void);  // Function to set file pointers
extern void write_Seg(void);   // Function to write data to Flash

// Define a structure to hold file information
typedef struct Files {
    short num_of_files;         // Number of files
    char file_name[11];         // Array to store file names (up to 10 characters + null terminator)
    char* file_ptr[3];           // Array of pointers to file locations in Flash (up to 3 files)
    int file_size[3];           // Array to store sizes of each file (up to 3 files)
} Files;

extern Files file;  // Declaration of an external variable 'file' of type 'Files'
extern void write_int_flash();
extern void write_SegC (int value1 , int value2);
#endif /* HEADER_FLASH_H_ */  // End of include guard
