/*
 * libslashfx
 * Copyright (C) 2005-2012 Kostas Michalopoulos
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Kostas Michalopoulos <badsector@runtimelegend.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <slashfx.h>

typedef struct _Buffer
{
	unsigned char	*data;
	unsigned int	position;
} Buffer;

typedef struct _Envelope
{
	double		*x;
	double		*y;
	int		size;
} Envelope;

typedef struct _Effect
{
	double		globalVol;
	double		squarization;
	double		delay, delayAmp;
	double		purify;
	unsigned int	waveFuncCode;
	unsigned int	length;
	int		upHerz;
	
	Envelope	freqEnv;
	Envelope	volEnv;
} Effect;

static double sineFunc(double v)
{
	return sin(v);
}

static double hemisineFunc(double v)
{
	return sin(v) + sin(v*2)/2;
}

static double squareFunc(double v)
{
	return sin(v)>0.0?1.0:0.0;
}

static Buffer *createBuffer(void *data)
{
	Buffer	*buffer = malloc(sizeof(Buffer));
	buffer->data = (unsigned char*)data;
	buffer->position = 0;
	
	return buffer;
}

static void destroyBuffer(Buffer *buffer)
{
	free(buffer);
}

static void readBuffer(Buffer *buffer, void *data, int length)
{
	memcpy(data, buffer->data + buffer->position, length);
	buffer->position += length;
}

static uint32_t readBuffer32(Buffer *buffer)
{
	uint8_t b1 = buffer->data[buffer->position++];
	uint8_t b2 = buffer->data[buffer->position++];
	uint8_t b3 = buffer->data[buffer->position++];
	uint8_t b4 = buffer->data[buffer->position++];

	return b1 | b2 << 8 | b3 << 16 | b4 << 24;
}

static double readBufferDouble(Buffer *buffer)
{
	union { uint64_t i; double d; } conv;

	conv.i = readBuffer32(buffer);
	conv.i |= (uint64_t) readBuffer32(buffer) << 32;
	return conv.d;
}

static void readEnvelope(Buffer *fb, Envelope *e)
{
	int	i;
	
	e->size = readBuffer32(fb);
	e->x = malloc(sizeof(double)*e->size);
	e->y = malloc(sizeof(double)*e->size);
	
	for (i=0;i<e->size;i++)
	{
		e->x[i] = readBufferDouble(fb);
		e->y[i] = readBufferDouble(fb);
	}
}

int ssfxLoadEffectFromMemory(char *ptr, short **wave, unsigned int *length)
{
	Buffer		*fb = createBuffer(ptr);
	char		id[4];
	char		version;
	Effect		fx;
	int		i, Fp, Vp, DP, Fnpp, Vnpp, wlength;
	double		T, F, V, Fs, Vs, SP, len, sample;
	double		(*waveFunc)(double) = NULL;
	double		*fwave;
	
	readBuffer(fb, id, 4);
	if (id[0] != 'S' || id[1] != 'S' || id[2] != 'F' || id[3] != 'X')
	{
		destroyBuffer(fb);
		return 0;
	}
	
	readBuffer(fb, &version, 1);
	if (version != 1)
	{
		destroyBuffer(fb);
		return 0;
	}
	
	fx.globalVol = readBufferDouble(fb);
	fx.squarization = readBufferDouble(fb);
	fx.delay = readBufferDouble(fb);
	fx.delayAmp = readBufferDouble(fb);
	fx.purify = readBufferDouble(fb);
	fx.waveFuncCode = readBuffer32(fb);
	fx.length = readBuffer32(fb);
	fx.upHerz = readBuffer32(fb);
	
	readEnvelope(fb, &fx.freqEnv);
	readEnvelope(fb, &fx.volEnv);
	
	destroyBuffer(fb);

	
	switch (fx.waveFuncCode)
	{
		case 0:
			waveFunc = sineFunc;
			break;
		case 1:
			waveFunc = hemisineFunc;
			break;
		case 2:
			waveFunc = squareFunc;
			break;
	}
	
	wlength = fx.length*22050/1000;
	*length = wlength;
	
	Fp = Vp = 0;
	F = fx.freqEnv.y[0]*(double)fx.upHerz;
	V = fx.volEnv.y[0];
	len = (fx.freqEnv.x[1] - fx.freqEnv.x[0])*(double)wlength;
	if (len <= 0.0)
		len = 1.0;
	Fs = (fx.freqEnv.y[1]*(double)fx.upHerz -
		fx.freqEnv.y[0]*(double)fx.upHerz)/len;
	len = (fx.volEnv.x[1] - fx.volEnv.x[0])*(double)wlength;
	Vs = (fx.volEnv.y[1] - fx.volEnv.y[0])/len;
	
	Fnpp = (int)(fx.freqEnv.x[1]*(double)wlength);
	Vnpp = (int)(fx.volEnv.x[1]*(double)wlength);
	
	fwave = malloc(wlength*sizeof(double));
	*wave = malloc(wlength*sizeof(short));
	T = 0;
	for (i=0;i<wlength;i++)
	{
		if (i == Fnpp && Fp < fx.freqEnv.size - 1)
		{
			Fp++;
			F = fx.freqEnv.y[Fp]*(double)fx.upHerz;
			len = (fx.freqEnv.x[Fp + 1] - fx.freqEnv.x[Fp])*
				(double)wlength;
			if (len <= 0.0)
				len = 1.0;
			Fs = (fx.freqEnv.y[Fp + 1]*(double)fx.upHerz -
				fx.freqEnv.y[Fp]*(double)fx.upHerz)/len;
			Fnpp = (int)(fx.freqEnv.x[Fp + 1]*(double)wlength);
		}
		if (i == Vnpp && Vp < fx.volEnv.size - 1)
		{
			Vp++;
			V = fx.volEnv.y[Vp];
			len = (fx.volEnv.x[Vp + 1] - fx.volEnv.x[Vp])*
				(double)wlength;
			if (len <= 0.0)
				len = 1.0;
			Vs = (fx.volEnv.y[Vp + 1] - fx.volEnv.y[Vp])/len;
			Vnpp = (int)(fx.volEnv.x[Vp + 1]*(double)wlength);
		}
		
		SP = waveFunc(T);
		T += F*3.141592653/22050.0;
		sample = SP;
		
		if (fx.squarization != 0.0)
		{
			sample = sample*(1-fx.squarization) +
				squareFunc(T)*fx.squarization;
		}
		
		if (fx.delay != 0.0 && fx.delayAmp != 0.0)
		{
			DP = i - (int)(fx.delay*(double)wlength);
			if (DP >= 0)
				sample = sample + fwave[DP]*fx.delayAmp;
		}
		
		if (fx.purify != 0.0)
		{
			sample = (sample + sineFunc(T)*fx.purify)/
				(1 + fx.purify);
		}
		
		sample *= V*fx.globalVol;
		
		if (sample > 0.95)
			sample = 0.95;
		else if (sample < -0.95)
			sample = -0.95;
			
		fwave[i] = sample;
		(*wave)[i] = (short)(sample * 32760.0);
		
		F += Fs;
		V += Vs;
	}
	free(fwave);
	
	free(fx.freqEnv.y);
	free(fx.freqEnv.x);
	
	return 1;
}

int ssfxLoadEffectFromFile(char *fname, short **wave, unsigned int *length)
{
	FILE	*f = fopen(fname, "rb");
	size_t	size;
	void	*buffer;
	int	r;
	if (!f)
		return 0;
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fseek(f, 0, SEEK_SET);
	buffer = malloc(size);
	fread(buffer, 1, size, f);
	fclose(f);
	
	r = ssfxLoadEffectFromMemory(buffer, wave, length);
	
	free(buffer);
	
	return r;
}
