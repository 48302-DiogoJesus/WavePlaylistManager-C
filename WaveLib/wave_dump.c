#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "wavelib.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

void hex_dump(FILE *fd, uint8_t *buffer, size_t size) {
	size_t total = 0;
	char line[72], *pl1, *pl2;
	uint8_t * p = buffer;
	line[sizeof line - 1] = '\0';
	while (size > 0) {
		memset(line, ' ', sizeof line - 1);
		pl1 = line;
		pl2 = line + 55;
		pl1 += sprintf(pl1, "%04zX: ", total);
		size_t n_bytes = min(16, size);
		for (size_t i = 0; i < n_bytes; ++i, ++p) {
			pl1 += sprintf(pl1, "%02X ", *p);
			*pl2++ = (isprint(*p) && *p != '\t' && *p != '\r'
				&& *p != '\n' && *p != '\xc') ? *p : '.';
		}
		*pl1 = ' ';
		fprintf(fd, "%s\n", line);
		total += n_bytes;
		size -= n_bytes;
	}
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s <wave filename>\n", argv[0]);
		return -1;
	}
	Wave *wave = wave_load(argv[1]);
	if (wave == NULL) {
		fprintf(stderr, "Error loading file \"%s\"\n", argv[1]);
		return -1;
	}
	printf("NumChannels=%u\n", wave_get_number_of_channels(wave));
	printf("SampleRate=%d\n", wave_get_sample_rate(wave));
	printf("BitsPerSample=%d\n\n", wave_get_bits_per_sample(wave));

	int frame_size = wave_get_bits_per_sample(wave) / 8 * wave_get_number_of_channels(wave);
	int period = wave_get_sample_rate(wave) / 100;	/* 10 mili segundo */
	int frame_index = 0; /* início do áudio */

	uint8_t buffer[frame_size * period];
	size_t read_samples = wave_get_samples(wave, frame_index, buffer, period);

	hex_dump(stdout, buffer, read_samples * frame_size);

	wave_destroy(wave);
}
