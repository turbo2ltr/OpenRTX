/***************************************************************************
 *   Copyright (C) 2019 by Federico Amedeo Izzo IU2NUO,                    *
 *                         Niccolò Izzo IU2KIN,                            *
 *                         Silvano Seva IU2KWO                             *
 *                         Joseph Stephen VK7JS                            *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/
 // This file contains functions for announcing radio functions using the building blocks in voicePrompts.h/c.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
 #include "core/voicePromptUtils.h"

static void vpInitIfNeeded(VoicePromptQueueFlags_T flags)
{
	if (flags & vpqInit)
		vpInit();
}

static void vpPlayIfNeeded(VoicePromptQueueFlags_T flags)
{
	if (flags & vpqPlayImmediately)
		vpPlay();
}

static void removeUnnecessaryZerosFromVoicePrompts(char *str)
{
	const int NUM_DECIMAL_PLACES = 1;
	int len = strlen(str);
	for(int i = len; i > 2; i--)
	{
		if ((str[i - 1] != '0') || (str[i - (NUM_DECIMAL_PLACES + 1)] == '.'))
		{
			str[i] = 0;
			return;
		}
	}
}
void announceVFO()
{
	vpInit();
	
	vpQueuePrompt(PROMPT_VFO);
	
	vpPlay();	
}

void announceChannelName(channel_t* channel, uint16_t channelIndex, 
VoicePromptQueueFlags_T flags)
{
	vpInitIfNeeded(flags);
	
	if (flags & vpqIncludeDescriptions)
	{
		vpQueuePrompt(PROMPT_CHANNEL);
	}
	vpQueueInteger(channelIndex);
	
	// Only queue the name if it is not the same as the raw number.
	// Otherwise the radio will say channel 1 1 for channel 1.
	char numAsStr[16]="\0";
    snprintf(numAsStr, 16, "%d", channelIndex);
	if (strcmp(numAsStr, channel->name) != 0)
		vpQueueString(channel->name, flags);
	
	vpPlayIfNeeded(flags);
}

static void vpQueueFrequency(freq_t freq)
{
		char buffer[16];
	int mhz = (freq / 1000000);
	int khz = ((freq%1000000) / 10);
	
	snprintf(buffer, 16, "%d.%05d", mhz, khz);
	removeUnnecessaryZerosFromVoicePrompts(buffer);
	
	vpQueueString(buffer, vpAnnounceCommonSymbols);

	vpQueuePrompt(PROMPT_MEGAHERTZ);
}

void announceFrequencies(freq_t rx, freq_t tx, VoicePromptQueueFlags_T flags)
{
	vpInitIfNeeded(flags);
		// if rx and tx frequencies differ, announce both, otherwise just announce 
		// one.
	if (rx == tx)
		vpQueueFrequency(rx);
	else
	{
		vpQueuePrompt(PROMPT_RECEIVE);
		vpQueueFrequency(rx);
		vpQueuePrompt(PROMPT_TRANSMIT);
		vpQueueFrequency(tx);
	}
	vpPlayIfNeeded(flags);
} 

void announceRadioMode(uint8_t mode, VoicePromptQueueFlags_T flags)
{
	vpInitIfNeeded(flags);

	if (flags & vpqIncludeDescriptions)
		vpQueuePrompt(PROMPT_MODE);
	
	switch(mode)
	{
		case DMR:
			vpQueueStringTableEntry(&currentLanguage->dmr);
			break;
		case FM:
			vpQueueStringTableEntry(&currentLanguage->fm);
			break;
		case M17:
			vpQueueStringTableEntry(&currentLanguage->m17);
			break;
	}
	
	vpPlayIfNeeded(flags);
}

void vpAnnounceChannelSummary(channel_t* channel, uint16_t channelIndex, 
VoicePromptQueueFlags_T flags)
{
	 	if (!channel) return;
		
	vpInitIfNeeded(flags);
	
	// mask off init and play because this function will handle init and play.
	VoicePromptQueueFlags_T localFlags=flags & ~(vpqInit | vpqPlayImmediately); 
	if (vpLevel == vpHigh)
		localFlags |= vpqIncludeDescriptions;
	
	announceChannelName(channel, channelIndex, localFlags);
	announceFrequencies(channel->rx_frequency , channel->tx_frequency, localFlags);
	announceRadioMode(channel->mode,  localFlags);
	
	vpPlayIfNeeded(flags);
}

void AnnounceInputChar(char ch)
{
	char buf[2] = "\0";
	buf[0] = ch;
	
		vpInit();
		
	uint8_t flags = vpAnnounceCaps | vpAnnounceSpace | vpAnnounceCommonSymbols | vpAnnounceLessCommonSymbols;
		
	vpQueueString(buf, flags);
	
	vpPlay();
}

void announceInputReceiveOrTransmit(bool tx, VoicePromptQueueFlags_T flags)
{
	vpInitIfNeeded(flags);
	
	if (tx)
		vpQueuePrompt(PROMPT_TRANSMIT);
	else
		vpQueuePrompt(PROMPT_RECEIVE);
		
	vpPlayIfNeeded(flags);
}

void ReplayLastPrompt()
{
	if (vpIsPlaying())
		vpTerminate();
	else
		vpPlay();
}

void announceError()
{
	vpInit();
		
	vpQueueStringTableEntry(&currentLanguage->error);
	
	vpPlay();
}	