//****************************************************************************
// Audio Hardware Management on STM32H MPU
//
// file Audio.cpp
//
// Copyright(c) 2025 Dad Design.
//****************************************************************************
#include "main.h"

extern SAI_HandleTypeDef hsai_BlockA1;
extern SAI_HandleTypeDef hsai_BlockB1;

#define SAI_HALF_BUFFER_SIZE  (AUDIO_BUFFER_SIZE * 2) // Stereo
#define SAI_BUFFER_SIZE 	  (AUDIO_BUFFER_SIZE * 4)

extern enum HardRev{
	Rev5,
	Rev7
} __HardRev;

// ------------------------------------------------------------------------
// AudioCallback
// ------------------------------------------------------------------------
extern void AudioCallback(AudioBuffer *pIn, AudioBuffer *pOut);

// Audio Buffer
NO_CACHE_RAM AudioBuffer In[AUDIO_BUFFER_SIZE];
NO_CACHE_RAM AudioBuffer Out1[AUDIO_BUFFER_SIZE];
NO_CACHE_RAM AudioBuffer Out2[AUDIO_BUFFER_SIZE];

NO_CACHE_RAM AudioBuffer* pOut;
NO_CACHE_RAM int32_t rxBuffer[SAI_BUFFER_SIZE];
NO_CACHE_RAM int32_t txBuffer[SAI_BUFFER_SIZE];

// ------------------------------------------------------------------------
// Convert int32_t buffer to float AudioBuffer
// ------------------------------------------------------------------------
inline float int32ToFloat(int32_t sample) {
    // Check if the sign bit (bit 23) is set
    if (sample & 0x00800000) {
        // Extend the sign bit to 32-bit (two’s complement format)
        sample |= 0xFF000000;
    } else {
        // Ensure only the lower 24 bits are used
        sample &= 0x00FFFFFF;
    }
    // Normalize to the range [-1.0, 1.0] by dividing by 2^23
    return static_cast<float>(sample) / 8388608.0f;
}

ITCM void ConvertToAudioBuffer( int32_t* intBuf, AudioBuffer* floatBuf) {
    for (size_t i = 0; i < AUDIO_BUFFER_SIZE; i++) {
        floatBuf[i].Left = int32ToFloat(intBuf[i * 2]);
        floatBuf[i].Right = int32ToFloat(intBuf[i * 2 + 1]);
    }
}

// ------------------------------------------------------------------------
// Convert float AudioBuffer to int32_t buffer
// ------------------------------------------------------------------------
inline int32_t floatToInt32(float sample) {
    // Clamp the value to the valid range [-1.0, 1.0]
    if (sample > 1.0f) sample = 1.0f;
    if (sample < -1.0f) sample = -1.0f;

    // Convert to a signed 24-bit integer (scale by 2^23)
    int32_t intSample = static_cast<int32_t>(sample * 8388608.0f);

    // Ensure the result is in a valid 24-bit format
    return intSample & 0xFFFFFF;
}

ITCM void ConvertFromAudioBuffer(AudioBuffer* floatBuf, int32_t* intBuf) {
    for (size_t i = 0; i < AUDIO_BUFFER_SIZE; i++) {
        intBuf[i * 2] = floatToInt32(floatBuf[i].Left);
        intBuf[i * 2 + 1] = floatToInt32(floatBuf[i].Right);
    }
}

// ------------------------------------------------------------------------
//  Callback for transmission complete
// ------------------------------------------------------------------------
ITCM void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
    __disable_irq();
    // Convert audio buffer from float to int32_t format and store in the second half of txBuffer
    ConvertFromAudioBuffer(pOut, &txBuffer[SAI_HALF_BUFFER_SIZE]);
    __enable_irq();
}

// ------------------------------------------------------------------------
//  Callback for half transmission complete
// ------------------------------------------------------------------------
ITCM void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
    __disable_irq();
    // Convert audio buffer from float to int32_t format and store in the first half of txBuffer
    ConvertFromAudioBuffer(pOut, txBuffer);
    __enable_irq();
}

// ------------------------------------------------------------------------
//  Callback for reception complete
// ------------------------------------------------------------------------
ITCM void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai) {

	// Convert received int32_t buffer to float format for processing
    ConvertToAudioBuffer(&rxBuffer[SAI_HALF_BUFFER_SIZE], In);
    // Process audio data
    AudioCallback(In, Out2);
    __disable_irq();
    pOut = Out2;
    __enable_irq();
}

// ------------------------------------------------------------------------
//  Callback for half reception complete
// ------------------------------------------------------------------------
ITCM void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
	// Convert received int32_t buffer to float format for processing
    ConvertToAudioBuffer(rxBuffer, In);
    // Process audio data
    AudioCallback(In, Out1);
    __disable_irq();
    pOut = Out1;
    __enable_irq();
}

// ------------------------------------------------------------------------
// Start Audio Callback
// ------------------------------------------------------------------------
HAL_StatusTypeDef StartAudio(){
	HAL_StatusTypeDef Result;

	// Buffers initialization
	pOut=Out1;
	for(uint16_t Index = 0; Index < AUDIO_BUFFER_SIZE; Index++ ){
		In[Index].Left = 0.0f;
		In[Index].Right = 0.0f;
		Out1[Index].Left = 0.0f;
		Out1[Index].Right = 0.0f;
		Out2[Index].Left = 0.0f;
		Out2[Index].Right = 0.0f;
	}
	for(uint16_t Index = 0; Index < SAI_BUFFER_SIZE; Index++ ){
		rxBuffer[Index] = 0;
		txBuffer[Index] = 0;
	}

	if(Rev5 == __HardRev){
		if(HAL_OK != (Result = HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t*) rxBuffer, SAI_BUFFER_SIZE))){
			return Result;
		}

		return HAL_SAI_Transmit_DMA(&hsai_BlockB1, (uint8_t*) txBuffer, SAI_BUFFER_SIZE);
	}else if(Rev7 == __HardRev){
		if(HAL_OK != (Result = HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t*) rxBuffer, SAI_BUFFER_SIZE))){
			return Result;
		}

		return HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t*) txBuffer, SAI_BUFFER_SIZE);
	}else{
		return HAL_ERROR;
	}
}

