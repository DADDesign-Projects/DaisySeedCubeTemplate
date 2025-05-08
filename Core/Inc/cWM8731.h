#pragma once
//****************************************************************************
// CODEC WM8731 Management
//
// file cWM8732.h
// Copyright(c) 2025 Dad Design.
//****************************************************************************
#include "main.h"

namespace Dad {
#define WM8731_ADR 0x1A

// LEFT LINE IN REGISTER (0x00)
union LeftLineIn {
    struct {
        uint16_t linvol   : 5;
        uint16_t reserved : 2;
        uint16_t linmute  : 1;
        uint16_t lrinboth : 1;
    } bits;
    uint16_t raw;

    static constexpr uint8_t address = 0x00;
};

// RIGHT LINE IN REGISTER (0x01)
union RightLineIn {
    struct {
        uint16_t rinvol   : 5;
        uint16_t reserved : 2;
        uint16_t rinmute  : 1;
        uint16_t rrinboth : 1;
    } bits;
    uint16_t raw;

    static constexpr uint8_t address = 0x01;
};

// LEFT HEADPHONE OUT REGISTER (0x02)
union LeftHeadphoneOut {
    struct {
        uint16_t lhpvol    : 7;
        uint16_t lzcen	   : 1;
        uint16_t lrhpboth  : 1;
    } bits;
    uint16_t raw;

    static constexpr uint8_t address = 0x02;
};

// RIGHT HEADPHONE OUT REGISTER (0x03)
union RightHeadphoneOut {
    struct {
        uint16_t rhvol     : 7;
        uint16_t rzcen     : 1;
        uint16_t rlhpboth  : 1;
    } bits;
    uint16_t raw;

    static constexpr uint8_t address = 0x03;
};

// ANALOG AUDIO PATH CONTROL REGISTER (0x04)
union AnalogAudioPath {
    struct {
        uint16_t micBoost  : 1;
        uint16_t micMute   : 1;
        uint16_t inSel     : 1;
        uint16_t bypass    : 1;
        uint16_t dacSel    : 1;
        uint16_t sidetone  : 1;
        uint16_t sideAtten : 2;
    } bits;
    uint16_t raw;

    static constexpr uint8_t address = 0x04;
};

// DIGITAL AUDIO PATH CONTROL REGISTER (0x05)
union DigitalAudioPath {
    struct {
        uint16_t adcHighPass : 1;
        uint16_t deEmphasis  : 2;
        uint16_t dacMute     : 1;
        uint16_t hpor		 : 1;
        uint16_t reserved    : 4;
    } bits;
    uint16_t raw;

    static constexpr uint8_t address = 0x05;
};

// POWER DOWN CONTROL REGISTER (0x06)
union PowerDownControl {
    struct {
        uint16_t lineInPD  : 1;
        uint16_t micPD     : 1;
        uint16_t adcPD     : 1;
        uint16_t dacPD     : 1;
        uint16_t outPD     : 1;
        uint16_t oscPD     : 1;
        uint16_t clkOutPD  : 1;
        uint16_t powerOff  : 1;
    } bits;
    uint16_t raw;

    static constexpr uint8_t address = 0x06;
};

// DIGITAL AUDIO INTERFACE FORMAT REGISTER (0x07)
union DigitalAudioInterfaceFormat {
    struct {
        uint16_t format   : 2; // 00=Right Justified, 01=Left Justified, etc.
        uint16_t iwl      : 2; // 00=16bit, 01=20bit, 10=24bit, 11=32bit
        uint16_t lrp      : 1;
        uint16_t lrswap   : 1;
        uint16_t master   : 1;
        uint16_t bclkinv  : 1; //Bit Clock Invert  1 = Invert BCLK  0 = Don’t invert BCLK
    } bits;
    uint16_t raw;

    static constexpr uint8_t address = 0x07;
};

// SAMPLE RATE CONTROL REGISTER (0x08)
union SampleRateControl {
    struct {
        uint16_t usbNormal : 1;
        uint16_t bosr      : 1;
        uint16_t sr        : 4;
        uint16_t clkidiv2  : 1;
        uint16_t clkodiv2  : 1;
    } bits;
    uint16_t raw;

    static constexpr uint8_t address = 0x08;
};

// DIGITAL INTERFACE ACTIVATION REGISTER (0x09)
union DigitalInterfaceActivation {
    struct {
        uint16_t activate : 1;
        uint16_t reserved : 7;
    } bits;
    uint16_t raw;

    static constexpr uint8_t address = 0x09;
};

// RESET REGISTER (0x0A)
union ResetRegister {
    struct {
        uint16_t reset : 9; // Any value resets
    } bits;
    uint16_t raw;

    static constexpr uint8_t address = 0x0F;
};

//****************************************************************************
// cWM8731
//
class cWM8731 {
public:
	cWM8731(){};
	HAL_StatusTypeDef Initialize(I2C_HandleTypeDef *phi2c){
		m_phi2c = phi2c;
		HAL_StatusTypeDef Result;

		// Reset
		m_ResetRegister.raw = 0;
	    if( HAL_OK != (Result = WriteReg(m_ResetRegister.address, m_ResetRegister.raw))){
	    	return Result;
	    }

		// Set Line Inputs to 0DB
		m_LeftLineIn.raw = 0;
		m_LeftLineIn.bits.linvol = 0x17;
	    if( HAL_OK != (Result = WriteReg(m_LeftLineIn.address, m_LeftLineIn.raw))){
	    	return Result;
	    }
		m_RightLineIn.raw = 0;
		m_RightLineIn.bits.rinvol = 0x17;
	    if( HAL_OK != (Result = WriteReg(m_RightLineIn.address, m_RightLineIn.raw))){
	    	return Result;
	    }

	    // Set Headphone To Mute
	    m_LeftHeadphoneOut.raw = 0;
	    if( HAL_OK != (Result = WriteReg(m_LeftHeadphoneOut.address, m_LeftHeadphoneOut.raw))){
	    	return Result;
	    }
	    m_RightHeadphoneOut.raw = 0;
	    if( HAL_OK != (Result = WriteReg(m_RightHeadphoneOut.address, m_RightHeadphoneOut.raw))){
	    	return Result;
	    }

	    // Analog and Digital Routing
	    m_AnalogAudioPath.raw = 0;
	    m_AnalogAudioPath.bits.micMute = 1;
	    m_AnalogAudioPath.bits.dacSel = 1;
	    if( HAL_OK != (Result = WriteReg(m_AnalogAudioPath.address, m_AnalogAudioPath.raw))){
	    	return Result;
	    }
	    m_DigitalAudioPath.raw = 0;
	    if( HAL_OK != (Result = WriteReg(m_DigitalAudioPath.address, m_DigitalAudioPath.raw))){
	    	return Result;
	    }

	    // Configure power management
	    m_PowerDownControl.raw = 0;
	    m_PowerDownControl.bits.micPD = 1;
		m_PowerDownControl.bits.oscPD = 1;
		m_PowerDownControl.bits.clkOutPD = 1;
	    if( HAL_OK != (Result = WriteReg(m_PowerDownControl.address, m_PowerDownControl.raw))){
	    	return Result;
	    }

	    // Digital Format
	    m_DigitalAudioInterfaceFormat.raw=0;
	    m_DigitalAudioInterfaceFormat.bits.format = 0b01; 	//11 = DSP Mode, frame sync + 2 data packed words
	    													//10 = I2S Format, MSB-First left-1 justified
	    													//01 = MSB-First, left justified
	    													//00 = MSB-First, right justified
	    m_DigitalAudioInterfaceFormat.bits.iwl = 0b10;		//11 = 32 bits
	    													//10 = 24 bits
	    													//01 = 20 bits
	    													//00 = 16 bits
	    m_DigitalAudioInterfaceFormat.bits.lrp = 0;
	    m_DigitalAudioInterfaceFormat.bits.lrswap = 1;      // yes
	    m_DigitalAudioInterfaceFormat.bits.master = 0;		// Slave
	    m_DigitalAudioInterfaceFormat.bits.bclkinv = 0;		// no
	    if( HAL_OK != (Result = WriteReg(m_DigitalAudioInterfaceFormat.address, m_DigitalAudioInterfaceFormat.raw))){
	    	return Result;
	    }

	    // Sample rate
	    m_SampleRateControl.raw = 0;
	    if( HAL_OK != (Result = WriteReg(m_SampleRateControl.address, m_SampleRateControl.raw))){
	    	return Result;
	    }

	    // Enable
	    m_DigitalInterfaceActivation.raw = 0;
	    if( HAL_OK != (Result = WriteReg(m_DigitalInterfaceActivation.address, m_DigitalInterfaceActivation.raw))){
	    	return Result;
	    }
	    m_DigitalInterfaceActivation.bits.activate = 1;
	    if( HAL_OK != (Result = WriteReg(m_DigitalInterfaceActivation.address, m_DigitalInterfaceActivation.raw))){
	    	return Result;
	    }

	    return Result;
	}

protected:
	HAL_StatusTypeDef WriteReg(uint8_t Reg, uint16_t Data){
		HAL_StatusTypeDef Result;

	    uint8_t Byte_1  = ((Reg << 1) & 0xfe) | ((Data >> 8) & 0x01);
	    uint8_t Byte_2  = Data & 0xff;
	    uint8_t Buff[2] = {Byte_1, Byte_2};

	    // Wait for previous transfer to be finished
	    while(HAL_I2C_GetState(m_phi2c) != HAL_I2C_STATE_READY) {};

	    // Write register
	    Result =  HAL_I2C_Master_Transmit(m_phi2c, WM8731_ADR << 1, Buff, 2, 500);
	    HAL_Delay(10);
	    return Result;
	}

	I2C_HandleTypeDef*			m_phi2c;
	LeftLineIn					m_LeftLineIn;
	RightLineIn					m_RightLineIn;
	LeftHeadphoneOut			m_LeftHeadphoneOut;
	RightHeadphoneOut			m_RightHeadphoneOut;
	AnalogAudioPath				m_AnalogAudioPath;
	DigitalAudioPath			m_DigitalAudioPath;
	PowerDownControl			m_PowerDownControl;
	DigitalAudioInterfaceFormat	m_DigitalAudioInterfaceFormat;
	SampleRateControl			m_SampleRateControl;
	DigitalInterfaceActivation  m_DigitalInterfaceActivation;
	ResetRegister				m_ResetRegister;
};
}//Dad
