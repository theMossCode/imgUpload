
#ifndef FAST_SPI_H
#define FAST_SPI_H

#include "Arduino.h"

void init_fast_spi();
void enable_spi(bool state);
void enable_workaround(NRF_SPIM_Type *spim, uint32_t ppi_channel, uint32_t gpiote_channel);
void disable_workaround(NRF_SPIM_Type *spim, uint32_t ppi_channel, uint32_t gpiote_channel);
void write_fast_spi(const uint8_t *ptr, uint32_t len);
void read_fast_spi(uint8_t *ptr, uint32_t len);

#endif