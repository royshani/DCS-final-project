#include "../header/flash.h"  // Include the header file for Flash functions and definitions
#include "../header/halGPIO.h" // Include GPIO functions (not used in this snippet but might be needed)
#include "string.h"  // Include string functions for handling flash_buffer

Files file;  // Define the global 'file' variable of type 'Files'

// Function to update the size of the file
void ScriptData(void) {
    // Set the size of the most recent file (indexed by num_of_files - 1)
    // to the length of flash_buffer minus 1
    file.file_size[file.num_of_files - 1] = strlen(flash_buffer) - 1;
}

// Function to write data to Flash memory
void write_Seg(void) {
    char* Flash_ptr_write;  // Pointer to Flash memory
    unsigned int k;         // Loop index

    // Initialize Flash pointer to point to the location of the most recent file
    Flash_ptr_write = file.file_ptr[file.num_of_files - 1];
    // Prepare for Flash segment erase
     if (erased_flash==0){
         FCTL1 = FWKEY + ERASE;
         erased_flash = 1;

     }
//    FCTL1 = FWKEY + ERASE;  // Set the Erase bit in the Flash control register
    FCTL3 = FWKEY;          // Clear the Lock bit to allow writing to Flash

    *Flash_ptr_write = 0;   // Perform a dummy write to erase the Flash segment

    // Prepare for Flash write operation
    FCTL1 = FWKEY + WRT;    // Set the Write bit to enable writing to Flash

    // Write data to Flash memory
    for (k = 0; k < file.file_size[file.num_of_files - 1]; k++) {
        // Skip newline characters (line feed and carriage return)
        if (flash_buffer[k] == 0x0A || flash_buffer[k] == 0x0D) {
            continue;  // Skip to the next character
        }
        *Flash_ptr_write++ =flash_buffer[k];  // Write the character to Flash and move the pointer
    }


    // Finalize the Flash operation
    FCTL1 = FWKEY;           // Clear the Write bit to stop further writes
    FCTL3 = FWKEY + LOCK;    // Set the Lock bit to lock the Flash segment
}

//************************************************************************
// Write phi and steps in flash
//************************************************************************
void write_SegC (int value1 , int value2)
{
  int *Flash_ptr1, *Flash_ptr2;                          // Flash pointer


  Flash_ptr1 = (int *) 0x103C;              // Initialize Flash pointer
  Flash_ptr2 = (int *) 0x103E;              // Initialize Flash pointer
   if (erased_flash == 0){
       FCTL1 = FWKEY + ERASE; // Set the Erase bit in the Flash control register
       FCTL3 = FWKEY;          // Clear the Lock bit to allow writing to Flash
       *Flash_ptr1 = 0;   // Perform a dummy write to erase the Flash segment
       erased_flash = 1;
    }


  FCTL3 = FWKEY;          // Clear the Lock bit to allow writing to Flash
  FCTL1 = FWKEY + WRT;                      // Set WRT bit for write operation

  *Flash_ptr1 = value1;
  *Flash_ptr2 = value2;

  FCTL1 = FWKEY;                            // Clear WRT bit
  FCTL3 = FWKEY + LOCK;                     // Set LOCK bit
}


