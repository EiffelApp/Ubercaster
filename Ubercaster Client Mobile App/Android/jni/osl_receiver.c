/*
 * osl_receiver.c 
 *
 * Author: Artem Bagautdinov, <artem.bagautdinov@gmail.com>
 * Copyright (C) 2013
 */

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <jni.h>
#include <android/log.h>
#include <opus.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>      /* OpenSL ES Android extensions */
#include "spsc_circular_queue.h"


/* OpenSL ES stuff */
static SLObjectItf osl_engine_obj = NULL;
static SLEngineItf osl_engine = NULL;
static SLObjectItf osl_mixer_obj = NULL;
static SLObjectItf osl_player_obj = NULL;
static SLPlayItf osl_player = NULL;
static SLAndroidSimpleBufferQueueItf osl_player_bq = NULL; 
static uint8_t *osl_player_buffers[2] = { NULL, NULL };

static uint64_t empty_buf_cnt = 0;

static void osl_player_bq_cbk(SLAndroidSimpleBufferQueueItf caller, void *arg)
{
    assert(caller != NULL);
    SpscCircularQueue *queue = (SpscCircularQueue *)arg;
    assert(queue != NULL);

    static int buf_index = 0;
    size_t buf_size = spsc_circular_queue_element_size(queue);

    if (!spsc_circular_queue_pop(queue, osl_player_buffers[buf_index])) {
        memset(osl_player_buffers[buf_index], 0, buf_size);
        empty_buf_cnt++;
    }

    (*caller)->Enqueue(caller, osl_player_buffers[buf_index], buf_size);
    buf_index = (buf_index == 0) ? 1 : 0;
}

void osl_player_init(SpscCircularQueue *queue, uint32_t samplerate)
{
    assert(queue != NULL);
    assert(samplerate > 0);
    assert(osl_engine_obj == NULL);
    assert(osl_engine == NULL);
    assert(osl_mixer_obj == NULL);
    assert(osl_player_obj == NULL);
    assert(osl_player == NULL);
    assert(osl_player_bq == NULL);

    SLresult rc = 0;

    /* initialize OpenSL ES audio engine */
    rc = slCreateEngine(&osl_engine_obj, 0, NULL, 0, NULL, NULL);
    assert(SL_RESULT_SUCCESS == rc);
    rc = (*osl_engine_obj)->Realize(osl_engine_obj, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == rc);
    rc = (*osl_engine_obj)->GetInterface(osl_engine_obj, SL_IID_ENGINE,
                                                                &osl_engine);
    assert(SL_RESULT_SUCCESS == rc);

    /* initialize OpenSL ES output mixer */
    const SLInterfaceID mixer_ids[] = { SL_IID_VOLUME };
    const SLboolean mixer_req[] = { SL_BOOLEAN_FALSE };
    rc = (*osl_engine)->CreateOutputMix(osl_engine, &osl_mixer_obj, 1,
                                                        mixer_ids, mixer_req);
    assert(SL_RESULT_SUCCESS == rc);
    rc = (*osl_mixer_obj)->Realize(osl_mixer_obj, SL_BOOLEAN_FALSE);
    assert(SL_RESULT_SUCCESS == rc);

    /* Initialize OpenSL ES audio player */
    /* audio source */
  	SLDataLocator_AndroidSimpleBufferQueue loc_bufq = {
        SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,
        2                                   /* 2 buffers in queue */
    };
  	SLDataFormat_PCM format_pcm = {
    	SL_DATAFORMAT_PCM,
        1,                                  /* mono */
        (SLuint32)(samplerate * 1000),      /* samplerate in milli-Hertz */
    	SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_PCMSAMPLEFORMAT_FIXED_16,
    	SL_SPEAKER_FRONT_CENTER,
        SL_BYTEORDER_LITTLEENDIAN
  	};
  	SLDataSource audio_src = { &loc_bufq, &format_pcm };
    /* audio sink */
  	SLDataLocator_OutputMix loc_outmix = {
        SL_DATALOCATOR_OUTPUTMIX,
    	osl_mixer_obj
    };
  	SLDataSink audio_sink = { &loc_outmix, NULL };
    const SLInterfaceID player_ids[] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE };
    const SLboolean player_req[] = { SL_BOOLEAN_TRUE };

  	rc = (*osl_engine)->CreateAudioPlayer(osl_engine, &osl_player_obj,
                                                    &audio_src, &audio_sink, 1,
                                                    player_ids, player_req);
  	assert(SL_RESULT_SUCCESS == rc);
  	rc = (*osl_player_obj)->Realize(osl_player_obj, SL_BOOLEAN_FALSE);
  	assert(SL_RESULT_SUCCESS == rc);
  	rc = (*osl_player_obj)->GetInterface(osl_player_obj, SL_IID_PLAY,
                                                                &osl_player);
  	assert(SL_RESULT_SUCCESS == rc);
  	rc = (*osl_player_obj)->GetInterface(osl_player_obj,
                                                SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                                &osl_player_bq);
  	assert(SL_RESULT_SUCCESS == rc);
    rc = (*osl_player_bq)->RegisterCallback(osl_player_bq, osl_player_bq_cbk,
                                                                        queue);
  	assert(SL_RESULT_SUCCESS == rc);

    /* allocate buffers */
    size_t buf_size = spsc_circular_queue_element_size(queue);
    osl_player_buffers[0] = calloc(1, buf_size);
    assert(osl_player_buffers[0] != NULL);
    osl_player_buffers[1] = calloc(1, buf_size);
    assert(osl_player_buffers[1] != NULL);

    rc = (*osl_player)->SetPlayState(osl_player, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == rc);
}

void osl_player_terminate()
{
    if (osl_player_obj) {
        if (osl_player) {
            SLresult rc = (*osl_player)->SetPlayState(osl_player,
                                                        SL_PLAYSTATE_PAUSED);
            assert(SL_RESULT_SUCCESS == rc);
        }

        (*osl_player_obj)->Destroy(osl_player_obj);
        osl_player_obj = NULL;
        osl_player = NULL;
        osl_player_bq = NULL;
    }

    if (osl_mixer_obj) {
        (*osl_mixer_obj)->Destroy(osl_mixer_obj);
        osl_mixer_obj = NULL;
    }

    if (osl_engine_obj) {
        (*osl_engine_obj)->Destroy(osl_engine_obj);
        osl_engine_obj = NULL;
        osl_engine = NULL;
    }

    if (osl_player_buffers[0]) {
        free(osl_player_buffers[0]);
        osl_player_buffers[0] = NULL;
    }

    if (osl_player_buffers[1]) {
        free(osl_player_buffers[1]);
        osl_player_buffers[1] = NULL;
    }
}

void osl_player_jumpstart(SpscCircularQueue *queue)
{
    assert(osl_player != NULL);

    size_t buf_size = spsc_circular_queue_element_size(queue);

    if (!spsc_circular_queue_pop(queue, osl_player_buffers[0]))
        memset(osl_player_buffers[0], 0, buf_size);

    if (!spsc_circular_queue_pop(queue, osl_player_buffers[1]))
        memset(osl_player_buffers[1], 0, buf_size);

    (*osl_player_bq)->Enqueue(osl_player_bq, osl_player_buffers[0], buf_size);
    (*osl_player_bq)->Enqueue(osl_player_bq, osl_player_buffers[1], buf_size);
}

/* UDP multicast stuff */
static int udp_sock = -1;
static bool udp_bcast_init()
{
    assert(udp_sock == -1);

    udp_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sock < 0) {
        __android_log_print(ANDROID_LOG_ERROR, "Ubercaster",
                            "Unable to create UDP socket");
        return false;
    }

    int reuse = 1;
    if (setsockopt(udp_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse,
                                                        sizeof(reuse)) < 0) {
        __android_log_print(ANDROID_LOG_ERROR, "Ubercaster",
                            "Failed to set SO_REUSEADDR for UDP socket");
        close(udp_sock);
        return false;
    }

    struct sockaddr_in sin_rcv;
    memset(&sin_rcv, 0, sizeof(sin_rcv));
    sin_rcv.sin_family = AF_INET;
    sin_rcv.sin_port = htons(12345);
    sin_rcv.sin_addr.s_addr = INADDR_ANY;

    if (bind(udp_sock, (struct sockaddr*)&sin_rcv, sizeof(sin_rcv))) {
        __android_log_print(ANDROID_LOG_ERROR, "Ubercaster",
                            "Can't bind the UDP socket");
        close(udp_sock);
        return false;
    }

    /* OK */
    return true;
}

static void udp_bcast_terminate()
{
    if (udp_sock != -1) {
        close(udp_sock);
        udp_sock = -1;
    }
}

/* receiver thread */
static pthread_t rcv_th_id;
static bool rcv_th_terminate = false;

static void* receiver_thread(void *arg)
{
    /* to get decoding that just works we have to use maximum size of the buffer
       which is (120 ms, 48kHz, mono, 16 bit) 11520 bytes of data */
    uint8_t dec_buf[11520];
    /* OPUS max bitrate is 512000 bit/sec, we use 120 msec (max) packets of data,
       so required size of the buffer is  512/8 * 120 = 7680 bytes + 2 bytes for
       Ubercaster streaming protocol header */
    uint8_t rcv_buf[7682];

    /* total capacity in elements of the circular queue; here is a brief
       description how it works: application reads the data from UDP socket,
       decodes them with OPUS decoder and then puts decoded data into
       the circular queue where they will be extracted during the playback */
    const int queue_elements_num = 20;
    const int start_after = 5;
    SpscCircularQueue *queue = NULL;

    OpusDecoder *opus_dec = NULL;
    uint32_t samplerate = 0;
    float frame_length = 0.0;

    bool player_initialized = false;
    bool player_started = false;
    int frames_in_queue = 0;

	while (!rcv_th_terminate) {
        ssize_t rcv_len = read(udp_sock, rcv_buf, sizeof(rcv_buf));
        if (rcv_len <= 0) {
            __android_log_print(ANDROID_LOG_WARN, "Ubercaster",
                            "Failed to receive UDP data");
            continue;
        }

        if (!player_initialized) {
            if (rcv_buf[0] != 0x01) {
                __android_log_print(ANDROID_LOG_ERROR, "Ubercaster",
                                    "Unsupported version of Ubercaster "
                                    "streaming protocol");
                return NULL;
            }

            /* extract samplerate */
            if ((rcv_buf[1] & 0xE0) == 0xC0)
                samplerate = 24000;
            else if ((rcv_buf[1] & 0xE0) == 0xE0)
                samplerate = 48000;
            else {
                __android_log_print(ANDROID_LOG_ERROR, "Ubercaster",
                                    "unsupported samplerate - 0x%02X "
                                    "0x%02X", rcv_buf[1], rcv_buf[1] & 0xE0);
                return NULL;
            }
            
            /* extract OPUS frame length */
            if ((rcv_buf[1] & 0x1C) == 0x04)
                frame_length = 2.5;
            else if ((rcv_buf[1] & 0x1C) == 0x08)
                frame_length = 5.0;
            else if ((rcv_buf[1] & 0x1C) == 0x0C)
                frame_length = 10.0;
            else if ((rcv_buf[1] & 0x1C) == 0x10)
                frame_length = 20.0;
            else if ((rcv_buf[1] & 0x1C) == 0x14)
                frame_length = 40.0;
            else if ((rcv_buf[1] & 0x1C) == 0x18)
                frame_length = 60.0;
            else {
                __android_log_print(ANDROID_LOG_ERROR, "Ubercaster",
                                    "Unsuported OPUS frame length - 0x%02X "
                                    "0x%02X", rcv_buf[1], rcv_buf[1] & 0xC1);
                return NULL;
            }

            /* initialize decoder */
            int opus_err = 0;
            opus_dec = opus_decoder_create(samplerate, 1, &opus_err);
            if (!opus_dec) {
                __android_log_print(ANDROID_LOG_ERROR, "Ubercaster",
                                    "unable to initialize OPUS decoder, "
                                    "error code - %d", opus_err);
                return NULL;
            }

            /* initialize circular queue */
            uint32_t frame_buf_size = samplerate / 1000 * frame_length * 2;
            queue = spsc_circular_queue_alloc(queue_elements_num, frame_buf_size);
            if (!queue) {
                __android_log_print(ANDROID_LOG_ERROR, "Ubercaster",
                                    "unable to allocate SPSC Circular Queue");
                opus_decoder_destroy(opus_dec);
                return NULL;
            }

            /* initialize the play and start the playback */
            osl_player_init(queue, samplerate);
            player_initialized = true;
        } else {
            int dec_samples = opus_decode(opus_dec, rcv_buf + 2, rcv_len - 2,
                                        (int16_t *)dec_buf, sizeof(dec_buf), 0);
            if (dec_samples <= 0) {
                __android_log_print(ANDROID_LOG_ERROR, "Ubercaster",
                                    "failed to decode the data");
                osl_player_terminate();
                spsc_circular_queue_free(queue);
                opus_decoder_destroy(opus_dec);
                exit(EXIT_FAILURE);
            }

            while (!spsc_circular_queue_push(queue, dec_buf) && !rcv_th_terminate)
                usleep((useconds_t)(frame_length * 1000.0));

            if (!player_started) {
                frames_in_queue++;

                if (frames_in_queue == start_after) {
                    osl_player_jumpstart(queue);
                    player_started = true;
                }
            }
        }
    }

    osl_player_terminate();

    if (empty_buf_cnt)
        __android_log_print(ANDROID_LOG_INFO, "Ubercaster",
                            "Empty buffers played: %" PRIu64, empty_buf_cnt);

    spsc_circular_queue_free(queue);
    opus_decoder_destroy(opus_dec);
}


jboolean Java_com_ubercaster_receiver_NativeReceiver_init(JNIEnv *env, jclass clazz)
{
    __android_log_print(ANDROID_LOG_INFO, "Ubercaster", "---> Java_com_receiver_init()");

    if (!udp_bcast_init())
        return JNI_FALSE;

    return JNI_TRUE;
}

void Java_com_ubercaster_receiver_NativeReceiver_start(JNIEnv *env, jclass clazz)
{
    __android_log_print(ANDROID_LOG_INFO, "Ubercaster", "---> Java_com_receiver_start()");
    rcv_th_terminate = false;
    pthread_create(&rcv_th_id, NULL, receiver_thread, NULL);
}

void Java_com_ubercaster_receiver_NativeReceiver_stop(JNIEnv *env, jclass clazz)
{
    __android_log_print(ANDROID_LOG_INFO, "Ubercaster", "---> Java_com_receiver_stop()");

    rcv_th_terminate = true;
    pthread_join(rcv_th_id, NULL);
}

void Java_com_ubercaster_receiver_NativeReceiver_terminate(JNIEnv *env, jclass clazz)
{
    __android_log_print(ANDROID_LOG_INFO, "Ubercaster", "---> Java_com_receiver_terminate()");
    udp_bcast_terminate();
}
