//****************************************************************************
// Flash memory IS25LP family Management on STM32H MPU
//
// file cW25Qxxx.cpp
// Copyright(c) 2025 Dad Design.
//****************************************************************************
#include "cIS25LPxxx.h"

namespace Dad {

//****************************************************************************
// Implementation of cW25Qxxx methods
//

// ------------------------------------------------------------------------
// Initializes the QSPI interface.
//
// Parameters:
//   phqspi - Pointer to the QSPI handle structure.
//
// Returns:
//   HAL_OK if initialization is successful, otherwise an error status.
// ------------------------------------------------------------------------
HAL_StatusTypeDef cIS25LPxxx::Init(QSPI_HandleTypeDef* phqspi, uint32_t MemoryAddress) {
    // Store the QSPI handle
    m_phqspi = phqspi;
    m_MemoryAddress = MemoryAddress;

    // ---------------------------------
    // Initialize command structures

    // General command structure for executing QSPI commands
    m_Command.Instruction = 0;
    m_Command.NbData = 0;
    m_Command.Address = 0;
    m_Command.AlternateBytes = 0;
    m_Command.AddressSize = 0;
    m_Command.AlternateBytesSize = 0;
    m_Command.DummyCycles = 0;
    m_Command.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    m_Command.AddressMode = QSPI_ADDRESS_NONE;
    m_Command.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    m_Command.DataMode = QSPI_DATA_NONE;
    m_Command.DdrMode = QSPI_DDR_MODE_DISABLE;
    m_Command.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    m_Command.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

    // Configure the command for writing data
    m_CommandQuadWrite.Address = 0;
    m_CommandQuadWrite.NbData = 0;
    m_CommandQuadWrite.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    m_CommandQuadWrite.Instruction = CMD_QUAD_PAGE_PROGRAM;
    m_CommandQuadWrite.AddressMode = QSPI_ADDRESS_1_LINE;
    m_CommandQuadWrite.AddressSize = QSPI_ADDRESS_24_BITS;
    m_CommandQuadWrite.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
    m_CommandQuadWrite.AlternateBytesSize = 0;
    m_CommandQuadWrite.AlternateBytes = 0;
    m_CommandQuadWrite.DummyCycles = 0;
    m_CommandQuadWrite.DdrMode = QSPI_DDR_MODE_DISABLE;
    m_CommandQuadWrite.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    m_CommandQuadWrite.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;
    m_CommandQuadWrite.DataMode = QSPI_DATA_4_LINES;

    // Configure the command for reading data
    m_CommandQuadRead.Address = 0;
    m_CommandQuadRead.NbData = 0;
    m_CommandQuadRead.InstructionMode = QSPI_INSTRUCTION_1_LINE;
    m_CommandQuadRead.Instruction = CMD_READ_FAST_QUAD_IO;
    m_CommandQuadRead.AddressMode = QSPI_ADDRESS_4_LINES;
    m_CommandQuadRead.AddressSize = QSPI_ADDRESS_24_BITS;
    m_CommandQuadRead.AlternateByteMode = QSPI_ALTERNATE_BYTES_4_LINES;
    m_CommandQuadRead.AlternateBytesSize = QSPI_ALTERNATE_BYTES_8_BITS;
    m_CommandQuadRead.AlternateBytes = 0x000000A0;
    m_CommandQuadRead.DummyCycles = 6;
    m_CommandQuadRead.DdrMode = QSPI_DDR_MODE_DISABLE;
    m_CommandQuadRead.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
    m_CommandQuadRead.SIOOMode = QSPI_SIOO_INST_ONLY_FIRST_CMD;
    m_CommandQuadRead.DataMode = QSPI_DATA_4_LINES;

    // ---------------------------------
    // Initialize Flash memory
    HAL_StatusTypeDef Result = HAL_OK;

    // Reset the flash memory device
    if (HAL_OK != (Result = Command(IS25CMD::CMD_SOFTWARE_RESET_EN))) {
        return Result;
    }
    if (HAL_OK != (Result = Command(IS25CMD::CMD_SOFTWARE_RESET))) {
        return Result;
    }
    HAL_Delay(10);

    // Retrieve device information
    uint8_t Data[3];
    if (HAL_OK != (Result = CommandRXData(IS25CMD::CMD_READ_JEDEC_ID, Data, 3))) {
        return Result;
    }
    m_ManufacturerID = Data[0];
    m_DeviceID = Data[1];
    m_Capacity = Data[2];

    // Configure Read Register
    IS25LP064A_ReadReg ReadReg;
    ReadReg.ods = 0b111;    // Driver strength: 50%
    ReadReg.dc = 0b10;      // 6 Dummy cycles and 2 Mode bit cycles
    ReadReg.wrap = 0;       // No wrap
    ReadReg.Brush = 0;

    // Enable write operation
    if (HAL_OK != (Result = Command(IS25CMD::CMD_WRITE_ENABLE))) {
        return Result;
    }
    if (HAL_OK != (Result = CommandTXData(IS25CMD::CMD_SET_READ_PARAMETERS, &(ReadReg.octet), 1))) {
        return Result;
    }

    // Activate Quad Mode
    IS25LP064A_StatusReg StatusReg;  // Status Register 1 of the flash memory

    if (HAL_OK != (Result = CommandRXData(IS25CMD::CMD_READ_STATUS, (uint8_t*)&StatusReg, 1))) {
        return Result;
    }

    // Enable write operation
    if (HAL_OK != (Result = Command(IS25CMD::CMD_WRITE_ENABLE))) {
        return Result;
    }

    // Set Quad mode enable bit
    StatusReg.qe = 1;

    if (HAL_OK != (Result = CommandTXData(IS25CMD::CMD_WRITE_STATUS, (uint8_t*)&StatusReg, 1))) {
        return Result;
    }

    WaitNoBusy();
    return MemoryMap();
}

// ------------------------------------------------------------------------
// Configures the QSPI memory in memory-mapped mode.
//
// In memory-mapped mode, the QSPI memory is directly accessible by the CPU,
// without requiring explicit read commands.
//
// Returns:
//   HAL status indicating success or failure of the configuration.
// ------------------------------------------------------------------------
inline HAL_StatusTypeDef cIS25LPxxx::MemoryMap() {
	HAL_StatusTypeDef Result;

    // Enable memory-mapped mode using the configured settings.
    Result = SwapModeMemoryMap();
    WaitNoBusy();
    return Result;
}

// ------------------------------------------------------------------------
// Performs a fast read operation from the QSPI memory.
// Parameters:
//  - pData: Pointer to the data buffer where the read data will be stored.
//  - Adresse: Starting address in memory from where the data will be read.
//  - Size: Number of bytes to read.
// Returns:
//  - HAL status
HAL_StatusTypeDef cIS25LPxxx::FastRead(uint8_t* pData, uint32_t Address, uint32_t NbData){
	HAL_StatusTypeDef Result;
	SwapModeIndirect();
	Result = ReadQuadData(Address, pData, NbData);
	MemoryMap();
	return Result;
}

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
HAL_StatusTypeDef cIS25LPxxx::FastWrite(uint8_t* pData, uint32_t Adresse, uint32_t NbData){
    HAL_StatusTypeDef Result;
    uint32_t Page_Adress;
    uint32_t Page_Size;

    SwapModeIndirect();

    // Calculate the size of the first page to write.
    Page_Adress = Adresse;
    Page_Size = 0x100 - (Page_Adress & 0xFF);  // Size of the first partial page (256 bytes).
    if (Page_Size > NbData) {
        Page_Size = NbData;  // Adjust size if less than the page size.
    }

    while (NbData > 0) {
        // Enable write operations on the flash memory.
        if (HAL_OK != (Result = Command(IS25CMD::CMD_WRITE_ENABLE))) {
            return Result;  // Return if enabling write failed.
        }

        // Write the current page of data to memory.
        if (HAL_OK != (Result = WriteQuadData(Page_Adress, pData, Page_Size))){
            return Result;  // Return if writing data failed.
        }

        // Wait until the write operation is complete.
        if (HAL_OK != (Result = WaitNoBusy())) {
            return Result;  // Return if waiting for the operation to complete failed.
        }
       // Move to the next chunk of data to write.
        pData += Page_Size;
        Page_Adress += Page_Size;
        NbData -= Page_Size;

        // Calculate the size of the next page to write.
        if (NbData >= 0x100) {
            Page_Size = 0x100;  // Full page size (256 bytes) if more data remains.
        } else {
            Page_Size = NbData;   // Remaining data size if less than a full page.
        }
    }

    // Switch back to direct mode.
    MemoryMap();
    return Result;  // Return the final status of the write operation.
}

// ------------------------------------------------------------------------
// Erases a sector (4K) of the QSPI memory.
// Parameters:
//  - Adresse: Address in the sector to erase.
// Returns:
//  - HAL status
HAL_StatusTypeDef cIS25LPxxx::EraseSector(uint32_t Adresse){
	HAL_StatusTypeDef Result;
    SwapModeIndirect();

	// Write enable
	if(HAL_OK != (Result = Command(IS25CMD::CMD_WRITE_ENABLE))){
		return Result;
	}

	// Erase sector
	uint32_t DataAdresse = (Adresse - m_MemoryAddress) & 0x00FFF000;
	if(HAL_OK != (Result = CommandTXData(IS25CMD::CMD_SECTOR_ERASE, (uint8_t*)&DataAdresse, 3))){
		return Result;
	}

	// Wait end of erase
	if(HAL_OK != (Result = WaitNoBusy())){
		return Result;
	}
	MemoryMap();
	return Result;
}

// ------------------------------------------------------------------------
// Erases a 32 KB block of the QSPI memory.
// Parameters:
//  - Adresse: Address in the 32 KB block to erase.
// Returns:
//  - HAL status
HAL_StatusTypeDef cIS25LPxxx::EraseBlock32(uint32_t Adresse){
	HAL_StatusTypeDef Result;
    SwapModeIndirect();

	// Write enable
	if(HAL_OK != (Result = Command(IS25CMD::CMD_WRITE_ENABLE))){
		return Result;
	}

	// Erase sector
	uint32_t DataAdresse = (Adresse - m_MemoryAddress) & 0x00FF8000;
	if(HAL_OK != (Result = CommandTXData(IS25CMD::CMD_BLOCK_ERASE_32K, (uint8_t*)&DataAdresse, 3))){
		return Result;
	}

	// Wait end of erase
	if(HAL_OK != (Result = WaitNoBusy())){
		return Result;
	}
	MemoryMap();
	return Result;
}

// ------------------------------------------------------------------------
// Erases a 64 KB block of the QSPI memory.
// Parameters:
//  - Adresse: Address in 64 KB block to erase.
// Returns:
//  - HAL status
HAL_StatusTypeDef cIS25LPxxx::EraseBlock64(uint32_t Adresse){
	HAL_StatusTypeDef Result;

    SwapModeIndirect();

	// Write enable
	if(HAL_OK != (Result = Command(IS25CMD::CMD_WRITE_ENABLE))){
		return Result;
	}

	// Erase sector
	uint32_t DataAdresse = (Adresse - m_MemoryAddress) & 0x00FF0000;
	if(HAL_OK != (Result = CommandTXData(IS25CMD::CMD_BLOCK_ERASE_64K, (uint8_t*) &DataAdresse, 3))){
		return Result;
	}

	// Wait end of erase
	if(HAL_OK != (Result = WaitNoBusy())){
		return Result;
	}
	MemoryMap();
	return Result;
}

// ------------------------------------------------------------------------
// Erase all the Chip
HAL_StatusTypeDef cIS25LPxxx::EraseChip(){
	HAL_StatusTypeDef Result;

    SwapModeIndirect();

	// Write enable
	if(HAL_OK != (Result = Command(IS25CMD::CMD_WRITE_ENABLE))){
		return Result;
	}

	// Erase sector
	if(HAL_OK != (Result = Command(IS25CMD::CMD_CHIP_ERASE))){
		return Result;
	}

	// Wait end of erase
	if(HAL_OK != (Result = WaitNoBusy())){
		return Result;
	}
	MemoryMap();
	return Result;
}

// ------------------------------------------------------------------------
// Waits until the flash memory is not busy.
// This method continuously checks the status register until the flash memory
// is no longer busy, indicating that the previous operation has completed.
// Returns:
//  - HAL status
HAL_StatusTypeDef cIS25LPxxx::WaitNoBusy() {
	HAL_StatusTypeDef Result;
	while(1) {
		IS25LP064A_StatusReg StatusReg;
		if(HAL_OK != (Result = CommandRXData(IS25CMD::CMD_READ_STATUS, (uint8_t*) &StatusReg, 1))) {
			return Result;
		} else if(StatusReg.wip == 0) {
			return Result;
		}
		HAL_Delay(1);
	}
}

} /* namespace Dad */
