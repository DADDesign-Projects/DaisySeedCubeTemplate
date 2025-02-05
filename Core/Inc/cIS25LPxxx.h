#pragma once
//****************************************************************************
// Flash memory IS25LP family Management on STM32H MPU
//
// file cW25Qxxx.cpp
// Copyright(c) 2025 Dad Design.
//****************************************************************************
#include "main.h"
#include "cHwQSPI.h"

namespace Dad {

enum IS25LP064A_Commands {
    CMD_WRITE_ENABLE        = 0x06, // Write Enable (WREN)
    CMD_WRITE_DISABLE       = 0x04, // Write Disable (WRDI)
    CMD_READ_STATUS         = 0x05, // Read Status Register (RDSR)
    CMD_WRITE_STATUS        = 0x01, // Write Status Register (WRSR)
    CMD_READ_FUNCTION       = 0x48, // Read Function Register (RDFR)
    CMD_WRITE_FUNCTION      = 0x42, // Write Function Register (WRFR)
    CMD_READ_ID             = 0xAB, // Read ID / Release Power Down (RDID, RDPD)
    CMD_READ_JEDEC_ID       = 0x9F, // Read JEDEC ID (RDJDID)
	CMD_READ_JEDEC_ID_QPI 	= 0xAF, // Read JEDEC ID in QPI mode (RDJDIDQ)
    CMD_READ_MANUF_ID       = 0x90, // Read Manufacturer & Device ID (RDMDID)
    CMD_READ_UNIQUE_ID      = 0x4B, // Read Unique ID (RDUID)
    CMD_READ_SFDP           = 0x5A, // Read SFDP (RDSFDP)
    CMD_ENTER_QPI_MODE      = 0x35, // Enter Quad Peripheral Interface mode (QPIEN)
    CMD_EXIT_QPI_MODE       = 0xF5, // Exit QPI mode (QPIDI)

    // Read commands
    CMD_READ_NORMAL         = 0x03, // Normal Read (NORD)
    CMD_READ_FAST           = 0x0B, // Fast Read (FRD)
    CMD_READ_FAST_DUAL_IO   = 0xBB, // Fast Read Dual I/O (FRDIO)
    CMD_READ_FAST_DUAL_OUT  = 0x3B, // Fast Read Dual Output (FRDO)
    CMD_READ_FAST_QUAD_IO   = 0xEB, // Fast Read Quad I/O (FRQIO)
    CMD_READ_FAST_QUAD_OUT  = 0x6B, // Fast Read Quad Output (FRQO)
    CMD_FAST_READ_DTR        = 0x0D, // Fast Read DTR Mode (FRDTR)
    CMD_FAST_READ_DUAL_IO_DTR = 0xBD, // Fast Read Dual I/O DTR Mode (FRDDTR)
    CMD_FAST_READ_QUAD_IO_DTR = 0xED, // Fast Read Quad I/O DTR Mode (FRQDTR)
    CMD_SET_READ_PARAMETERS  = 0xC0,  // Set Read Parameters (SRP)

    // Program commands
    CMD_PAGE_PROGRAM        = 0x02, // Page Program (PP)
    CMD_QUAD_PAGE_PROGRAM   = 0x32, // Quad Input Page Program (PPQ)

    // Erase commands
    CMD_SECTOR_ERASE        = 0x20, // Sector Erase (SER)
    CMD_BLOCK_ERASE_32K     = 0x52, // Block Erase 32KB (BER32)
    CMD_BLOCK_ERASE_64K     = 0xD8, // Block Erase 64KB (BER64)
    CMD_CHIP_ERASE          = 0xC7, // Chip Erase (CER)

    // Suspend & Resume
    CMD_SUSPEND_PROGRAM     = 0x75, // Suspend during Program/Erase (PERSUS)
    CMD_RESUME_PROGRAM      = 0x7A, // Resume Program/Erase (PERRSM)

    // Power Management
    CMD_DEEP_POWER_DOWN     = 0xB9, // Enter Deep Power Down (DP)
    CMD_RELEASE_POWER_DOWN  = 0xAB, // Release Deep Power Down (RDPD)

    // Reset
    CMD_SOFTWARE_RESET_EN   = 0x66, // Enable Software Reset (RSTEN)
    CMD_SOFTWARE_RESET      = 0x99, // Software Reset (RST)

    // Security & Protection
    CMD_LOCK_SECTOR         = 0x24, // Sector Lock (SECLOCK)
    CMD_UNLOCK_SECTOR       = 0x26, // Sector Unlock (SECUNLOCK)
    CMD_ERASE_INFO_ROW      = 0x64, // Erase Information Row (IRER)
    CMD_PROGRAM_INFO_ROW    = 0x62, // Program Information Row (IRP)
    CMD_READ_INFO_ROW       = 0x68, // Read Information Row (IRRD)

    // No operation
    CMD_NOP                 = 0x00  // No Operation (NOP)
};

#define IS25CMD (uint32_t)IS25LP064A_Commands

// Structure du Status Register pour IS25LP064A
union IS25LP064A_StatusReg {
	struct{
		uint8_t wip   : 1; // Bit 0: Write In Progress (1 si en écriture, 0 si prêt)
		uint8_t wel   : 1; // Bit 1: Write Enable Latch (1 si activé)
		uint8_t bp0   : 1; // Bit 2: Block Protection Bit 0
		uint8_t bp1   : 1; // Bit 3: Block Protection Bit 1
		uint8_t bp2   : 1; // Bit 4: Block Protection Bit 2
		uint8_t bp3   : 1; // Bit 5: Block Protection Bit 3
		uint8_t qe    : 1; // Bit 6: Quad Enable (1 si mode Quad activé)
		uint8_t srwd  : 1; // Bit 7: Status Register Write Disable
	};
	uint8_t octet;
};

// Structure du Function Register pour IS25LP064A
union IS25LP064A_FunctionReg{
	struct  {
		uint8_t reserved : 1; // Reserved
		uint8_t tbs      : 1; // Top/Bottom Selection (0 = Top, 1 = Bottom)
		uint8_t psus     : 1; // Program Suspend Status
		uint8_t esus     : 1; // Erase Suspend Status
		uint8_t irl      : 4; // Information Row Lock Bit 0
	};
	uint8_t octet;
};

// Structure du Read Register pour IS25LP064A
union IS25LP064A_ReadReg{
	struct {
		uint8_t Brush : 2; // Burst Length
		uint8_t wrap  : 1; // Wrap Enable
		uint8_t dc    : 2; // Dummy Cycle Bit
		uint8_t ods   : 3; // Output Driver Strength

	};
	uint8_t	octet;
};


//****************************************************************************
// cIS25LPxxx
//
class cIS25LPxxx : public cHwQSPI{
public:
	// ------------------------------------------------------------------------
    // Constructor & Destructor
	cIS25LPxxx(){};
	virtual ~cIS25LPxxx(){};

	// ------------------------------------------------------------------------
	// Initializes the QSPI interface.
	//
	// Parameters:
	//   phqspi - Pointer to the QSPI handle structure.
	//
	// Returns:
	//   HAL_OK if initialization is successful, otherwise an error status.
	// ------------------------------------------------------------------------
	HAL_StatusTypeDef Init(QSPI_HandleTypeDef* phqspi, uint32_t MemoryAddress = 0x90000000);

	// ------------------------------------------------------------------------
	// Configures the QSPI memory in memory-mapped mode.
	//
	// In memory-mapped mode, the QSPI memory is directly accessible by the CPU,
	// without requiring explicit read commands.
	//
	// Returns:
	//   HAL status indicating success or failure of the configuration.
	// ------------------------------------------------------------------------
	HAL_StatusTypeDef MemoryMap();

	// ------------------------------------------------------------------------
	// Performs a fast read operation from the QSPI memory.
	// Parameters:
	//  - pData: Pointer to the data buffer where the read data will be stored.
	//  - Adresse: Starting address in memory from where the data will be read.
	//  - Size: Number of bytes to read.
	// Returns:
	//  - HAL status
	HAL_StatusTypeDef FastRead(uint8_t* pData, uint32_t Address, uint32_t NbData);

	// ------------------------------------------------------------------------
	// Performs a fast write operation to the QSPI memory.
	// This method writes data to the QSPI memory in pages, handling page boundaries
	// and ensuring the write operation completes before proceeding to the next page.
	// Parameters:
	//  - pData: Pointer to the data buffer containing the data to be written.
	//  - Adresse: Starting address in memory where the data will be written.
	//  - NbData: Number of bytes to write.
	// Returns:
	//  - HAL status indicating success or failure of the operation.
	HAL_StatusTypeDef FastWrite(uint8_t* pData, uint32_t Adresse, uint32_t NbData);

	// ------------------------------------------------------------------------
	// Erases a sector (4K) of the QSPI memory.
	// Parameters:
	//  - Adresse: Address in the sector to erase.
	// Returns:
	//  - HAL status
	HAL_StatusTypeDef EraseSector(uint32_t Adresse);

	// ------------------------------------------------------------------------
	// Erases a 32 KB block of the QSPI memory.
	// Parameters:
	//  - Adresse: Address in the 32 KB block to erase.
	// Returns:
	//  - HAL status
	HAL_StatusTypeDef EraseBlock32(uint32_t Adresse);

	// ------------------------------------------------------------------------
	// Erases a 64 KB block of the QSPI memory.
	// Parameters:
	//  - Adresse: Address in 64 KB block to erase.
	// Returns:
	//  - HAL status
	HAL_StatusTypeDef EraseBlock64(uint32_t Adresse);

	// ------------------------------------------------------------------------
	// Erase all the Chip
	HAL_StatusTypeDef EraseChip();

protected:
	// =======================================================================
	// Protected Methods

	// ------------------------------------------------------------------------
	// Waits until the flash memory is not busy.
	// This method continuously checks the status register until the flash memory
	// is no longer busy, indicating that the previous operation has completed.
	// Returns:
	//  - HAL status
	HAL_StatusTypeDef WaitNoBusy();

	// =======================================================================
	// Protected Data

	uint8_t m_ManufacturerID = 0;              // Manufacturer ID of the flash memory
	uint8_t m_DeviceID = 0;                    // Device ID of the flash memory
	uint8_t m_Capacity = 0;                    // Flash memory size, represented as 2^Capacity
};

} /* namespace Dad */
