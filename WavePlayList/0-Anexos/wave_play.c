/*
 * Baseado no exemplo
 * https://www.alsa-project.org/alsa-doc/alsa-lib/_2test_2pcm_min_8c-example.html
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

#include "wavelib.h"

void play(Wave* wave);

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s <wave filename>", argv[0]);
		return -1;
	}

	Wave *wave = wave_load(argv[1]);
	if (wave == NULL) {
		fprintf(stderr, "Error loading file \"%s\"\n", argv[1]);
		return -1;
	}
	const snd_pcm_sframes_t period_size = 64;
	int frame_size = 2;

	uint8_t buffer[period_size * frame_size];
	size_t frame_index = 0;

	play(wave);

	wave_destroy(wave);
}

#define	SOUND_DEVICE	"default"

void play(Wave *wave) {

	snd_pcm_t *handle = NULL;
	int result = snd_pcm_open(&handle, SOUND_DEVICE, SND_PCM_STREAM_PLAYBACK, 0);
	if (result < 0) {
		printf("snd_pcm_open(&handle, %s, SND_PCM_STREAM_PLAYBACK, 0): %s\n",
				SOUND_DEVICE, snd_strerror(result));
		exit(EXIT_FAILURE);
	}

	snd_config_update_free_global();

	result = snd_pcm_set_params(handle,
					  SND_PCM_FORMAT_S16_LE,
					  SND_PCM_ACCESS_RW_INTERLEAVED,
					  wave_get_number_of_channels(wave),
					  wave_get_sample_rate(wave),
					  1,
					  500000);
	if (result < 0) {
		fprintf(stderr, "Playback open error: %s\n", snd_strerror(result));
		exit(EXIT_FAILURE);
	}

	const snd_pcm_sframes_t period_size = 64;
	int frame_size = snd_pcm_frames_to_bytes(handle, 1);

	uint8_t buffer[period_size * frame_size];
	size_t frame_index = 0;

	size_t read_frames = wave_get_samples(wave, frame_index, buffer, period_size);

	while (read_frames > 0) {
		snd_pcm_sframes_t wrote_frames = snd_pcm_writei(handle, buffer, read_frames);
		if (wrote_frames < 0)
			wrote_frames = snd_pcm_recover(handle, wrote_frames, 0);
		if (wrote_frames < 0) {
			printf("snd_pcm_writei failed: %s\n", snd_strerror(wrote_frames));
			break;
		}

		if (wrote_frames < read_frames)
			fprintf(stderr, "Short write (expected %li, wrote %li)\n",
					read_frames, wrote_frames);

		frame_index += period_size;
		read_frames = wave_get_samples(wave, frame_index, buffer, period_size);
	}
	/* pass the remaining samples, otherwise they're dropped in close */
	result = snd_pcm_drain(handle);
	if (result < 0)
		printf("snd_pcm_drain failed: %s\n", snd_strerror(result));

	snd_pcm_close(handle);
	snd_config_update_free_global();
}
