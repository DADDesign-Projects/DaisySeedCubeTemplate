#pragma once
//****************************************************************************
// QSPI Interface Hardware Management on STM32H MPU
//
// file cHwQSPI.h
//
// Copyright(c) 2025 Dad Design.
//****************************************************************************
#include "main.h"

namespace Dad {

//****************************************************************************
// QSPI Interface Management
//
class cHwQSPI {
public:
    // ------------------------------------------------------------------------
    // Constructor & Destructor
    cHwQSPI() {};
    virtual ~cHwQSPI() {};

protected:
    // ------------------------------------------------------------------------
    // Sends a QSPI command with no data phase.
    //
    // Parameters:
    //   Instruction - The command instruction to be sent.
    //
    // Returns:
    //   HAL_OK if successful, HAL_ERROR if the command fails.
    // ------------------------------------------------------------------------
    inline HAL_StatusTypeDef Command(uint32_t Instruction) {
        // Set the command instruction
        m_Command.Instruction = Instruction;

        // Set data length to 0 (no data phase)
        m_Command.NbData = 0;
        m_Command.DataMode = QSPI_DATA_NONE;

        // Send the command
        return HAL_QSPI_Command(m_phqspi, &m_Command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
    }

    // ------------------------------------------------------------------------
    // Sends a QSPI command with address phase.
    //
    // Parameters:
    //   Instruction - The command instruction to be sent.
    //   Address - The target address of the command.
    //
    // Returns:
    //   HAL_OK if successful, HAL_ERROR if the command fails.
    // ------------------------------------------------------------------------
    inline HAL_StatusTypeDef CommandAddress(uint32_t Instruction, uint32_t Address) {
        // Set the command instruction
        m_CommandAddress.Instruction = Instruction;

        // Set data length to 0 (no data phase)
        m_CommandAddress.NbData = 0;
        m_CommandAddress.DataMode = QSPI_DATA_NONE;
        m_CommandAddress.Address=Address;

        // Send the command
        return HAL_QSPI_Command(m_phqspi, &m_CommandAddress, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
    }


    // ------------------------------------------------------------------------
    // Sends a QSPI command with a data transmission phase.
    //
    // Parameters:
    //   Instruction - The command instruction to be sent.
    //   pData       - Pointer to the data buffer to be transmitted.
    //   NbData      - Number of bytes to transmit.
    //
    // Returns:
    //   HAL_OK if successful, HAL_ERROR if the command fails.
    // ------------------------------------------------------------------------
    inline HAL_StatusTypeDef CommandTXData(uint32_t Instruction, uint8_t* pData, uint32_t NbData) {
        // Set the command instruction and the number of bytes to send
        m_Command.Instruction = Instruction;
        m_Command.NbData = NbData;
        m_Command.DataMode = QSPI_DATA_1_LINE;

        // Send the command
        if (HAL_QSPI_Command(m_phqspi, &m_Command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            return HAL_ERROR;
        }

        // Transmit the data
        return HAL_QSPI_Transmit(m_phqspi, pData, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
    }

    // ------------------------------------------------------------------------
    // Sends a QSPI command and receives data.
    //
    // Parameters:
    //   Instruction - The command instruction to be sent.
    //   pData       - Pointer to the buffer where received data will be stored.
    //   NbData      - Number of bytes to receive.
    //
    // Returns:
    //   HAL_OK if successful, HAL_ERROR if the command fails.
    // ------------------------------------------------------------------------
    HAL_StatusTypeDef CommandRXData(uint32_t Instruction, uint8_t* pData, uint32_t NbData) {
        // Set the command instruction and the number of bytes to receive
        m_Command.Instruction = Instruction;
        m_Command.NbData = NbData;
        m_Command.DataMode = QSPI_DATA_1_LINE;

        // Send the command
        if (HAL_QSPI_Command(m_phqspi, &m_Command, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            return HAL_ERROR;
        }

        // Receive the data
        return HAL_QSPI_Receive(m_phqspi, pData, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
    }

    // ------------------------------------------------------------------------
    // Writes data to the QSPI memory using Quad-SPI (QSPI) in indirect mode.
    //
    // Parameters:
    //   Address - The target address in the QSPI memory where data will be written.
    //   pData   - Pointer to the buffer containing the data to be written.
    //   NbData  - Number of bytes to write.
    //
    // Returns:
    //   HAL_OK if successful, HAL_ERROR if the command fails.
    // ------------------------------------------------------------------------
    HAL_StatusTypeDef WriteQuadData(uint32_t Address, uint8_t* pData, uint32_t NbData) {

        // Set the address and the number of bytes to write in the write command structure.
        m_CommandQuadWrite.Address = Address - m_MemoryAddress;
        m_CommandQuadWrite.NbData = NbData;

        // Send the write command to the QSPI memory.
        if (HAL_QSPI_Command(m_phqspi, &m_CommandQuadWrite, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            return HAL_ERROR;  // Return error if the command fails.
        }

        // Transmit the data to the QSPI memory.
        return HAL_QSPI_Transmit(m_phqspi, pData, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);

    }

    // ------------------------------------------------------------------------
    // Reads data using Quad-SPI (QSPI) in indirect mode.
    //
    // Parameters:
    //   Address - The starting address in the QSPI memory to read from.
    //   pData   - Pointer to the buffer where the received data will be stored.
    //   NbData  - Number of bytes to read.
    //
    // Returns:
    //   HAL_OK if successful, HAL_ERROR if the command fails.
    // ------------------------------------------------------------------------
    HAL_StatusTypeDef ReadQuadData(uint32_t Address, uint8_t* pData, uint32_t NbData) {

    	// Set the address and the number of bytes to read in the read command structure.
    	m_CommandQuadRead.Address = Address - m_MemoryAddress;
        m_CommandQuadRead.NbData = NbData;

        // Send the QSPI read command.

        if (HAL_QSPI_Command(m_phqspi, &m_CommandQuadRead, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK) {
            return HAL_ERROR;  // Return error if the command fails.
        }

        // Receive the data from the QSPI memory.
        return HAL_QSPI_Receive(m_phqspi, pData, HAL_QSPI_TIMEOUT_DEFAULT_VALUE);
    }


    // ------------------------------------------------------------------------
    // Configures the QSPI memory in memory-mapped mode. (Direct Mode)
    //
    // In memory-mapped mode, the QSPI memory is directly accessible by the CPU,
    // without requiring explicit read commands.
    //
    // Returns:
    //   HAL_OK if successful, HAL_ERROR if the command fails.
    // ------------------------------------------------------------------------
    inline HAL_StatusTypeDef SwapModeMemoryMap() {
    	QSPI_MemoryMappedTypeDef cfg;

   		// Set default address and data length for memory-mapped mode.
        m_CommandQuadRead.Address = 0;
        m_CommandQuadRead.NbData = 0;

        // Disable timeout counter since memory-mapped mode does not require it.
        cfg.TimeOutActivation = QSPI_TIMEOUT_COUNTER_DISABLE;
        cfg.TimeOutPeriod = 0;

        // Enable memory-mapped mode using the configured settings.
        return HAL_QSPI_MemoryMapped(m_phqspi, &m_CommandQuadRead, &cfg);
    }

    // ------------------------------------------------------------------------
    // Function to set the memory access mode to Indirect Mode
    //
    // Returns:
    //   HAL_OK if successful, HAL_ERROR if the command fails.
    //
    inline HAL_StatusTypeDef setIndirectMode(){

        return HAL_QSPI_Abort(m_phqspi);      		// Abort any ongoing QSPI operation
     }

protected:

    // =======================================================================
    // Protected Data

    QSPI_HandleTypeDef* m_phqspi = nullptr;   	// Pointer to the QSPI interface handle, used for all QSPI operations.
    uint32_t			m_MemoryAddress;		// Address of memory


    QSPI_CommandTypeDef m_Command;            	// Command structure for executing simple QSPI commands without address or data.
    QSPI_CommandTypeDef m_CommandAddress;      	// Command structure for executing simple QSPI commands with address.

    QSPI_CommandTypeDef m_CommandQuadWrite;   	// Command structure for write data.
    QSPI_CommandTypeDef m_CommandQuadRead;    	// Command structure for read data.

};

} /* namespace Dad */

