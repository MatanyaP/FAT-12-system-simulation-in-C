/*****************************************************************************
	my_format.c
	Matanya Peretz 303021679
	
	format a file system to FAT12.
 ****************************************************************************/

/**     Includes     ************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "fat12.h"


// Boot sector values
#define BOOT_JMP                {0xeb, 0x3c, 0x90}
#define OEM_ID                  ("MATAN_FAT")    // Must be longer than sizeof(boot.oem_id)
#define SECTOR_SIZE             (DEFAULT_SECTOR_SIZE)
#define SECTORS_PER_CLUSTER     (1)
#define RESERVED_SECTORS_COUNT  (1)
#define NUMBER_OF_FATS          (2)
#define NUMBER_OF_DIRENTS       (224)
#define SECTOR_COUNT            (2880)
#define MEDIA_TYPE              (0xf0)
#define FAT_SIZE_SECTORS        (9)
#define SECTORS_PER_TRACK       (18)
#define NUMBER_OF_HEADS         (2)
#define HIDDEN_SECTORS_COUNT    (0)
#define SECTOR_COUNT_LARGE      (0)

// FAT values
#define FIRST_FAT_SECTOR       (1)
#define FAT_SIZE               (9)  // 9 Sectors
#define RESERVED_SECTOR_VALUE   (0x00FFFFF0)

// Directory Entry Values
#define ROOT_DIRECTORY_SIZE    (14) // 14 sectors
#define FREE_DIRECTORY_ENTRY   (0xe5)

/**     Globals     ************************************/

int fid; /* global variable set by the open() function */


/**     Functions   ************************************/

int fd_write(int sector_number, char *buffer)
{
    int dest = 0, len = 0;
    dest = lseek(fid, sector_number * SECTOR_SIZE, SEEK_SET);
    if (dest != sector_number * SECTOR_SIZE)
    {
        /* Error handling */
        return -1;
    }
    len = write(fid, buffer, SECTOR_SIZE);
    if (len != SECTOR_SIZE)
    {
        /* error handling here */
        return -1;
    }
    return len;
}


int main(int argc, char *argv[])
{
	boot_record_t   boot                = {0};
    char            boot_jump[]         = BOOT_JMP;
    char            sector[SECTOR_SIZE] = {0};
    int             sector_index        = 0;
    int             sector_counter      = 0;
    int             fat_count           = 0;

	if (argc != 2)
	{
		printf("Usage: %s <floppy_image>\n", argv[0]);
		exit(1);
	}

	if ( (fid = open (argv[1], O_RDWR|O_CREAT, 0644))  < 0 )
	{
		perror("Error: ");
		return -1;
	}

    /** Creates the boot sector.                 *****************************************/
    // Sets bootjmp to 0x903ceb - copied from school solution image.
    memcpy(boot.bootjmp, boot_jump, sizeof(boot_jump));
    memcpy(boot.oem_id, OEM_ID, sizeof(boot.oem_id)); // Sets oem_id

    // Sets boot sector values according to example code...
    boot.sector_size = SECTOR_SIZE;
    boot.sectors_per_cluster = SECTORS_PER_CLUSTER;
    boot.reserved_sector_count = RESERVED_SECTORS_COUNT;
    boot.number_of_fats = NUMBER_OF_FATS;
    boot.number_of_dirents = NUMBER_OF_DIRENTS;
    boot.sector_count = SECTOR_COUNT;
    boot.media_type = MEDIA_TYPE;
    boot.fat_size_sectors = FAT_SIZE_SECTORS;
    boot.sectors_per_track = SECTORS_PER_TRACK;
    boot.nheads = NUMBER_OF_HEADS;
    boot.sectors_hidden = HIDDEN_SECTORS_COUNT;
    boot.sector_count_large = SECTOR_COUNT_LARGE;

    // Prints data about boot sector
    printf("sector_size: %d\n", boot.sector_size);
    printf("sectors_per_cluster: %d\n", boot.sectors_per_cluster);
    printf("reserved_sector_count: %d\n", boot.reserved_sector_count);
    printf("number_of_fats: %d\n", boot.number_of_fats);
    printf("number_of_dirents: %d\n", boot.number_of_dirents);
    printf("sector_count: %d\n", boot.sector_count);

    // Copies boot sector to the sector variable
    memcpy(sector, &boot, sizeof(boot));
    // Writes the boot sector to the image
    if (sizeof(sector) != fd_write(0, sector))
    {
        fprintf(stderr, "Failed to write boot sector.\n");
        close(fid);
        return -1;
    }

    /** Creates the File Allocation Tables.     *****************************************/
    memset(sector, 0, sizeof(sector)); // Sets sector to zero
    sector_counter = FIRST_FAT_SECTOR;
    // Creates all the file allocation tables.
    for (fat_count = 0; fat_count < boot.number_of_fats; fat_count++)
    {
        *((uint32_t *) sector) = RESERVED_SECTOR_VALUE; // Handles first 2 entries, and sets them to reserved value.
        // Creates all the sectors of the current file allocation table.
        for (sector_index = sector_counter; sector_index < sector_counter + FAT_SIZE; sector_index++)
        {
            if (sizeof(sector) != fd_write(sector_index, sector))
            {
                fprintf(stderr, "Failed to write FAT sector.\n");
                close(fid);
                return -1;
            }
            *((uint32_t *) sector) = 0; // Resets sector to zero after write of first entries
        }
        sector_counter = sector_index;
    }

    /** Creates the Root Directory.             *****************************************/
    memset(sector, 0, sizeof(sector)); // Sets sector to zero
    for (sector_index = sector_counter; sector_index < sector_counter + ROOT_DIRECTORY_SIZE; sector_index++)
    {
        if (sizeof(sector) != fd_write(sector_index, sector))
        {
            fprintf(stderr, "Failed to write root directory sector.\n");
            close(fid);
            return -1;
        }
    }

    /** Creates the rest of the directory entries.  *************************************/
    sector[0] = FREE_DIRECTORY_ENTRY;
    for(; sector_index < SECTOR_COUNT; sector_index++)
    {
        if (sizeof(sector) != fd_write(sector_index, sector))
        {
            fprintf(stderr, "Failed to write directory entry sector.\n");
            close(fid);
            return -1;
        }
    }
	
	close(fid);
	return 0;
}

