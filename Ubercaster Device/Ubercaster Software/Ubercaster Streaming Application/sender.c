/*
 * sender.c - Implementation of a sender application for Ubercaster project,
 * this application starts audio capture with ALSA, encodes the captured
 * audio with OPUS and then do UDP multicasting of raw OPUS packets.
 *
 * Author: Artem Bagautdinov, <artem.bagautdinov@gmail.com>
 *	   KJ Yoo,            <kyungjindaum@gmail.com>
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <alloca.h>
#include <limits.h>
#include <inttypes.h>
#include <alsa/asoundlib.h>
#include </home/opus/include/opus.h>
#include <sched.h>
#include <pthread.h>
#include "spsc_circular_queue.h"

/* ALSA doesn't allow to include <sys/time.h>, have to declare function here */
struct timezone;
int gettimeofday(struct timeval *tv, struct timezone *tz);
int usleep(unsigned long usec);


/* ALSA audio stuff */
static snd_pcm_t *acapt_hdl = NULL;

static bool audio_capture_configure(uint32_t samplerate)
{
    assert(acapt_hdl != NULL);

	snd_pcm_hw_params_t *hw_params = NULL;
	snd_pcm_hw_params_alloca(&hw_params);

	int err = 0;

    /* set hardware params */
	if ((err = snd_pcm_hw_params_any(acapt_hdl, hw_params)) < 0) {
		fprintf(stderr, "ERROR: snd_pcm_hw_params_any() has failed - %s\n",
                                                            snd_strerror(err));
		return false;
	}

    /* interleaved samples */
	if ((err = snd_pcm_hw_params_set_access(acapt_hdl, hw_params,
                                        SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		fprintf(stderr, "ERROR: snd_pcm_hw_params_set_access() has "
                                        "failed - %s\n", snd_strerror(err));
        return false;
	}

    /* 1 channel (mono) capture */
	if ((err = snd_pcm_hw_params_set_channels(acapt_hdl, hw_params, 1)) < 0) {
		fprintf(stderr, "ERROR: snd_pcm_hw_params_set_channels() has "
                                        "failed - %s\n", snd_strerror(err));
        return false;
	}

    /* 16 bit PCM samples */
	if ((err = snd_pcm_hw_params_set_format(acapt_hdl, hw_params,
                                                SND_PCM_FORMAT_S16_LE)) < 0) {
		fprintf(stderr, "ERROR: snd_pcm_hw_params_set_format() has "
                                        "failed - %s\n", snd_strerror(err));
        return false;
	}

    /* set samplerate */
	if ((err = snd_pcm_hw_params_set_rate(acapt_hdl, hw_params, samplerate, 0)) < 0) {
		fprintf(stderr, "ERROR: snd_pcm_hw_params_set_rate() has "
                                        "failed - %s\n", snd_strerror(err));
        return false;
	}

    /* apply hardware parameters */
	if ((err = snd_pcm_hw_params(acapt_hdl, hw_params)) < 0) {
		fprintf(stderr, "ERROR: snd_pcm_hw_params() has failed - %s\n\n",
                                                            snd_strerror(err));
        return false;
	}

    /* set software parameters */
	snd_pcm_sw_params_t *sw_params = NULL;
	snd_pcm_sw_params_alloca(&sw_params);

	if ((err = snd_pcm_sw_params_current(acapt_hdl, sw_params)) < 0) {
		fprintf(stderr, "ERROR: snd_pcm_sw_params_current() has failed - %s\n",
                                                            snd_strerror(err));
        return false;
	}

	if ((err = snd_pcm_sw_params_set_start_threshold(acapt_hdl,
                                                        sw_params, 0U)) < 0) {
		fprintf(stderr, "ERROR: snd_pcm_sw_params_set_start_threshold() has "
                                        "failed - %s\n", snd_strerror(err));
        return false;
	}

    /* apply software parameters */
	if ((err = snd_pcm_sw_params(acapt_hdl, sw_params)) < 0) {
		fprintf(stderr, "ERROR: snd_pcm_sw_params() has failed - %s\n",
                                                            snd_strerror(err));
        return false;
	}

    return true;
}

static void audio_capture_init(uint32_t samplerate)
{
    assert(acapt_hdl == NULL);

	if (snd_pcm_open(&acapt_hdl, "default", SND_PCM_STREAM_CAPTURE, 0) < 0)
		exit(EXIT_FAILURE);

    if (!audio_capture_configure(samplerate)) {
        snd_pcm_close(acapt_hdl);
        acapt_hdl = NULL;
		exit(EXIT_FAILURE);
    }
}

static void audio_capture_terminate()
{
    if (acapt_hdl != NULL) {
        snd_pcm_close(acapt_hdl);
        acapt_hdl = NULL;
    }
}

static bool audio_capture_read(uint8_t *buf, unsigned long samples_num)
{
    assert(buf != NULL);

    long rc = 0;

    do {
        rc = snd_pcm_readi(acapt_hdl, buf, samples_num);

        if (rc < 0) {
            fprintf(stderr, "ERROR: snd_pcm_readi() has failed - %s\n",
                                                            snd_strerror(rc));
            return false;
        }

        buf += (rc*2);
        samples_num -= rc;
    } while (samples_num > 0);

    return true;
}


/* OPUS stuff */
static OpusEncoder *opus_enc = NULL;

static void audio_encoder_init(uint32_t samplerate, int complexity,
                                                            uint32_t bitrate)
{
    assert(opus_enc == NULL);

    int err = 0;
    /* 1 channel (mono), OPUS audio profile */
    opus_enc = opus_encoder_create(samplerate, 1, OPUS_APPLICATION_AUDIO, &err);
    if (!opus_enc) {
        fprintf(stderr, "ERROR: opus_encoder_create() has failed - %d\n", err);
        exit(EXIT_FAILURE);
    }

    opus_encoder_ctl(opus_enc, OPUS_SET_COMPLEXITY(complexity));
    opus_encoder_ctl(opus_enc, OPUS_SET_BITRATE(bitrate));
    opus_encoder_ctl(opus_enc, OPUS_SET_SIGNAL(OPUS_SIGNAL_MUSIC));
    opus_encoder_ctl(opus_enc, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_FULLBAND));
}

static void audio_encoder_terminate()
{
    if (opus_enc) {
        opus_encoder_destroy(opus_enc);
        opus_enc = NULL;
    }
}


/* UDP multicast stuff */
static int udp_sock = -1;
static struct sockaddr_in udp_sin;

static void udp_bcast_init(const char *bcast_addr, int port)
{
    assert(bcast_addr != NULL);
    assert(udp_sock == -1);

    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        fprintf(stderr, "ERROR: unable to create UDP socket, socket() has "
                                            "failed - %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    int broadcastEnable = 1;
    if (setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &broadcastEnable,
                                            sizeof(broadcastEnable)) != 0) {
        fprintf(stderr, "ERROR: can't set broadcast flag for UDP socket - %s\n",
                                                            strerror(errno));
        close(udp_sock);
        exit(EXIT_FAILURE);
    }

    memset(&udp_sin, 0, sizeof(udp_sin));
    udp_sin.sin_family = AF_INET;
    udp_sin.sin_addr.s_addr = inet_addr(bcast_addr);
    udp_sin.sin_port = htons(port);

    if (bind(udp_sock, (const struct sockaddr*)&udp_sin, sizeof(udp_sin))) {
        fprintf(stderr, "ERROR: can't bind the socked with broadcast address "
                                                    "- %s\n", strerror(errno));
        close(udp_sock);
        exit(EXIT_FAILURE);
    }
}

static void udp_bcast_terminate()
{
    if (udp_sock != -1) {
        close(udp_sock);
        udp_sock = -1;
        memset(&udp_sin, 0, sizeof(udp_sin));
    }
}


/* SPSC Queue stuff */
static SpscCircularQueue *queue = NULL;

static void queue_init(int elements_num, size_t element_size)
{
    assert(queue == NULL);

    queue = spsc_circular_queue_alloc(elements_num, element_size);
    if (!queue) {
        fprintf(stderr, "ERROR: unable to allocate circular queue\n");
        exit(EXIT_FAILURE);
    }
}

static void queue_terminate()
{
    if (queue) {
        spsc_circular_queue_free(queue);
        queue = NULL;
    }
}


/* Signals handling */
static volatile bool app_terminate = false;

static void sig_handler(int signo)
{
	switch (signo) {
	case SIGINT:
	case SIGHUP:
	case SIGKILL:
		app_terminate = 1;
		break;
	default:
		break;
	}
}


/* Application options */
/* TODO: move these options into a config file */
static uint32_t samplerate = 48000;         /* 24kHz or 48kHz */
static float frame_length = 20;           /* 2.5/5/10/20/40/60 ms */
static int opus_complexity = 10;            /* 0 - 10 and 10 is for the highest
                                               complexity */
static uint32_t opus_bitrate = 64000;      /* 128kbps */
static const char *bcast_addr = "10.0.0.255";
static int port = 12345;
static int queue_elements_num = 100;


/* Threads */
static volatile bool terminate_threads = false;

/* total number of captured audio buffers */
static uint64_t capt_buffers = 0;
/* number fo dropped audio buffers */
static uint64_t capt_buffers_dropped = 0;
/* total time for snd_pcm_readi */
static uint64_t capt_time = 0;
/* min/max times for audio capture with snd_pcm_readi() */
static uint32_t capt_min_time = 0;
static uint32_t capt_max_time = 0;

void* capture_thread(void *arg)
{
    size_t capt_samples_num = samplerate / 1000 * frame_length;
    uint8_t *capt_buf = calloc(1, capt_samples_num * 2);
    assert(capt_buf != NULL);

    /* start the capture */
	snd_pcm_prepare(acapt_hdl);
    snd_pcm_start(acapt_hdl);
    snd_pcm_reset(acapt_hdl);

    while (!terminate_threads) {
		/* capture, track the capture time  */
        struct timeval start_ts, stop_ts;
        gettimeofday(&start_ts, NULL);

        if (!audio_capture_read(capt_buf, capt_samples_num))
            continue;

        gettimeofday(&stop_ts, NULL);
        uint32_t time_delta = (stop_ts.tv_sec - start_ts.tv_sec) * 1000000 +
                                        (stop_ts.tv_usec - start_ts.tv_usec);

        if (capt_min_time == 0)
            capt_min_time = time_delta;
        else
            capt_min_time = (time_delta < capt_min_time) ?
                                                time_delta : capt_min_time;

        if (capt_max_time == 0)
            capt_max_time = time_delta;
        else
            capt_max_time = (time_delta > capt_max_time) ?
                                                time_delta : capt_max_time;

        capt_time += time_delta;
        capt_buffers++;

        if (!spsc_circular_queue_push(queue, capt_buf)) {
            capt_buffers_dropped++;
            sched_yield();
        }
    }

    free(capt_buf);
    return NULL;
}

/* total number of encoded buffers */
static uint64_t enc_buffers = 0;
/* total time for opus_encode() */
static uint64_t enc_time = 0;
/* min/max times for OPUS audio encoding with opus_encode() */
static uint32_t enc_min_time = 0;
static uint32_t enc_max_time = 0;

void* enc_send_thread(void *arg)
{
    size_t capt_samples_num = samplerate / 1000 * frame_length;
    uint8_t *capt_buf = calloc(1, capt_samples_num * 2);
    assert(capt_buf != NULL);
    /* OPUS max bitrate is 512000 bit/sec, we use 120 msec (max) packets of data,
       so required size of the buffer is  512/8 * 120 = 7680 bytes + 2 bytes for
       the header */
    uint8_t *out_buf = calloc(1, 7682);
    assert(out_buf != NULL);
    uint8_t *enc_buf = out_buf + 2;

    /* Ubercaster packet header:
    
       First byte (8 bits) is for Ubercaster protocol version, current version
       is 1, so the value is of the first byte is 0000 0001 . This value will be
       increased with each significant change in the protocol header.
       
       Next 3 bits (2nd byte) are for samplerate, currently Ubercaster supports
       only two samplerates 24kHz and 48kHz:
           * values from 000 to 101 are reserved for lower samplerate values,
             since the protocol and the application itself is experimental it
             may require to add qick support for lower bitrates without making
             changes to the protocol version or the Ubercaster protocol parser 
             at the client side
           * 110 is for 24 kHz
           * 111 is for 48 kHz

       Next 3 bits (of the second byte) are for the size of audio frame:
          * 000 - this value is not used
          * 001 - 2.5 ms frames
          * 010 -   5 ms frames
          * 011 -  10 ms frames
          * 100 -  20 ms frames
          * 101 -  40 ms frames
          * 110 -  60 ms frames
          * 111 - this value is not used
    */

    out_buf[0] = 1;

    uint8_t samplerate_enc = 0, frame_length_enc = 0;

    if (samplerate == 24000)
        samplerate_enc = 0xC0;
    else
        samplerate_enc = 0xE0;

    if (frame_length == 2.5)
        frame_length_enc = 0x04;
    else if (frame_length == 5.0)
        frame_length_enc = 0x08;
    else if (frame_length == 10.0)
        frame_length_enc = 0x0C;
    else if (frame_length == 20.0)
        frame_length_enc = 0x10;
    else if (frame_length == 40.0)
        frame_length_enc = 0x14;
    else    /* 60 ms */
        frame_length_enc = 0x18;

    out_buf[1] = samplerate_enc | frame_length_enc;

    while (!terminate_threads) {
        if (!spsc_circular_queue_pop(queue, capt_buf)) {
            // sched_yield();
            usleep(500);
            continue;
        }

        /* encoding */
        struct timeval start_ts, stop_ts;
        gettimeofday(&start_ts, NULL);
        ssize_t len = opus_encode(opus_enc, (opus_int16 *)capt_buf,
                                    capt_samples_num, enc_buf, 7680);

        if (len <= 0) {
            fprintf(stderr, "ERROR: failed to encode data\n");
            continue;
        }

        gettimeofday(&stop_ts, NULL);
        uint32_t time_delta = (stop_ts.tv_sec - start_ts.tv_sec) * 1000000 +
                                        (stop_ts.tv_usec - start_ts.tv_usec);

        if (enc_min_time == 0)
            enc_min_time = time_delta;
        else
            enc_min_time = (time_delta < enc_min_time) ?
                                                time_delta : enc_min_time;

        if (enc_max_time == 0)
            enc_max_time = time_delta;
        else
            enc_max_time = (time_delta > enc_max_time) ?
                                                time_delta : enc_max_time;

        enc_time += time_delta;
        enc_buffers++;

        /* send */
        while (sendto(udp_sock, out_buf, len + 2, 0,
                            (struct sockaddr *)&udp_sin, sizeof(udp_sin)) < 0) {
            if (EINTR != errno) {
                fprintf(stderr, "ERROR: unable to send data - %s\n",
                                                            strerror(errno));
                break;
            }
        }
    }

    free(out_buf);
    free(capt_buf);
    return NULL;
}


int main(int argc, char *argv[])
{
    /* setup termination signals handler */
	signal(SIGINT, sig_handler);
	signal(SIGHUP, sig_handler);
	signal(SIGKILL, sig_handler);

    audio_capture_init(samplerate);
    atexit(audio_capture_terminate);

    audio_encoder_init(samplerate, opus_complexity, opus_bitrate);
    atexit(audio_encoder_terminate);

    udp_bcast_init(bcast_addr, port);
    atexit(udp_bcast_terminate);

    size_t element_size = (samplerate / 1000 * frame_length) * 2;
    queue_init(queue_elements_num, element_size);
    atexit(queue_terminate);

    pthread_t capt_th, enc_send_th;
    if (pthread_create(&enc_send_th, NULL, enc_send_thread, NULL)) {
        fprintf(stderr, "ERROR: unable to spawn encoding and sending thread\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&capt_th, NULL, capture_thread, NULL)) {
        fprintf(stderr, "ERROR: unable to spawn capturing thread\n");
        terminate_threads = true;
        pthread_join(enc_send_th, NULL);
        exit(EXIT_FAILURE);
    }

	printf("Capturing, encoding and sending Ctrl+C to terminate...\n");
    struct timeval start_ts, cur_ts;

    gettimeofday(&start_ts, NULL);

	while (!app_terminate) {
        sleep(1);
        gettimeofday(&cur_ts, NULL);
        uint32_t time_delta_ms = (cur_ts.tv_sec - start_ts.tv_sec) * 1000 + 
                                    (cur_ts.tv_usec - start_ts.tv_usec) / 1000;

        printf("%u.%03u capt:%"PRIu64" drop:%"PRIu64" enc: %"PRIu64"\n",
                time_delta_ms/1000, time_delta_ms % 1000, capt_buffers,
                capt_buffers_dropped, enc_buffers);
    }

    terminate_threads = true;
    pthread_join(capt_th, NULL);
    pthread_join(enc_send_th, NULL);

    printf("\n");
    printf("Captured packets: %"PRIu64"    Packets dropped at "
            "capture: %" PRIu64"\n", capt_buffers, capt_buffers_dropped);
    printf("Capture time (min/avg/max): %u/%u/%u microseconds\n",
            capt_min_time, (uint32_t)(capt_time/capt_buffers), capt_max_time);
    printf("Encoded packets: %"PRIu64"\n", enc_buffers);
    printf("Encoding time (min/avg/max): %u/%u/%u microseconds\n",
            enc_min_time, (uint32_t)(enc_time/enc_buffers), enc_max_time);

    /* OK */
    exit(EXIT_SUCCESS);
}
