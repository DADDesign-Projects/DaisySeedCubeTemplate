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

// Convert int32_t to float (assuming 24-bit audio data in 32-bit container)
#define int32ToFloat(sample) static_cast<float>(0xFFFFFF & sample) / static_cast<float>(0x7FFFFF)

// Convert float to int32_t (assuming 24-bit audio data in 32-bit container)
#define floatToInt32(sample) static_cast<int32_t>(sample * static_cast<float>(0x7FFFFF))


// ------------------------------------------------------------------------
// AudioCallback
// ------------------------------------------------------------------------
extern void AudioCallback(AudioBuffer *pIn, AudioBuffer *pOut);

// Audio Buffer
AudioBuffer In[AUDIO_BUFFER_SIZE] = {0};
AudioBuffer Out1[AUDIO_BUFFER_SIZE] = {0};
AudioBuffer Out2[AUDIO_BUFFER_SIZE] = {0};

AudioBuffer* pOut=Out1;
int32_t rxBuffer[SAI_BUFFER_SIZE] = {0};
int32_t txBuffer[SAI_BUFFER_SIZE] = {0};

// ------------------------------------------------------------------------
// Convert int32_t buffer to float AudioBuffer
// ------------------------------------------------------------------------
void ConvertToAudioBuffer( int32_t* intBuf, AudioBuffer* floatBuf) {
    for (size_t i = 0; i < AUDIO_BUFFER_SIZE; i++) {
        floatBuf[i].Left = int32ToFloat(intBuf[i * 2]);
        floatBuf[i].Right = int32ToFloat(intBuf[i * 2 + 1]);
    }
}

// ------------------------------------------------------------------------
// Convert float AudioBuffer to int32_t buffer
// ------------------------------------------------------------------------
void ConvertFromAudioBuffer(AudioBuffer* floatBuf, int32_t* intBuf) {
    for (size_t i = 0; i < AUDIO_BUFFER_SIZE; i++) {
        intBuf[i * 2] = floatToInt32(floatBuf[i].Left);
        intBuf[i * 2 + 1] = floatToInt32(floatBuf[i].Right);
    }
}

// ------------------------------------------------------------------------
//  Callback for transmission complete
// ------------------------------------------------------------------------
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai) {
    __disable_irq();
    // Convert audio buffer from float to int32_t format and store in the second half of txBuffer
    ConvertFromAudioBuffer(pOut, &txBuffer[SAI_HALF_BUFFER_SIZE]);
    __enable_irq();
}

// ------------------------------------------------------------------------
//  Callback for half transmission complete
// ------------------------------------------------------------------------
void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
    __disable_irq();
    // Convert audio buffer from float to int32_t format and store in the first half of txBuffer
    ConvertFromAudioBuffer(pOut, txBuffer);
    __enable_irq();
}

// ------------------------------------------------------------------------
//  Callback for reception complete
// ------------------------------------------------------------------------
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai) {

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
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai) {
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

	// Reset CODEC
	HAL_GPIO_WritePin(RESET_CODEC_GPIO_Port, RESET_CODEC_Pin, GPIO_PIN_SET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(RESET_CODEC_GPIO_Port, RESET_CODEC_Pin, GPIO_PIN_RESET);
	HAL_Delay(1);
	HAL_GPIO_WritePin(RESET_CODEC_GPIO_Port, RESET_CODEC_Pin, GPIO_PIN_SET);
	HAL_Delay(1);

	if(HAL_OK != (Result = HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t*) rxBuffer, SAI_BUFFER_SIZE))){
		return Result;
	}

	return HAL_SAI_Transmit_DMA(&hsai_BlockA1, (uint8_t*) txBuffer, SAI_BUFFER_SIZE);
}

