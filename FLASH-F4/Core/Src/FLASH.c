#include "FLASH.h"


/**
* @brief Unlock FLASH
* @retval FLASH Lock state 
*
*/
uint8_t FLASH_Unlock()
{
	/* UNLOCK FLASH */
	// ----------------------------- LL and HAL ----------------------------- //
	FLASH->KEYR = 0x45670123U;
	FLASH->KEYR = 0xCDEF89ABU;
	return (FLASH->SR & FLASH_CR_LOCK);

}
/**
* @brief lock FLASH
* @retval 
*
*/
void FLASH_Lock()
{
	// ----------------------------- LL and HAL ----------------------------- //
	FLASH->CR |= FLASH_CR_LOCK; 
}
/**
* @retval OptLock state
*
*/
void FLASH_LockOpt()
{
	// alow program and erase in the user config sector
	
	// ----------------------------- LL and HAL ----------------------------- //
	FLASH->OPTCR |= FLASH_OPTCR_OPTLOCK; 
}

/**
* @retval OptLock state
*
*/
uint8_t FLASH_UnlockOpt()
{
	// alow program and erase in the user config sector
	
	// ----------------------------- LL and HAL ----------------------------- //
	FLASH->OPTKEYR = 0x08192A3B;
	FLASH->OPTKEYR = 0x4C5D6E7F;
	return ( FLASH->OPTCR & FLASH_OPTCR_OPTLOCK) ;
	
}
/**
* @param SectorAdd : sector address
*		 							data : data to transmit
*	    						length : number of data to write
*									size : size of one data in byte
* @retval Error state 
*
*/
HAL_StatusTypeDef FLASH_Write(uint32_t SectorAdd, void * data, uint8_t size, uint16_t length)
{
	uint8_t i ;
  // --------------------------------------- LL & HAL ---------------------------------------//

	FLASH_Erase_Sector(SectorAdd, FLASH_VOLTAGE_RANGE_3);
	FLASH_Unlock();
	// wait for busy flag
	
	while ( ( FLASH->SR & FLASH_SR_BSY) ) ;
	
	// program bits reset 8bits default
	FLASH->CR &= ~(0x03UL << 8 );
	
	if ( data == NULL) 
	{
		while ( FLASH->SR & FLASH_SR_BSY ); 	
		FLASH_Lock();
		return HAL_ERROR;
	}
	
	if ( size == 1 )
	{
		FLASH->CR |= FLASH_CR_PG; // activate programming
	
		for ( i = 0; i < length; ++i)
		{
			* ( (volatile uint8_t *) (SectorAdd++) ) = ( (uint8_t *)data ) [i];
			if ( ( FLASH->SR & FLASH_SR_PGAERR ) || (FLASH->SR & FLASH_SR_PGPERR) || ( FLASH->SR & FLASH_SR_PGSERR) )
			{
				while ( FLASH->SR & FLASH_SR_BSY );
				FLASH->SR |=  ( (FLASH_SR_PGAERR) | (FLASH_SR_PGPERR) | (FLASH_SR_PGSERR));
				FLASH->CR &= ~(FLASH_CR_PG);
				FLASH_Lock();
				return HAL_ERROR;
			}
		}
	}
	
	else if ( size == 2 )
	{
		// program x16
		FLASH->CR |= (0x01UL << 8 );
		FLASH->CR |= FLASH_CR_PG; // activate programming
	
		for ( i = 0; i < length; ++i)
		{
			* ( (volatile uint16_t *) (SectorAdd) ) = ( (uint16_t *)data ) [i];
			SectorAdd += 2;
			
			if ( ( FLASH->SR & FLASH_SR_PGAERR ) || (FLASH->SR & FLASH_SR_PGPERR) || ( FLASH->SR & FLASH_SR_PGSERR) )
			{
				while ( FLASH->SR & FLASH_SR_BSY );
				FLASH->SR |=  ( (FLASH_SR_PGAERR) | (FLASH_SR_PGPERR) | (FLASH_SR_PGSERR));
				FLASH->CR &= ~(FLASH_CR_PG);
				FLASH_Lock();
				return HAL_ERROR;
			}
		}
	}
	else if ( size == 4 )
	{
		// program x32
		FLASH->CR |= (0x02UL << 8 );

		FLASH->CR |= FLASH_CR_PG; // activate programming
	
		for ( i = 0; i < length; ++i)
		{
			* ( (volatile uint32_t *) (SectorAdd) ) = ( (uint32_t *)data )[i];
			SectorAdd += 4;
			
			if ( ( FLASH->SR & FLASH_SR_PGAERR ) || (FLASH->SR & FLASH_SR_PGPERR) || ( FLASH->SR & FLASH_SR_PGSERR) )
			{
				while ( FLASH->SR & FLASH_SR_BSY );
				FLASH->SR |=  ( (FLASH_SR_PGAERR) | (FLASH_SR_PGPERR) | (FLASH_SR_PGSERR));
				FLASH->CR &= ~(FLASH_CR_PG);
				FLASH_Lock();
				return HAL_ERROR;
			}
		}
	}
	else if ( size == 8 )
	{
		// program x64
		FLASH->CR |= (0x03UL << 8);
		
		FLASH->CR |= FLASH_CR_PG; // activate programming
	
		for ( i = 0; i < length; ++i)
		{
			* ( (volatile uint64_t *) (SectorAdd) ) = ( (uint64_t *)data ) [i];
			SectorAdd += 8;
			
			if ( ( FLASH->SR & FLASH_SR_PGAERR ) || (FLASH->SR & FLASH_SR_PGPERR) || ( FLASH->SR & FLASH_SR_PGSERR) )
			{
				while ( FLASH->SR & FLASH_SR_BSY );
				FLASH->SR |=  ( (FLASH_SR_PGAERR) | (FLASH_SR_PGPERR) | (FLASH_SR_PGSERR));
				FLASH->CR &= ~(FLASH_CR_PG);
				FLASH_Lock();
				return HAL_ERROR;
			}
		}
	}
	else
	{
		while ( FLASH->SR & FLASH_SR_BSY );
		FLASH->CR &= ~(FLASH_CR_PG);
		FLASH_Lock();
		return HAL_ERROR;
	}

	while ( FLASH->SR & FLASH_SR_BSY ); 
	
	FLASH->CR &= ~(FLASH_CR_PG);
	
	FLASH_Lock();
	
	return HAL_OK;
}

/**
* @param size : number of bytes for one data
*		 SectorID : sector address
*		 RxBuf : buffer to store data
*	     length : number of data to read
* @retval Error state 
*
*/
HAL_StatusTypeDef FLASH_Read(uint32_t SectorID, void * RxBuf, uint8_t size, uint16_t length)
{
	// ---------------------- LL & HAL ---------------------//
	uint8_t i;
	HAL_StatusTypeDef state = HAL_ERROR;
	
	while ( FLASH->SR & FLASH_SR_BSY ); 
	
	
	if ( size == 1 )
	{
		for ( i = 0; i < length; ++i)
		{
			((uint8_t * )RxBuf)[i] = *((volatile uint8_t * ) SectorID++);
		}
		state = HAL_OK;
	
	}
	else if ( size == 2 )
	{
		for ( i = 0; i < length; ++i)
		{
			((uint16_t * )RxBuf)[i] = *((volatile uint16_t * ) SectorID);
			SectorID += 2;
		}
		state = HAL_OK;
		
	}
	else if ( size == 4 )
	{
		for ( i = 0; i < length; ++i)
		{
			((uint32_t * )RxBuf)[i] = *((volatile uint32_t * ) SectorID);
			SectorID += 4;
		}
		state = HAL_OK;
		
	}
	else if ( size == 8 )
	{
		for ( i = 0; i < length; ++i)
		{
			((uint64_t * )RxBuf)[i] = *((volatile uint64_t * ) SectorID);
			SectorID += 8;
		}
		state = HAL_OK;	
	}
	
	else{
		__asm("NOP");
	}
	
	while ( FLASH->SR & FLASH_SR_BSY ); 
	
	return state;
}
